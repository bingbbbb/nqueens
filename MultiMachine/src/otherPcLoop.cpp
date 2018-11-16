#include <string.h>         //memset()
#include <arpa/inet.h>      //inet_pton()
#include <unistd.h>         //close()
#include <sys/syscall.h>    //SYS_gettid
#include <fcntl.h>      //fcntl()

#include "otherPcLoop.h"
#include "ConfigLoad.h"
#include "MLog.h"

using namespace nqueens;

otherPcLoop::otherPcLoop()
    :sockfd_(socket(AF_INET, SOCK_STREAM, 0)),
    nqueens_(std::stoi(ConfigLoad::getIns()->getValue("n"))),
    looping_(false),
    quit_(false),
    threadId_(static_cast<pid_t>(::syscall(SYS_gettid))),
    epFd_(epoll_create(20)),
    pEvents_(new struct epoll_event[20]),
    calThreadPool_(std::stoi(ConfigLoad::getIns()->getValue("nThread_otherPc"))),
    livedThreadNum_(0)
{
    nqueens_.clear();
    for (auto &calThread : calThreadPool_)
    {
        calThread = std::make_shared<CalThread>(std::stoi(ConfigLoad::getIns()->getValue("n")), epFd_);
        fd2Thread_[calThread->getEventFd()] = calThread;
        //calThread->start();
    }
}

otherPcLoop::~otherPcLoop()
{
    close(sockfd_);
}

void otherPcLoop::connectMainPc()
{
    memset(&servaddr_, 0, sizeof(servaddr_));
    servaddr_.sin_family = AF_INET;
    servaddr_.sin_port = htons(std::stoi(ConfigLoad::getIns()->getValue("port")));
    inet_pton(AF_INET, ConfigLoad::getIns()->getValue("ip").c_str(), &servaddr_.sin_addr);
    if (!connect(sockfd_, (struct sockaddr *)&servaddr_, sizeof(servaddr_)))
        printf("connect mainPc success\n");
    else
        RUNTIME_ERROR();
    set_non_blocking(sockfd_);
    event_.data.fd = sockfd_;
    event_.events = EPOLLIN | EPOLLET;
    epoll_ctl(epFd_, EPOLL_CTL_ADD, sockfd_, &event_);
}

void otherPcLoop::start()
{
    for (auto &calThread : calThreadPool_)
        calThread->start();
}

void otherPcLoop::loop()
{
    start();
    looping_ = true;
    quit_ = false;
    int nFdNum, fd;
    
    auto iter = fd2Thread_.begin();
    uint64_t sum;
    ssize_t s;

    while(!quit_)
    {
        nFdNum = epoll_wait(epFd_, pEvents_.get(), 20, -1);
        if (nFdNum == -1)
            RUNTIME_ERROR();
        for(int i=0;i<nFdNum;i++)
        {
            fd = pEvents_[i].data.fd;
            if (fd == sockfd_)//收到主计算机发来的信息
            {
                if (pEvents_[i].events & EPOLLIN)
                {
                    //printf("get buffer from mainPc\n");
                    int ret = nqueens_.readTaskBufFromFd(fd);
                    if (ret == 0)
                        {//主进程直接关闭不知道会不会进入这里
                            for (auto &calThread : calThreadPool_)
                                calThread->quit();
                            quit();
                            break;
                        }
                    else if (ret == -1)
                        continue;
                    else    //ret == 1
                    {
                        nqueens_.rdBufToTask();
                        //nqueens_.calSumOfTasks();
                        //nqueens_.writeSumToFd(fd);
                        //nqueens_.clear();
                        for (auto &calThread : calThreadPool_)
                        {
                            if (!nqueens_.isEmpty())
                            {
                                calThread->addTasks(std::stoi(ConfigLoad::getIns()->getValue("pcTasksThreadNumAdd")), nqueens_);
                                livedThreadNum_++;
                            }
                        }
                    }
                }
            }
            else
            {
                s = read(fd, &sum, sizeof(uint64_t));
                nqueens_.addSum(sum);
                if (s != sizeof(uint64_t))
                    RUNTIME_ERROR();
                
                iter = fd2Thread_.find(fd);
                if (!nqueens_.isEmpty())
                    iter->second->addTasks(10, nqueens_);
                else
                {
                    //iter->second->quit();
                    livedThreadNum_--;
                    if (livedThreadNum_ == 0)
                    {
                        nqueens_.writeSumToFd(sockfd_);
                        nqueens_.clear();
                    }
                }
            }
        }
    }
}

void otherPcLoop::set_non_blocking(int fd)
{
        int flag;
        flag = fcntl(fd, F_GETFL);
        flag  |= O_NONBLOCK;
        fcntl(fd, F_SETFL, flag);
}