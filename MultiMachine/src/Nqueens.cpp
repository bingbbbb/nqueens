#include <stdio.h>
#include <sys/socket.h>     //recv()
#include <unistd.h>         //write()
#include<algorithm>         //min()
#include "Nqueens.h"
#include "MLog.h"

using namespace nqueens;

Nqueens::Nqueens(int n):
        sum_(0),
        queenNum_(n),
        upperlim_(1),
        tasks_({{0, 0, 0}}),
        wrBuf_(50),
        rdBuf_(50),
        endC_(static_cast<char>(0xFF))
{
    upperlim_ = (upperlim_ << n) - 1;
    for (int i = 0;i < 12;++i)
        endChar_[i] = endC_;
};

Nqueens::~Nqueens()
{
    
}

void Nqueens::genTasksSub(uint32_t row, uint32_t ld, uint32_t rd, int i)  
{  
    if (i < taskLevel_)  
    {
        uint32_t pos = upperlim_ & ~(row | ld | rd);   
        while (pos)    
        {
            uint32_t p = pos & -pos;                                                
            pos -= p;                             
            genTasksSub(row + p, (ld + p) << 1, (rd + p) >> 1, i + 1);                                
        }  
    }  
    else     
    {  
        //printf("row:%ld  ld:%ld  rd:%ld \n", row, ld, rd); 
        tasks_.push_back({row, ld, rd});
    }  
}  

void Nqueens::genTasks(int taskLevel)
{
    tasks_.clear();
    taskLevel_ = taskLevel;
    genTasksSub(0, 0, 0, 0);
}

void Nqueens::calSumOfTasks()
{
    for(std::vector<uint32_t> task : tasks_)
    {
        calSum(task[0], task[1], task[2]);
        //printf("row:%ld  ld:%ld  rd:%ld sum:%ld \n", task[0], task[1], task[2], sum_); 
    }
}

void Nqueens::calSum(uint32_t row, uint32_t ld, uint32_t rd)
{
    if (row != upperlim_)  
    {
        uint32_t pos = upperlim_ & ~(row | ld | rd);   
        while (pos)
        { 
            uint32_t p = pos & -pos;
            pos -= p;                             
            calSum(row + p, (ld + p) << 1, (rd + p) >> 1);                                
        }  
    }
    else     
    {
        sum_++;  
    }
}

//nqueens如果有n个以上的任务数，就增加到当前对象中，并返回ture，不够就全部增加进来并返回false
bool Nqueens::addTasks(int n, Nqueens& nqueens)
{   
    //printf("main tasksNum: %ld \n", nqueens.tasks_.size());
    //printf("calThread tasksNum: %ld \n", tasks_.size());
    if (nqueens.tasks_.size() > n)
    {
        tasks_.insert(tasks_.begin(), nqueens.tasks_.end()-n-1, nqueens.tasks_.end()-1);
        nqueens.tasks_.erase(nqueens.tasks_.end()-n-1, nqueens.tasks_.end()-1);
        //printf("after \n");
        //printf("main tasksNum: %ld \n", nqueens.tasks_.size());
        //printf("calThread tasksNum: %ld \n", tasks_.size());
        return true;
    }
    else
    {
        tasks_.swap(nqueens.tasks_);
        return false;
    }
    
}

void Nqueens::clear()
{
    tasks_.clear(); 
    sum_ = 0;
}

uint64_t Nqueens::readSumFromFd(int fd)
{
    printf("get result from otherPc[%d]\n", fd);
    int ret = 0;
    char m_cTmpBuf[50];
    ret = recv(fd, m_cTmpBuf, sizeof(m_cTmpBuf), MSG_DONTWAIT);
    if (ret != 8)
        RUNTIME_ERROR();
    for(int i=0;i<8;i++)
        longCharArr_.charArr[i] = m_cTmpBuf[i];
    return longCharArr_.long_;
}

void Nqueens::taskTowrBuf()
{
    //printf("tasks_.size(): %d\n", tasks_.size());
    for(auto foreach_1 : tasks_)
    {
        for(auto foreach_2 : foreach_1)
        {
            intCharArr_.int_ = foreach_2;
            wrBuf_.append(intCharArr_.charArr, 4);
        }
    }
    wrBuf_.append(endChar_, 12);
    //printf("wrBuf_.size(): %d \n", wrBuf_.size());
}

void Nqueens::writeTaskToFd(int fd)
{
    int ret = 0;
    //size_t oriSize = m_wrBuf.size();
    ret = ::write(fd, wrBuf_.GetPtr(), std::min(wrBuf_.size(), wrBuf_.sizeEnd()));
    while (ret > 0)
    {
        wrBuf_.discard(ret); //已写入ret字节，可以去掉
        if (!wrBuf_.size())  //若已经写完，终止循环
            break;
        ret = ::write(fd, wrBuf_.GetPtr(), std::min(wrBuf_.size(), wrBuf_.sizeEnd()));
    }
    if (ret < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
        RUNTIME_ERROR();
    printf("write %zu tasks to otherPc[%d]\n", tasks_.size(), fd);
}

bool Nqueens::needWr()
{
    return !!wrBuf_.size();
}

/**
 * 0关闭连接，1正常读完完整任务，-1正常读完非完整任务
 */
int Nqueens::readTaskBufFromFd(int fd)
{
    int ret = 0;
    char m_cTmpBuf[4096];
    ret = recv(fd, m_cTmpBuf, sizeof(m_cTmpBuf), MSG_DONTWAIT);
    while (ret > 0)
    {
        rdBuf_.append(m_cTmpBuf, ret);
        ret = recv(fd, m_cTmpBuf, sizeof(m_cTmpBuf), MSG_DONTWAIT);
    }
    if (ret == 0) //连接关闭
        return 0;
    else if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
        {
            //结束符
            const char *pBuf = rdBuf_.GetStartPtr();
            size_t bufSize = rdBuf_.size();
            for(int i=0;i<12;i++)
            {
                if (pBuf[(rdBuf_.writerIndex()+rdBuf_.capacity()-12+i) % rdBuf_.capacity()]!=endChar_[i])
                    return -1;
            }
            return 1;
        }
    RUNTIME_ERROR();
}

void Nqueens::rdBufToTask()
{
    //clear();
    const char *pBuf = rdBuf_.GetStartPtr();
    size_t bufSize = rdBuf_.size();
    //printf("rdBuf_.size(): %ld \n", bufSize);
    int taskNum = (bufSize - 12) / 12;//应该是刚好整除的
    for(int i=0;i<taskNum;i++)
    {
        uint32_t row, ld, rd;
        for(int j=0;j<4;j++)
        {
            intCharArr_.charArr[j] = pBuf[(rdBuf_.readerIndex()+j) % rdBuf_.capacity()];
        }
        row = intCharArr_.int_;
        for(int j=0;j<4;j++)
        {
            intCharArr_.charArr[j] = pBuf[(rdBuf_.readerIndex()+j+4) % rdBuf_.capacity()];
        }
        ld = intCharArr_.int_;
        for(int j=0;j<4;j++)
        {
            intCharArr_.charArr[j] = pBuf[(rdBuf_.readerIndex()+j+8) % rdBuf_.capacity()];
        }
        rd = intCharArr_.int_;
        tasks_.push_back({row, ld, rd});
        rdBuf_.discard(12);
    }
    rdBuf_.discard(12);
    printf("get %zu tasks from mainPc\n", tasks_.size());
}

void Nqueens::writeSumToFd(int fd)
{
    longCharArr_.long_ = sum_;
    char charArr[8];
    for(int i=0;i<8;i++)
        charArr[i] = longCharArr_.charArr[i];
    int ret = ::write(fd, charArr, sizeof(charArr));
    if (ret < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
        RUNTIME_ERROR();
    printf("write sum to mainPc\n");
    //clear();
}