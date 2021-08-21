#ifndef PROGRAM_THREAD_H
#define PROGRAM_THREAD_H
//----------------------------------------------------------------------------------------------------------------------
#include <stdint.h>
#include <memory>
#include <functional>
#include <list>
//----------------------------------------------------------------------------------------------------------------------
#include "chanlib_export.h"
//----------------------------------------------------------------------------------------------------------------------
#ifndef DEFAULT_THREAD_PRIORITY
#define DEFAULT_THREAD_PRIORITY 4
#endif
#ifndef DEFAULT_THREAD_REALTIME
#define DEFAULT_THREAD_REALTIME 0
#endif
#ifndef DEFAULT_THREAD_SLEEP_NS
#define DEFAULT_THREAD_SLEEP_NS 10000000
#endif
#define DECMILLION 1000000
#define DECBILLION 1000000000
#define PROCESS_PACKETS_AT_ONCE 1000
//----------------------------------------------------------------------------------------------------------------------
#ifdef __cplusplus/////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class InterModuleMessage
{
public:
	InterModuleMessage(){}
	virtual ~InterModuleMessage(){}
    uint32_t code;
};
//----------------------------------------------------------------------------------------------------------------------
class ProgramThread
{
public:
	ProgramThread(uint64_t SleepNS=DEFAULT_THREAD_SLEEP_NS):inMQueue(false),ThreadSleepNS(SleepNS){int32_t ret; 
													pthread_mutexattr_t		cmdMutAttr;
													ret = pthread_mutexattr_init (&cmdMutAttr);
													ret = pthread_mutexattr_settype(&cmdMutAttr, PTHREAD_MUTEX_NORMAL);
													ret = pthread_mutex_init (&cmdMut, &cmdMutAttr);
													epollFD = epoll_create1(EPOLL_CLOEXEC);}
	virtual ~ProgramThread(){if(epollFD >= 0)close(epollFD);if(tmrFD >= 0)close(tmrFD);}
	virtual void thread_run();
	virtual void thread_job(){}
	virtual void thread_pre_loop(){}
	virtual void process_message();
	virtual void init_module();
	virtual void join_thread();
	static int threadNumGenerator;
	static int thread_num_generate();
	uint32_t EpollErrors=0;
	uint32_t EpollEvents=0;
	int stop = 0;
	pthread_mutex_t			cmdMut;
	MessageQueue<std::unique_ptr<InterModuleMessage>> inMQueue;
	template <typename F, typename O, typename ...A>
	void add_pollable_handler(int fd, uint32_t events, F func, O obj, A... arg);
protected:
	virtual int start_thread();
	virtual void PeriodicTask();
	static void * _entry_func(void *This) {((ProgramThread *)This)->thread_run(); return NULL;}
	pthread_t thread;
	string threadname="";
	uint64_t ThreadSleepNS=DEFAULT_THREAD_SLEEP_NS;
	int epollFD=-1;
	int tmrFD=-1;
	list<function<void(void)>> epollFn;	
};
//----------------------------------------------------------------------------------------------------------------------
template <typename F, typename O, typename ...A>
void ProgramThread::add_pollable_handler(int fd, uint32_t events, F func, O obj, A... arg)
{
	int res=0;
	struct epoll_event ev;
	ev.events = events;
	ev.data.ptr = &(*epollFn.insert(epollFn.end(), std::bind(func, obj, arg...)));
	if((res = epoll_ctl(epollFD, EPOLL_CTL_ADD, fd, &ev)) < 0)
	{
        //здесь могла быть ваша реклама
	}
}
//----------------------------------------------------------------------------------------------------------------------
#endif/*__cplusplus*///////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------------------------------
#endif/*PROGRAM_MODULE_H*/
