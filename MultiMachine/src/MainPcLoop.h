#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <iostream>
#include <sys/types.h>          //pid_t
#include <unistd.h>             //syscall()
#include <sys/syscall.h>        //SYS_gettid
#include <sys/eventfd.h>        //eventfd()
#include <sys/epoll.h>          //epoll_create()
#include <map>
#include "Nqueens.h"
#include "CalThread.h"

#define MaxEvents 20

namespace nqueens
{
class MainPcLoop
{
public:
    //typedef boost::function<void()> EventCallback;
    MainPcLoop();
    ~MainPcLoop();
    void waitforConnect();
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
    int getEpFd() {return epFd_;}
    void start();
    void genTasks(int taskLevel);
    uint64_t getSum();
    void set_non_blocking(int fd);
    
    
    MainPcLoop(const MainPcLoop &) = delete;
    const MainPcLoop &operator=(const MainPcLoop &) = delete;

private:
    void abortNotInLoopThread();
    bool looping_;
    bool quit_;
    const pid_t threadId_;
    int epFd_;
    std::unique_ptr<struct epoll_event[]> pEvents_;
    struct epoll_event event_;
    Nqueens nqueens_;
    std::vector<std::shared_ptr<CalThread>> calThreadPool_;
    std::unordered_map<int, std::shared_ptr<CalThread>> fd2Thread_;   //其他线程的fd
    //std::vector<int> otherPcFd_;
    std::unordered_map<int, std::shared_ptr<Nqueens>> fd2nqueens_;    //其他机器的fd
    size_t livedThreadNum_;
};
}
#endif