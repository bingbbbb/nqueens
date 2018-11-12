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
    //string s = ConfigLoad::getIns()->getValue("listen_num");
    //std::cout << s << std::endl;
    _LOG(Level::INFO, {"MLog test", WHERE});
    _LOG(Level::INFO, {"MLog test", WHERE});
    _LOG(Level::INFO, {"MLog test", WHERE});

    {
        int n = std::stoi(ConfigLoad::getIns()->getValue("n"));
        int mainThread = std::stoi(ConfigLoad::getIns()->getValue("mainThread"));
        MainPcLoop loop(n, mainThread);
        //loop.waitforConnect();
        time_t tm;
        tm = time(0);
        //Nqueens* nqueens = loop.getNqueens();
        loop.genTasks(std::stoi(ConfigLoad::getIns()->getValue("taskLevel")));
        //nqueens->calSumOfTasks();
        loop.loop();
        //loop.start();
        uint64_t sum = loop.getSum();
        printf("共有%ld种排列, 计算时间%d秒 \n", sum, (int) (time(0) - tm));
    }
    STOP_LOG();
}