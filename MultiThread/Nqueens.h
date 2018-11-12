#ifndef NQUEENS_H
#define NQUEENS_H

#include <vector>
#include <stdint.h>             /* Definition of uint64_t */

namespace nqueens
{
class Nqueens
{
public:
    Nqueens(int n);
    ~Nqueens();
    void genTasks(int taskLevel);
    void genTasksSub(long row, long ld, long rd, int i, int taskLevel);
    //void giveTasks(Nqueens *pNqueens, int taskNum);
    void calSumOfTasks();
    void calSum(long row, long ld, long rd);
    uint64_t getSum(){return sum_;};
    bool addTasks(int n, Nqueens& nqueens);
    bool isEmpty() {return tasks_.empty();}
    void clear();
    Nqueens(const Nqueens &) = delete;
    const Nqueens &operator=(const Nqueens &) = delete;
    void addSum(uint64_t sum) { sum_ += sum;}
private:
    
    uint64_t sum_;
    int queenNum_;
    long upperlim_;
    std::vector<std::vector<long>> tasks_; 

};
}
#endif