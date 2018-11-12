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
    void calSumOfTasks();
    const uint64_t getSum(){return sum_;};
    bool addTasks(int n, Nqueens& nqueens);
    bool isEmpty() {return tasks_.empty();}
    void clear();
    void addSum(uint64_t sum) { sum_ += sum;}
    
    Nqueens(const Nqueens &) = delete;
    const Nqueens &operator=(const Nqueens &) = delete;
    
private:
    void genTasksSub(long row, long ld, long rd, int i);
    void calSum(long row, long ld, long rd);
    int taskLevel_;
    uint64_t sum_;
    int queenNum_;
    long upperlim_;
    std::vector<std::vector<long>> tasks_; 

};
}
#endif