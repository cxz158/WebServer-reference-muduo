# WebServer-reference-muduo
# 简介
本项目是由C++11编写的一个静态web服务器，参考了陈硕大佬的muduo网络库，linya的WebServer和游双的《Linux高性能服务器编程》。支持http长连接，并且实现了异步日志。在本机上使用webbench并发
1000个clients访问, 短连接能达到大概1.7wQPS 长连接能达到大概4wQPS（这个数值有待商确）。
# Usage
```
mkdir build
cd build
cmake ..
```
# 项目模型
本项目使用的并发模型是Reactor+非阻塞io+线程池，设计基本遵循muduo中所说的one loop per thread。
一个mainloop负责接受连接，然后将连接交给线程池中的loopthread处理，loopthread全权负责接受到的连接，即同时负责接受数据，对数据进行处理和发送数据。
项目使用基于对象的设计，而非面向对象的设计，大量使用std::bind来代替虚函数。
# 项目测试
短连接测试
![image](https://github.com/cxz158/WebServer-reference-muduo/blob/main/img/cxzweb%E6%B5%8B%E8%AF%95%E7%9F%AD%E8%BF%9E%E6%8E%A5.png)
 
长连接测试
![image](https://github.com/cxz158/WebServer-reference-muduo/blob/main/img/cxzweb%E6%B5%8B%E8%AF%95%E9%95%BF%E8%BF%9E%E6%8E%A5.png)

负载情况
![image](https://github.com/cxz158/WebServer-reference-muduo/blob/main/img/cxzwebcup%E8%B4%9F%E8%BD%BD.png)

linyaWeb长连接测试
![image](https://github.com/cxz158/WebServer-reference-muduo/blob/main/img/linyaweb%E6%B5%8B%E8%AF%95%E9%95%BF%E8%BF%9E%E6%8E%A5.png)
