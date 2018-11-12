#include <string.h>         //memset()
#include <arpa/inet.h>      //inet_pton()
#include <unistd.h>         //close()
#include "otherPcLoop.h"
#include "ConfigLoad.h"
#include "MLog.h"

using namespace nqueens;

otherPcLoop::otherPcLoop()
    :sockfd_(socket(AF_INET, SOCK_STREAM, 0))
{

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
}