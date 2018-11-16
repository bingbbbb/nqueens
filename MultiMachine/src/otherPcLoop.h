#ifndef OTHERPCLOOP
#define OTHERPCLOOP

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <map>

#include "CalThread.h"
#include "Nqueens.h"

namespace nqueens
{
class otherPcLoop
{
public:
    otherPcLoop();
    ~otherPcLoop();
    void connectMainPc();
    void start();
    void loop();
    void quit() {quit_ = true;}
    void set_non_blocking(int fd);

    otherPcLoop(const otherPcLoop &) = delete;
    const otherPcLoop &operator=(const otherPcLoop &) = delete;
private:
    int sockfd_;                
    struct sockaddr_in  servaddr_;
    Nqueens nqueens_;
    bool looping_;
    bool quit_;
    const pid_t threadId_;
    int epFd_;
    std::unique_ptr<struct epoll_event[]> pEvents_;
    struct epoll_event event_;
    std::vector<std::shared_ptr<CalThread>> calThreadPool_;
    std::map<int, std::shared_ptr<CalThread>> fd2Thread_;
    size_t livedThreadNum_;
};
}


#endif 