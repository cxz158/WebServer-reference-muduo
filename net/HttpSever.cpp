/******************************************************************
*File Name: HttpSever.cpp
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Mon 01 Mar 2021 05:25:08 AM PST
***************************************************************/
#include "HttpSever.h"
#include "SocketOps.h"
#include "../base/log.h"
#include "Timer.h"
#include <sys/stat.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>

const char* ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char* error_403_title = "Forbidden";
const char* error_403_form = "You do not have permission to get file from this server.\n";
const char* error_404_title = "Not Found";
/* const char* error_404_form = "hello world!\n"; */
const char* error_404_form = "The request file was not found on this sever.\n";
const char* error_500_title = "Internal Error";
const char* error_500_form = "There was an unusual problem serving the requested file.\n";

/* 网站根目录 */
const char* doc_root = "../html";

struct HttpData
{
    static const int FILENAME_LEN = 200;
    static const int KEEPALIVE_TIME = 8000; //ms
    METHOD m_mthod;
    char* m_url;
    char m_real_file[FILENAME_LEN];
    std::string m_response;
    char* m_file_address;
    int m_file_size;
    char* m_version;
    char* m_host;
    int m_content_length;
    bool m_linger;

    HttpData():m_mthod(METHOD::GET), m_url(nullptr), m_file_address(nullptr),
            m_file_size(0), m_version(nullptr), m_content_length(0), m_linger(false)
    {
        bzero(m_real_file, sizeof m_real_file);
    }

/*     void init() */
/*     { */
/*         m_mthod = METHOD::GET; */
/*         m_url = nullptr; */
/*         bzero(m_real_file, sizeof m_real_file); */
/*         m_response.clear(); */
/*         unmap(); */
/*         m_version = nullptr; */
/*         m_host = nullptr; */
/*         m_content_length = 0; */
/*         m_linger = false; */
/*     } */

    void unmap()
    {
        if(m_file_address)
        {
            munmap(m_file_address, m_file_size);
            m_file_address = nullptr;
            m_file_size = 0;
        }
    }

    ~HttpData()
    {
        unmap();
    }    
};

HttpSever::HttpSever(EventLoop* loop, int threadNum, int port, std::string name)
    : tcpServer_(new TcpServer(loop, threadNum, port, name)),
      mutex_()
{
    tcpServer_->setMessageCallback(std::bind(&HttpSever::onMessage, this,
                                             std::placeholders::_1, std::placeholders::_2));
    tcpServer_->setConnectionCallback(std::bind(&HttpSever::removeHttpConn, this,
                                                std::placeholders::_1));
}

std::string response_status_line(int status, const char* title)
{
    return format_string("%s %d %s\r\n", "HTTP/1.1", status, title);
}

std::string response_header(const HttpData& httpdata)
{
    std::string ret;
    ret = format_string("Content-Length: %d\r\n", httpdata.m_file_size);
    ret += format_string("Connection: %s\r\n", (httpdata.m_linger) ? "keep-alive" : "close");
    ret += format_string("%s", "\r\n");
    return ret;
}

//构造消息头
bool do_response(HttpData& httpdata, HTTP_CODE ret)
{
    switch(ret)
    {
    case HTTP_CODE::INTERNAL_ERROR:
        {
            httpdata.m_response = response_status_line(500, error_500_title);
            httpdata.m_response += response_header(httpdata);
            httpdata.m_response += error_500_form;
            break;
        }
    case HTTP_CODE::BAD_REQUEST:
        {
            httpdata.m_response = response_status_line(400, error_400_title);
            httpdata.m_response += response_header(httpdata);
            httpdata.m_response += format_string("%s\r\n", error_400_form);
            break;
        }
    case HTTP_CODE::NO_REQUEST:
        {
            httpdata.m_response = response_status_line(404, error_404_title);
            httpdata.m_response += response_header(httpdata);
            httpdata.m_response += format_string("%s\r\n", error_404_form);
            break;  
        }
    case HTTP_CODE::FORBIDDEN_REQUEST:
        {
            httpdata.m_response = response_status_line(403, error_403_title);
            httpdata.m_response += response_header(httpdata);
            httpdata.m_response += format_string("%s\r\n", error_403_form);
            break;
        }
    case HTTP_CODE::FILE_REQUEST:
        {
            httpdata.m_response = response_status_line(200, ok_200_title);
            httpdata.m_response += response_header(httpdata);        
            break;
        }
    default:
        return false;
    }
    return true;
}

void HttpSever::onMessage(const TcpConnectionPtr& conn, Buffer* buff)
{
    /* log("%s\n%s", get_time().c_str(), buff->readbegin()); */
    bool keepAlive = false;
    //存在buff中的最后只有半个请求的情况，
    /* char* compeletindex = nullptr; */
    /* while(buff->readableBytes()) */
    /* { */
        HttpData httpdata;
        /* compeletindex = buff->readbegin(); */
        CHECK_STATE checksatte = CHECK_STATE::CHECK_STATE_REQUESTLINE;
        HTTP_CODE ret = parse_main(httpdata, *buff, checksatte);
        /* if(ret == HTTP_CODE::OPEN_REQUEST)  //剩余数据无法组成一个完整的请求无法继续解析 */
        /* { */
        /*      buff->retrive_to(compeletindex); */
        /*      break; */
        /* } */
        if(httpdata.m_linger)
            keepAlive = true;
        do_response(httpdata, ret);
        conn->send(httpdata.m_response);
        if(ret == HTTP_CODE::FILE_REQUEST)
        {
            conn->send(httpdata.m_file_address, httpdata.m_file_size);
        }
    /* } */
    if(!keepAlive)
    {
        conn->shutdown();
        return;
    }
    buff->init();
    auto it = https_.find(conn->name());
    std::weak_ptr<Timer> wtimer;
    if(it != https_.end())
    {
        wtimer = it->second;
        std::shared_ptr<Timer> stimer = wtimer.lock();
        if(stimer)
            stimer->disabled(); 
    }
    wtimer = conn->getLoop()->addTimer(std::bind(&TcpConnection::forceClose,conn),
                                       HttpData::KEEPALIVE_TIME); 
    {
        MutexLockGuard lock(mutex_);
        https_[conn->name()] = wtimer;
    }
}

//判断即将解析的一行数据是否符合http规范，以\r\n结尾
LINE_STATUS HttpSever::parse_line(Buffer& buff)
{
    char* tmp = buff.findLine();
    if( tmp == nullptr )
        return LINE_STATUS::LINE_INCOMPLETE;
    if( tmp == buff.readbegin() )
        return LINE_STATUS::LINE_BAD;
    if( *(tmp -1) == '\r' )
    {
        *tmp = '\0';
        *(tmp -1) = '\0';
        buff.retrive_to(tmp);
        return LINE_STATUS::LINE_OK;
    }
    else 
        return LINE_STATUS::LINE_BAD;
}

//解析请求行
HTTP_CODE HttpSever::parse_request_line(HttpData& httpdata, Buffer& buff, CHECK_STATE& checkstate)
{
    char* url = strpbrk(buff.checkPeek(), " \t");        
    if(url == nullptr)
        return HTTP_CODE::BAD_REQUEST;
    *url++ = '\0';
    char* method = buff.checkPeek();
    if(strcmp(method, "GET") == 0)
        httpdata.m_mthod = METHOD::GET;
    else if(strcmp(method, "HEAD") == 0)
        httpdata.m_mthod = METHOD::HEAD;
    else
        return HTTP_CODE::BAD_REQUEST;

    url += strspn(url, " \t");

    char* version = strpbrk(url, " \t");
    if(version == nullptr)
        return HTTP_CODE::BAD_REQUEST;
    *version++ = '\0';
    version += strspn(version, " \t");
    httpdata.m_version = version;
    
    if(strncasecmp(url, "http://", 7) == 0)
    {
        url += 7;
        url = strchr(url, '/');
    }
    httpdata.m_url = url;
    if(!httpdata.m_url || httpdata.m_url[0] !='/')
        return HTTP_CODE::BAD_REQUEST;
    checkstate = CHECK_STATE::CHECK_STATE_HEADER;
    return HTTP_CODE::NO_REQUEST;
}

//解析请求头
HTTP_CODE HttpSever::parse_request_header(HttpData& httpdata, Buffer& buff, 
                                           CHECK_STATE& checkstate)
{
    char* text = buff.checkPeek();
    //遇到空行解析完毕
    if(*buff.checkPeek() == '\0')
    {
        //若存在消息体 则继续解析消息体
        buff.checkFinish();
        if( httpdata.m_content_length != 0 )
        {
            checkstate = CHECK_STATE::CHECK_STATE_CONTENT;
            return HTTP_CODE::NO_REQUEST;
        }
        return HTTP_CODE::GET_REQUEST; //解析完毕， 获取到一个合格的请求
    }

    //是否支持长链接
    else if(strncasecmp(text, "Connection:", 11) == 0)
    {
        text += 11;
        text += strspn(text, " \t");
        if((strcasecmp(text, "keep-alive") == 0) || (strcasecmp(text, "Keep-Alive") == 0))
            httpdata.m_linger = true;
    }
    //请求体的长度是多少
    else if(strncasecmp(text, "Content-Length:", 15) == 0)
    {
        text += 15;
        text += strspn(text, " \t");
        httpdata.m_content_length = atoi(text);
    }
    //发起请求方主机名称
    else if(strncasecmp(text, "Host:", 5) == 0)
    {
        text += 5;
        text += strspn(text, " \t");
        httpdata.m_host = text;
    }
    buff.checkFinish();
    return HTTP_CODE::NO_REQUEST;
}

//对于消息体我们不再进行解析，只判断其是否接受完毕
HTTP_CODE HttpSever::parse_request_content(HttpData& httpdata, Buffer& buff)
{
    if(buff.readend() >= (buff.checkPeek() + httpdata.m_content_length))
    {
        *(buff.checkPeek() + httpdata.m_content_length) = '\0';
        return HTTP_CODE::GET_REQUEST;
    }
    return HTTP_CODE::NO_REQUEST;
}

//主有限状态机
HTTP_CODE HttpSever::parse_main(HttpData& httpdata, Buffer& buff, CHECK_STATE& checkstate)
{
    LINE_STATUS line_status = LINE_STATUS::LINE_OK;
    HTTP_CODE retcode = HTTP_CODE::OPEN_REQUEST;
    while((checkstate ==CHECK_STATE::CHECK_STATE_CONTENT && line_status == LINE_STATUS::LINE_OK)
          || ((line_status = parse_line(buff)) == LINE_STATUS::LINE_OK))
    {
        switch(checkstate)
        {
        case CHECK_STATE::CHECK_STATE_REQUESTLINE:
            {
                retcode = parse_request_line(httpdata, buff, checkstate);
                buff.checkFinish();
                if(retcode == HTTP_CODE::BAD_REQUEST)
                {
                    return HTTP_CODE::BAD_REQUEST;
                }
                break;
            }
        case CHECK_STATE::CHECK_STATE_HEADER:
            {
                retcode = parse_request_header(httpdata, buff, checkstate);
                buff.checkFinish();
                if(retcode == HTTP_CODE::BAD_REQUEST)
                {
                    return HTTP_CODE::BAD_REQUEST;
                }
                else if(retcode == HTTP_CODE::GET_REQUEST)
                {
                    return do_request(httpdata);
                }
                break;
            }
        case CHECK_STATE::CHECK_STATE_CONTENT:
            {
                retcode = parse_request_content(httpdata, buff);
                if(retcode == HTTP_CODE::GET_REQUEST)
                {
                    buff.checkFinish();
                    return  do_request(httpdata);
                }
                line_status = LINE_STATUS::LINE_INCOMPLETE;
                break;
            }
        default:
            return HTTP_CODE::INTERNAL_ERROR;
        }
    }
    return HTTP_CODE::OPEN_REQUEST;
}

HTTP_CODE HttpSever::do_request(HttpData& httpdata)
{
    strcpy(httpdata.m_real_file, doc_root);
    int len = strlen(doc_root);
    strncpy(httpdata.m_real_file+ len, httpdata.m_url, httpdata.FILENAME_LEN - len -1);

    struct stat file_stat;
    if((stat(httpdata.m_real_file, &file_stat)) < 0)
        return HTTP_CODE::NO_REQUEST;

    if(!(file_stat.st_mode & S_IROTH))
        return HTTP_CODE::FORBIDDEN_REQUEST;
    
    if(S_ISDIR(file_stat.st_mode))
        return HTTP_CODE::BAD_REQUEST;

    int filefd = open(httpdata.m_real_file, O_RDONLY);
    httpdata.m_file_address = (char*)mmap(nullptr, file_stat.st_size, PROT_READ,
                                          MAP_PRIVATE, filefd, 0);
    httpdata.m_file_size = file_stat.st_size;
    close(filefd);
    return HTTP_CODE::FILE_REQUEST;
}

