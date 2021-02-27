/******************************************************************
*File Name: SocketOps.h
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Sun 21 Feb 2021 04:58:36 AM PST 
***************************************************************/
#pragma once
#include <arpa/inet.h>
#include <string>

int socket_bind_listend_noblock(int port, struct sockaddr_in& addr);
std::string sock_ntop_ipv4(const struct sockaddr_in& addr);
std::string get_time();
void ignore_sig_pipe();
