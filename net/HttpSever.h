/******************************************************************
*File Name: HttpSever.h
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Mon 01 Mar 2021 05:19:52 AM PST
***************************************************************/
#pragma once
#include <memory>
#include <map>
#include "TcpServer.h"
#include "HttpState.h"

struct HttpData;
class HttpSever
{
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

    HttpSever(EventLoop* loop, int threadNum, int port = 80, std::string name = "HttpSever");
    void onMessage(const TcpConnectionPtr& conn, Buffer* buff);
    ~HttpSever() {}

private:
    using HttpDataSptr = std::shared_ptr<HttpData>;
    using TimerWptr = std::weak_ptr<Timer>;

    LINE_STATUS parse_line(Buffer& buff);
    HTTP_CODE parse_request_line(HttpData& httpdata, Buffer& buff, CHECK_STATE& checkstate);
    HTTP_CODE parse_request_header(HttpData& httpdata, Buffer& buff, CHECK_STATE& checkstate);
    HTTP_CODE parse_request_content(HttpData& httpdata, Buffer& buff);
    HTTP_CODE parse_main(HttpData& httpdata, Buffer& buff, CHECK_STATE& checkstate);
    HTTP_CODE do_request(HttpData& httpdata); 

    void removeHttpConn(const TcpConnectionPtr& conn)
    {
        https_.erase(conn->name());
    }
    /* void do_http(); */

    std::shared_ptr<TcpServer> tcpServer_;
    std::map<std::string, TimerWptr> https_;
};

