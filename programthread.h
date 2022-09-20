#ifndef PROGRAM_THREAD_H
#define PROGRAM_THREAD_H
//----------------------------------------------------------------------------------------------------------------------
#include <stdint.h>
#include <memory>
#include <functional>
#include <list>
//----------------------------------------------------------------------------------------------------------------------
#include "programthread_config.h"
#include "MessageQueue.h"
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
	ProgramThread();
	virtual ~ProgramThread();
	virtual void thread_run();
	virtual void thread_job(){}
	virtual void thread_pre_loop(){}
	virtual void process_message();
	virtual void init_module();
	virtual void join_thread();
	static int threadNumGenerator;
	uint32_t EpollErrors=0;
	uint32_t EpollEvents=0;
	int stop = 0;
	pthread_mutex_t			cmdMut;
	MessageQueue<std::unique_ptr<InterModuleMessage>> inMQueue;
	template <typename F, typename O, typename ...A>
	void add_pollable_handler(int fd, uint32_t events, F func, O obj, A... arg);
protected:
	static int thread_num_generate();
	virtual int start_thread();
	virtual void PeriodicTask();
	static void * _entry_func(void *This) {((ProgramThread *)This)->thread_run(); return NULL;}
	pthread_t thread;
	string threadname="";
	uint64_t ThreadSleepNS;
	uint32_t eventsAtOnce=0;
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
#endif/*PROGRAM_THREAD_H*/
