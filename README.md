# WebServer-reference-muduo
# 简介
本项目是由C++11编写的一个静态web服务器，能够解析GET请求，参考了陈硕大佬的muduo网络库，linya的WebServer和游双的《Linux高性能服务器编程》。支持http长连接，并且实现了异步日志。在本机上使用webbench并发
1000个clients访问, 短连接能达到大概1.7wQPS 长连接能达到大概4wQPS（这个数值有待商确）。
# Usage
```
mkdir build
cd build
cmake ..
make
```
# 项目设计
- 本项目使用的并发模型是Reactor+非阻塞io复用+线程池，设计基本遵循muduo中所说的one loop per thread。
一个mainloop负责接受连接，然后将连接交给线程池中的loopthread处理，loopthread全权负责接受到的连接，即同时负责接受数据，对数据进行处理和发送数据。
- 项目使用基于对象的设计，而非面向对象的设计，大量使用std::bind来代替虚函数。
- 项目使用的是epoll的ET触发模式，因为一个连接的数据始终只由接管它的io线程负责，所以没有设置EPOLLONESHOT,如果要新增线程池用于计算数据的话，考虑到数据争议的问题需要添加EPOLLONESHOT

# 项目测试
 - 项目测试使用到了支持长连接测试webbench，测试结果如下图所示，作为对比同时测试了linyaWeb在本机环境下的长连接的测试情况，测试不是非常严谨，仅作一点参考。
 - 在长连接测试时，本项目打开网页的响应响应速度已经是非常缓慢了，而linyaweb则感觉不到明显的卡顿，我认为这是因为目前本项目在长连接的处理上一个错误设计导致的，但也是这个错误的设计，让本项目在长连接测试中取得较高的成绩。
 - 在接收到来自一个连接的大量请求（一次最大限度的read）时，本项目会一直等到处理完该连接的所有请求后，才会去处理下一个连接的请求，这导致后到的连接需要等待许久才能得到响应。这样的服务器在现实环境中显然十分容易遭到攻击。
 - 关于负载情况，短连接测试下mainloop(webserver)会占用一定cpu，而长连接下则基本不占用，这是因为短连接测试需要频繁的accept和close连接，而长连接不需要，这也是其处理能力更强的原因。

- 短连接测试
![image](https://github.com/cxz158/WebServer-reference-muduo/blob/main/img/cxzweb%E6%B5%8B%E8%AF%95%E7%9F%AD%E8%BF%9E%E6%8E%A5.png)
 
- 长连接测试
![image](https://github.com/cxz158/WebServer-reference-muduo/blob/main/img/cxzweb%E6%B5%8B%E8%AF%95%E9%95%BF%E8%BF%9E%E6%8E%A5.png)

- 负载情况（短连接）
![image](https://github.com/cxz158/WebServer-reference-muduo/blob/main/img/cxzwebcup%E8%B4%9F%E8%BD%BD.png)

- 负载情况（长连接）
![image](https://github.com/cxz158/WebServer-reference-muduo/blob/main/img/cpu%E8%B4%9F%E8%BD%BD%E9%95%BF%E8%BF%9E%E6%8E%A5.png)

- linyaWeb长连接测试
![image](https://github.com/cxz158/WebServer-reference-muduo/blob/main/img/linyaweb%E6%B5%8B%E8%AF%95%E9%95%BF%E8%BF%9E%E6%8E%A5.png)
