#include <sys/eventfd.h>        //eventfd()
#include <assert.h>             //assert()
#include <unistd.h>             //write()
#include <sys/epoll.h>          //epoll_ctl() epoll_event

#include "CalThread.h"
using namespace nqueens;

CalThread::CalThread(int n, int epFd)
    :eventFd_(eventfd(0, EFD_NONBLOCK)),
    epFd_(epFd),
    nqueens_(n),
    looping_(false),
    quit_(false)
{
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = eventFd_;
    int ret = epoll_ctl(epFd_, EPOLL_CTL_ADD, eventFd_, &ev);
    if (ret != 0)
    {
        throw std::runtime_error("epoll add listen fd fail");
    }
    nqueens_.clear();
}

CalThread::~CalThread()
{
    close(eventFd_);
}

void CalThread::CalLoop()
{   
    threadId_ = static_cast<pid_t>(::syscall(SYS_gettid));
    printf("CalThread %d start \n", threadId_);
    assert(!looping_);
    looping_ = true;
    quit_ = false;
    uint64_t sum = 0;

    while (!quit_)
    {
        //if (!nqueens_.isEmpty())
        {
            std::unique_lock<std::mutex> lck(mutex_);
            cond_.wait(lck, [this] { return !isEmpty()||quit_;});
            if (quit_)
                break;
            //std::lock_guard<std::mutex> guard(mutex_);
            nqueens_.calSumOfTasks();
            sum = nqueens_.getSum();
            //printf("sum: %ld \n", sum);
        
            int s = ::write(eventFd_, &sum, sizeof(uint64_t));
            nqueens_.clear();
            if (s != sizeof(uint64_t))
                throw std::runtime_error("write error in CalLoop()");
        }
    }
    printf("quit CalThread %d \n", threadId_);
    looping_ = false;
}

void CalThread::start()
{
    pThread_ = std::make_shared<std::thread>(
        std::bind(&CalThread::CalLoop, shared_from_this()));
}

bool CalThread::addTasks(int n, Nqueens& mainThreadNqueens)
{
    bool ret;
    {
        std::unique_lock<std::mutex> lck(mutex_);
        //nqueens_.clear();
        ret = nqueens_.addTasks(n, mainThreadNqueens);
        cond_.notify_one();
        //printf("add Task\n");
    }
    return ret;
}

void CalThread::quit()
{
    {
        //std::unique_lock<std::mutex> lck(mutex_);
        cond_.notify_one();
    }
    quit_ = true;
    if (pThread_->joinable())
        pThread_->join();
}