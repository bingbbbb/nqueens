#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <iostream>
#include <sys/types.h>          //pid_t
#include <unistd.h>             //syscall()
#include <sys/syscall.h>        //SYS_gettid
#include <boost/function.hpp>   //boost::function
#include <sys/eventfd.h>        //eventfd()
#include <sys/epoll.h>          //epoll_create()
#include <map>
#include "Nqueens.h"
#include "CalThread.h"

#define MaxEvents 20

namespace nqueens
{
class NqEventLoop
{
public:
    //typedef boost::function<void()> EventCallback;
    NqEventLoop(int n, int threadNum);
    ~NqEventLoop();
    void loop();
    void assertInLoopThread()
    {
        if (!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }
    bool isInLoopThread() const 
    {
        return threadId_ == static_cast<pid_t>(::syscall(SYS_gettid));
    }
    void quit() {quit_ = true;}
    int getEventFd() {return eventFd_;}
    int getEpFd() {return epFd_;}
    //Nqueens* getNqueens() {return &nqueens_;}
    void start();
    void genTasks(int taskLevel);
    uint64_t getSum();
    
    NqEventLoop(const NqEventLoop &) = delete;
    const NqEventLoop &operator=(const NqEventLoop &) = delete;

private:
    void abortNotInLoopThread();
    bool looping_;
    bool quit_;
    const pid_t threadId_;
    int eventFd_;
    int epFd_;
    std::unique_ptr<struct epoll_event[]> pEvents_;
    struct epoll_event event_;
    Nqueens nqueens_;
    std::vector<std::shared_ptr<CalThread>> calThreadPool_;
    std::map<int, std::shared_ptr<CalThread>> fd2Thread_;
};
}
#endif