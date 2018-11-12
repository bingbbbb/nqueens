#ifndef OTHERPCLOOP
#define OTHERPCLOOP

#include <sys/socket.h>
#include <netinet/in.h>

namespace nqueens
{
class otherPcLoop
{
public:
    otherPcLoop();
    ~otherPcLoop();
    void connectMainPc();

    otherPcLoop(const otherPcLoop &) = delete;
    const otherPcLoop &operator=(const otherPcLoop &) = delete;
private:
    int sockfd_;
    struct sockaddr_in  servaddr_;
};
}


#endif 