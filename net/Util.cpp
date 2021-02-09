#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>


//创建socket监听描述符 成功返回对应的socket描述符 失败返回-1
int socket_bind_listen(int port)
{
    if(port < 0 || port > 65535) return -1;
    
    int listen_fd = 0;
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0) == -1)) return -1;

    //设置socket复用ip地址选项
    int reuse = 1;
    if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
    {
        close(listen_fd);
        return -1;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = port;
    if(bind(listen_fd, (struct sockaddr*)&address, sizeof(address)) == -1)
    {
        close(listen_fd);
        return -1;
    }

    if(listen(listen_fd, 2048) == -1)
    {
        close(listen_fd);
        return -1;
    }

    return listen_fd;
}

int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    if(old_option == -1)
        return -1;
    int new_option = old_option | SOCK_NONBLOCK;
    if(fcntl(fd, F_SETFL, new_option) == -1)
        return -1;
    return old_option;
}
