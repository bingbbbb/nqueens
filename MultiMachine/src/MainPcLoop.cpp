#include <assert.h>     //assert()
#include <fcntl.h>      //fcntl()
#include <netinet/in.h>
#include <string.h>
#include "MLog.h"
#include "MainPcLoop.h"
#include "ConfigLoad.h"

using namespace nqueens;
__thread MainPcLoop* t_loopInThisThread = nullptr;

MainPcLoop::MainPcLoop()
    :looping_(false),
    quit_(false),
    threadId_(static_cast<pid_t>(::syscall(SYS_gettid))),
    epFd_(epoll_create(MaxEvents)),
    pEvents_(new struct epoll_event[MaxEvents]),
    nqueens_(std::stoi(ConfigLoad::getIns()->getValue("n"))),
    calThreadPool_(std::stoi(ConfigLoad::getIns()->getValue("nThread"))),
    livedThreadNum_(0)
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
        calThread = std::make_shared<CalThread>(std::stoi(ConfigLoad::getIns()->getValue("n")), epFd_);
        fd2Thread_[calThread->getEventFd()] = calThread;
        //calThread->start();
    }
}

MainPcLoop::~MainPcLoop()
{
    assert(!looping_);
    t_loopInThisThread = nullptr;
    close(epFd_);
    for(auto fd2nq : fd2nqueens_)
        close(fd2nq.first);
}

void MainPcLoop::waitforConnect()
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  
    //initiate the socket address structure
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family         = AF_INET;
    server_addr.sin_addr.s_addr    = htonl(INADDR_ANY);
    server_addr.sin_port           = htons(std::stoi(ConfigLoad::getIns()->getValue("port")));

    //bind the local protocol type to a socket address
    bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    //initiate the epoll
    set_non_blocking(listen_fd);
    event_.data.fd = listen_fd;
    event_.events = EPOLLIN | EPOLLET;
    epoll_ctl(epFd_, EPOLL_CTL_ADD, listen_fd, &event_);

    //start listening
    listen(listen_fd, 20);
    int trigger_num;
    socklen_t addr_len;
    struct sockaddr_in client_addr;
    while(fd2nqueens_.size()<std::stoi(ConfigLoad::getIns()->getValue("pcNum"))-1)
    {
        trigger_num = epoll_wait(epFd_, pEvents_.get(), MaxEvents, -1);
        for(int i = 0; i < trigger_num; i++)
        {
            if(pEvents_[i].data.fd  == listen_fd)
            {      
                addr_len = sizeof(sockaddr);
                int connect_fd = accept(listen_fd,(sockaddr *)&client_addr, &addr_len);
                set_non_blocking(connect_fd);
                //otherPcFd_.push_back(connect_fd);
                fd2nqueens_[connect_fd] = std::make_shared<Nqueens>(std::stoi(ConfigLoad::getIns()->getValue("n")));
                event_.data.fd = connect_fd;
                event_.events = EPOLLIN | EPOLLOUT | EPOLLET;
                epoll_ctl(epFd_, EPOLL_CTL_ADD, connect_fd, &event_);
                printf("a new connection : %d\n", connect_fd);

                /**分配初始任务
                fd2nqueens_[connect_fd]->clear();
                if (!nqueens_.isEmpty())
                {
                    fd2nqueens_[connect_fd]->addTasks(200,nqueens_);
                    livedThreadNum_++;
                }
                fd2nqueens_[connect_fd]->taskTowrBuf();
                fd2nqueens_[connect_fd]->writeTaskToFd(connect_fd);
                */
            }
        }
    }
    printf("connected finished\n");
    close(listen_fd);
}

void MainPcLoop::start()
{
    for (auto &calThread : calThreadPool_)
    {
        if (!nqueens_.isEmpty())
        {
            calThread->addTasks(std::stoi(ConfigLoad::getIns()->getValue("ctTasksNumInit")), nqueens_);
            livedThreadNum_++;
        }
        calThread->start();
        //有时候会死循环,原因是任务数量不够初始分配
    }
    for(auto iter : fd2nqueens_)
    {
        //分配初始任务
        iter.second->clear();
        if (!nqueens_.isEmpty())
        {
            iter.second->addTasks(std::stoi(ConfigLoad::getIns()->getValue("pcTasksNumInit")), nqueens_);
            livedThreadNum_++;
        }
        iter.second->taskTowrBuf();
        iter.second->writeTaskToFd(iter.first);
    }
}

void MainPcLoop::loop()
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
    //size_t livedThreadNum = fd2Thread_.size();

    while (!quit_)
    {
        nFdNum = epoll_wait(epFd_, pEvents_.get(), MaxEvents, -1);
        if (nFdNum == -1)
            throw std::runtime_error("epoll_wait fail");
        for (int i = 0; i < nFdNum; ++i)
        {
            fd = pEvents_[i].data.fd;
            auto iter_nq = fd2nqueens_.find(fd);
            if (iter_nq == fd2nqueens_.end())//收到计算线程发来的信息
            {   
                s = read(fd, &sum, sizeof(uint64_t));
                nqueens_.addSum(sum);
                if (s != sizeof(uint64_t))
                    RUNTIME_ERROR();
                

                iter = fd2Thread_.find(fd);
                if (!nqueens_.isEmpty())
                    iter->second->addTasks(std::stoi(ConfigLoad::getIns()->getValue("ctTasksNumAdd")), nqueens_);
                else
                {
                    iter->second->quit();
                    livedThreadNum_--;
                    if (livedThreadNum_ == 0)
                        quit();
                }
            }
            else    //收到其他计算机发来的信息
            {
                if (pEvents_[i].events & EPOLLIN)
                {
                    sum = iter_nq->second->readSumFromFd(fd);
                    nqueens_.addSum(sum);
                    iter_nq->second->clear();
                    if (!nqueens_.isEmpty())
                    {
                        iter_nq->second->addTasks(std::stoi(ConfigLoad::getIns()->getValue("pcTasksNumInit")),nqueens_);
                        iter_nq->second->taskTowrBuf();
                        iter_nq->second->writeTaskToFd(fd);
                    }
                    else
                    {

                        livedThreadNum_--;
                        if (livedThreadNum_ == 0)
                            quit();
                    }
                    continue;
                }
                if (pEvents_[i].events & EPOLLOUT)
                {
                    if (iter_nq->second->needWr())
                        iter_nq->second->writeTaskToFd(fd);
                }
            }
        }
    }
    looping_ = false;
}

void MainPcLoop::genTasks(int taskLevel)
{
    return nqueens_.genTasks(taskLevel);
}

uint64_t MainPcLoop::getSum()
{
    return nqueens_.getSum();
}

void MainPcLoop::set_non_blocking(int fd)
{
        int flag;
        flag = fcntl(fd, F_GETFL);
        flag  |= O_NONBLOCK;
        fcntl(fd, F_SETFL, flag);
}

void MainPcLoop::abortNotInLoopThread()
{
    printf("EventLoop::abortNotInLoopThread - EventLoop 0x%p \
        was created in threadId_ = %d, current thread id = %d \n", 
        this, threadId_, static_cast<pid_t>(::syscall(SYS_gettid)));
    throw std::runtime_error("runtime_error in abortNotInLoopThread()");
}