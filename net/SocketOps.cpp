/******************************************************************
*File Name: /home/cxz/code/mywork/net/SocketOps.cpp
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Sun 21 Feb 2021 05:29:36 AM PST
***************************************************************/
#include "SocketOps.h"
#include "../base/log.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

const int LISTENQUEUE = 1024;

int socket_bind_listend_noblock(uint16_t port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if(sockfd < 0)
        log_error_fatal("create socket failed\n");

    struct sockaddr_in addr;
    bzero(&addr, sizeof addr);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    int ret = bind(sockfd, (struct sockaddr*)&addr, sizeof addr);
    if(ret < 0)
        log_error_fatal("socket bind failed\n");

    ret = listen(sockfd, LISTENQUEUE);
    if(ret < 0)
        log_error_fatal("socket listen failed\n");
    return sockfd;
}

std::string sock_ntop_ipv4(const struct sockaddr_in& addr)
{
    char retstr[128];
    char portstr[8];
    if(inet_ntop(AF_INET, &addr.sin_addr, retstr, sizeof retstr) == nullptr)
    {
        return nullptr;
    }
    int port = ntohs(addr.sin_port);
    if(port != 0)
    {
        snprintf(portstr, sizeof portstr, ":%d",port);
        strcat(retstr, portstr);
    }
    return retstr;
}
