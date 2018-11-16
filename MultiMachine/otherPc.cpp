#include <iostream>
#include <string>
#include "src/ConfigLoad.h"
#include "src/otherPcLoop.h"
using namespace nqueens;
using std::string;
int main()
{
    otherPcLoop loop;
    loop.connectMainPc();
    loop.loop();
}