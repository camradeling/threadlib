#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
//----------------------------------------------------------------------------------------------------------------------
#include "programthread.h"
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
	thread_schedparam.__sched_priority = DEFAULT_THREAD_PRIORITY;
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
		//
	}
	else
	{
		tmrFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
		if(tmrFD < 0)
		{
			//
		}
		else
		{
			struct itimerspec tmrVal = {0};
			tmrVal.it_value.tv_sec = tmrVal.it_interval.tv_sec = ThreadSleepNS / 1000000000;
			tmrVal.it_value.tv_nsec = tmrVal.it_interval.tv_nsec = ThreadSleepNS % 1000000000;

			if(timerfd_settime(tmrFD, 0, &tmrVal, NULL) < 0)
			{
				//
			}
			else
			{
				add_pollable_handler(tmrFD, EPOLLIN, &ProgramThread::PeriodicTask, this);
			}
		}
	}
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
    while(processed < PROCESS_PACKETS_AT_ONCE && (packet = inMQueue.pop()) != nullptr);
}
//----------------------------------------------------------------------------------------------------------------------