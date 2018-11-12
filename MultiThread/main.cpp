/************************************************************
 * 
 * 主线程负责分配任务，不参与计算任务，任务由线程池中的线程计算，并将结果
 * 反馈给主线程，领取新任务，直至所有任务完成 。
 * 
*************************************************************/
#include <stdio.h>
#include <time.h>
#include <stdexcept>
#include "Nqueens.h"
#include "NqEventLoop.h"
using namespace nqueens;

int main()
{
    //throw std::runtime_error("runtime_error in main");
    {
        NqEventLoop loop(17, 6);
        //有时候会死循环
        typedef boost::function<void()> EventCallback;
        time_t tm;
        tm = time(0);
        //Nqueens* nqueens = loop.getNqueens();
        loop.genTasks(3);
        //nqueens->calSumOfTasks();
        loop.loop();
        //loop.start();
        uint64_t sum = loop.getSum();
        printf("共有%ld种排列, 计算时间%d秒 \n", sum, (int) (time(0) - tm));
    }
    
    return 0;
}

