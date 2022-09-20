#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/signal.h>
//----------------------------------------------------------------------------------------------------------------------
#include "programthread.h"
//----------------------------------------------------------------------------------------------------------------------
ProgramThread::ProgramThread():inMQueue(false)
{
	int32_t ret; 
	pthread_mutexattr_t		cmdMutAttr;
	ret = pthread_mutexattr_init (&cmdMutAttr);
	ret = pthread_mutexattr_settype(&cmdMutAttr, PTHREAD_MUTEX_NORMAL);
	ret = pthread_mutex_init (&cmdMut, &cmdMutAttr);
	epollFD = epoll_create1(EPOLL_CLOEXEC);
	char* span = getenv("THREAD_PERIODIC_TIMER_SPAN");
	uint64_t ispan;
	if(span)
		ispan=atoi(span);
	ThreadSleepNS=ispan?ispan:THREAD_PERIODIC_TIMER_SPAN_DEFAULT;
	char* eventlimit = getenv("PROCESS_PACKETS_AT_ONCE");
	uint32_t ilim;
	if(eventlimit)
		ilim=atoi(eventlimit);
	eventsAtOnce=ilim?ilim:PROCESS_PACKETS_AT_ONCE_DEFAULT;
}
//----------------------------------------------------------------------------------------------------------------------
ProgramThread::~ProgramThread()
{
	if(epollFD >= 0)
		close(epollFD);
	if(tmrFD >= 0)
		close(tmrFD);
}
//----------------------------------------------------------------------------------------------------------------------
int ProgramThread::threadNumGenerator = 1;
//----------------------------------------------------------------------------------------------------------------------
int ProgramThread::thread_num_generate()
{
	return ProgramThread::threadNumGenerator++;
}
//----------------------------------------------------------------------------------------------------------------------
int ProgramThread::start_thread()
{
    pthread_attr_t thread_attr;
	struct sched_param thread_schedparam;
	int32_t ret;
	ret = pthread_attr_init (&thread_attr);
	int schedtype = SCHED_OTHER;
	ret = pthread_attr_setschedpolicy(&thread_attr, schedtype);
	uint32_t iprio=THREAD_PRIORITY_DEFAULT;
	char* prio = getenv("THREAD_PRIORITY");
	if(prio)
		iprio=atoi(prio);
	thread_schedparam.__sched_priority = iprio?iprio:THREAD_PRIORITY_DEFAULT;
	ret = pthread_attr_setschedparam (&thread_attr, &thread_schedparam);
	ret = pthread_create(&thread, &thread_attr, _entry_func, this);
	pthread_setschedparam(thread, schedtype, &thread_schedparam);
	if(threadname != "")
	{
		ret = pthread_setname_np(thread, threadname.substr(0,14).c_str());
	}
	return (ret == 0);
}
//----------------------------------------------------------------------------------------------------------------------
int ProgramThread::start_periodic_timer()
{
	int res = 0;
	tmrFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if(tmrFD < 0)
		return -1;
	struct itimerspec tmrVal = {0};
	tmrVal.it_value.tv_sec = tmrVal.it_interval.tv_sec = ThreadSleepNS / 1000000000;
	tmrVal.it_value.tv_nsec = tmrVal.it_interval.tv_nsec = ThreadSleepNS % 1000000000;
	res = timerfd_settime(tmrFD, 0, &tmrVal, NULL);
	if(!res)
		add_pollable_handler(tmrFD, EPOLLIN, &ProgramThread::PeriodicTask, this);
	return res;
}
//----------------------------------------------------------------------------------------------------------------------
void ProgramThread::join_thread()
{
	stop=1;
	if(thread)
		pthread_join(thread, NULL);
}
//----------------------------------------------------------------------------------------------------------------------
void ProgramThread::init_module()
{
	if(epollFD < 0)
	{
		//need some error processing
		return;
	}
	if(ThreadSleepNS)
		start_periodic_timer();
	add_pollable_handler(inMQueue.fd(), EPOLLIN, &ProgramThread::process_message, this);
    inMQueue.start();
	start_thread();
}
//----------------------------------------------------------------------------------------------------------------------
void ProgramThread::PeriodicTask()
{
	uint64_t expirations;
	read(tmrFD, &expirations, sizeof(expirations));
	pthread_mutex_lock(&cmdMut);
    thread_job();
	pthread_mutex_unlock(&cmdMut);
}
//----------------------------------------------------------------------------------------------------------------------
void ProgramThread::thread_run()
{
	stop = 0;
	sigset_t signal_mask;
	sigfillset(&signal_mask);
	pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
	thread_pre_loop();
	while(!stop)
	{
		struct epoll_event ev;
		if(epoll_wait(epollFD, &ev, 1, -1) < 0) {
			int err = errno;
			if(err != EINTR)
			{
				//и чо делать?
			}
			continue;
		}
		std::function<void(void)> *fn = (std::function<void(void)>*) ev.data.ptr;
		(*fn)();
	}
}
//----------------------------------------------------------------------------------------------------------------------
void ProgramThread::process_message()
{
	int processed=0;
    std::unique_ptr<InterModuleMessage> packet=nullptr;
    while(processed < eventsAtOnce && (packet = inMQueue.pop()) != nullptr);
}
//----------------------------------------------------------------------------------------------------------------------