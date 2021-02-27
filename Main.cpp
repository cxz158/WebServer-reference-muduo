/******************************************************************
*File Name: Main.cpp
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Sun 21 Feb 2021 03:07:37 AM PST
***************************************************************/
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstdio>
#include <memory>
#include "net/EventLoop.h"
#include "net/TcpServer.h"
#include "net/Acceptor.h"
#include "net/SocketOps.h"
#include "base/log.h"
using namespace std;

void onMessage(const TcpServer::TcpConnectionPtr& conn,Buffer* buf)
{
    printf("get %zu bytes from %s\n",buf->readableBytes(), sock_ntop_ipv4(conn->getPeerAddr()).c_str());
    conn->send(buf->getMessage());
}

int main()
{
    EventLoop mainloop;
    TcpServer http(&mainloop, 5000,4);
    http.setMessageCallback(onMessage);
    mainloop.loop();
    return 0;
}
