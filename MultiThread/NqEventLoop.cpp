#include <assert.h>     //assert()

#include "NqEventLoop.h"

using namespace nqueens;
__thread NqEventLoop* t_loopInThisThread = nullptr;

NqEventLoop::NqEventLoop(int n, int threadNum)
    :looping_(false),
    quit_(false),
    threadId_(static_cast<pid_t>(::syscall(SYS_gettid))),
    eventFd_(eventfd(0, EFD_NONBLOCK)),
    epFd_(epoll_create(MaxEvents)),
    pEvents_(new struct epoll_event[MaxEvents]),
    nqueens_(n),
    calThreadPool_(threadNum)
{
    printf("EventLoop created 0x%p in thread %d \n", this, threadId_);
    //std::cout << "EventLoop created " << this << " in thread " << threadId_;
    if (t_loopInThisThread)
    {
        printf("Another EventLoop 0x%p exists in this thread %d \n", 
            t_loopInThisThread, threadId_);
        throw std::runtime_error("runtime_error in Eventloop()");
    }
    else
    {
        t_loopInThisThread = this;
    }
    for (auto &calThread : calThreadPool_)
    {
        calThread = std::make_shared<CalThread>(n, epFd_);
        fd2Thread_[calThread->getEventFd()] = calThread;
        //calThread->start();
    }
    
}

NqEventLoop::~NqEventLoop()
{
    assert(!looping_);
    t_loopInThisThread = nullptr;
    close(eventFd_);
    close(epFd_);
}

void NqEventLoop::start()
{
    for (auto &calThread : calThreadPool_)
    {
        calThread->addTasks(50, nqueens_);
        calThread->start();
    }
}

void NqEventLoop::loop()
{
    start();
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    int nFdNum, fd;
    auto iter = fd2Thread_.begin();
    uint64_t sum;
    ssize_t s;
    size_t livedThreadNum = fd2Thread_.size();

    while (!quit_)
    {
        nFdNum = epoll_wait(epFd_, pEvents_.get(), MaxEvents, -1);
        if (nFdNum == -1)
            throw std::runtime_error("epoll_wait fail");
        for (int i = 0; i < nFdNum; ++i)
        {
            fd = pEvents_[i].data.fd;
            s = read(fd, &sum, sizeof(uint64_t));
            nqueens_.addSum(sum);
            if (s != sizeof(uint64_t))
                throw std::runtime_error("read fail");
            

            iter = fd2Thread_.find(fd);
            if (!nqueens_.isEmpty())
                iter->second->addTasks(10, nqueens_);
            else
            {
                iter->second->quit();
                livedThreadNum--;
                if (livedThreadNum == 0)
                    quit();
            }
        }
    }
    looping_ = false;
}

void NqEventLoop::genTasks(int taskLevel)
{
    return nqueens_.genTasks(taskLevel);
}

uint64_t NqEventLoop::getSum()
{
    return nqueens_.getSum();
}

void NqEventLoop::abortNotInLoopThread()
{
    printf("EventLoop::abortNotInLoopThread - EventLoop 0x%p \
        was created in threadId_ = %d, current thread id = %d \n", 
        this, threadId_, static_cast<pid_t>(::syscall(SYS_gettid)));
    throw std::runtime_error("runtime_error in abortNotInLoopThread()");
}