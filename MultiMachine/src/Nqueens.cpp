#include <stdio.h>
#include "Nqueens.h"

using namespace nqueens;
Nqueens::Nqueens(int n):
        sum_(0),
        queenNum_(n),
        upperlim_(1),
        tasks_({{0, 0, 0}})
{
    upperlim_ = (upperlim_ << n) - 1;
};

Nqueens::~Nqueens()
{
    
}

void Nqueens::genTasksSub(long row, long ld, long rd, int i)  
{  
    if (i < taskLevel_)  
    {
        long pos = upperlim_ & ~(row | ld | rd);   
        while (pos)    
        {
            long p = pos & -pos;                                                
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
    for(std::vector<long> task : tasks_)
    {
        calSum(task[0], task[1], task[2]);
        //printf("row:%ld  ld:%ld  rd:%ld sum:%ld \n", task[0], task[1], task[2], sum_); 
    }
}

void Nqueens::calSum(long row, long ld, long rd)
{
    if (row != upperlim_)  
    {
        long pos = upperlim_ & ~(row | ld | rd);   
        while (pos)
        { 
            long p = pos & -pos;
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
