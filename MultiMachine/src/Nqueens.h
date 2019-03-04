#ifndef NQUEENS_H
#define NQUEENS_H

#include <vector>
#include <stdint.h>             /* Definition of uint64_t */
#include "Buffer.h"

namespace nqueens
{
union longCharArr
{
    uint64_t long_;
    char charArr[8];
};
union intCharArr
{
    uint32_t int_;
    char charArr[4];
};
class Nqueens
{
public:
    Nqueens(int n);
    ~Nqueens();
    void genTasks(int taskLevel);
    void calSumOfTasks();
    const uint64_t getSum(){return sum_;};
    bool addTasks(int n, Nqueens& nqueens);
    bool isEmpty() {return tasks_.empty();}
    void clear();
    void addSum(uint64_t sum) { sum_ += sum;}
    //MainPcLoop用到的函数
    uint64_t readSumFromFd(int fd);
    void taskTowrBuf();
    void writeTaskToFd(int fd);
    bool needWr();
    //otherPcLoop用到的函数
    int readTaskBufFromFd(int fd);
    void rdBufToTask();
    void writeSumToFd(int fd);


    Nqueens(const Nqueens &) = delete;
    const Nqueens &operator=(const Nqueens &) = delete;
    
private:
    void genTasksSub(uint32_t row, uint32_t ld, uint32_t rd, int i);
    void calSum(uint32_t row, uint32_t ld, uint32_t rd);
    int taskLevel_;
    uint64_t sum_;
    int queenNum_;
    uint32_t upperlim_;
    std::vector<std::vector<uint32_t>> tasks_; 
    Buffer wrBuf_;
    Buffer rdBuf_;
    longCharArr longCharArr_;
    intCharArr intCharArr_;
    char endC_;
    char endChar_[12];
};
}
#endif