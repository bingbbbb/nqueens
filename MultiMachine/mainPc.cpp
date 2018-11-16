#include <iostream>
#include <string>
#include "src/ConfigLoad.h"
#include "src/MLog.h"
#include "src/Nqueens.h"
#include "src/MainPcLoop.h"
using namespace nqueens;
using std::string;

int main()
{

    {
        MainPcLoop loop;
        loop.genTasks(std::stoi(ConfigLoad::getIns()->getValue("taskLevel")));
        loop.waitforConnect();
        time_t tm;
        tm = time(0);
        //Nqueens* nqueens = loop.getNqueens();
        //nqueens->calSumOfTasks();
        loop.loop();
        //loop.start();
        uint64_t sum = loop.getSum();
        printf("共有%ld种排列, 计算时间%d秒 \n", sum, (int) (time(0) - tm));
    }
    STOP_LOG();
    return 0;
}