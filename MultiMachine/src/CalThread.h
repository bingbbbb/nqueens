#ifndef CALTHREAD_H
#define CALTHREAD_H

#include <thread>
#include <mutex>            //std::mutex
#include <sys/syscall.h>    //SYS_gettid
#include <atomic>
#include <condition_variable>
#include "Nqueens.h"

namespace nqueens
{
class CalThread : public std::enable_shared_from_this<CalThread>
{
public:
    CalThread(int n, int epFd);
    ~CalThread();
    void start();
    void CalLoop();
    bool addTasks(int n, Nqueens& mainThreadNqueens);
    void quit();
    int getEventFd() {return eventFd_;}
    bool isEmpty() {return nqueens_.isEmpty();}
private:
    int eventFd_;
    int epFd_;
    std::shared_ptr<std::thread> pThread_;
    Nqueens nqueens_;
    bool looping_;
    std::atomic<bool> quit_;
    std::mutex mutex_;
    pid_t threadId_;
    std::condition_variable cond_;

};
}
#endif