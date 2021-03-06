/******************************************************************
*File Name: Buffer.h
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Wed 24 Feb 2021 06:32:55 AM PST
***************************************************************/
#pragma once
#include <vector>
#include <string>
#include <string.h>
#include <algorithm>
//所有的Buffer 操作都在IO线程中进行， Buffer无需保证线程安全
//**需要Buffer的理由：
//1.对于read() 读到的数据可能构不成一条消息无法立即对进行decode, 此时有两种做法，一是使用recv()设定设置选项，让recv()阻塞直到
//读到指定的字符后才返回,二是利用Buffer暂存不完整的消息，等到下一次read(),直到Buffer中存在一个完整的消息后，才对其进行处理。
//显然对于非阻塞io来说使用的是第二种方法
//
//2.对于write() 操作系统能够接受的数据可能比你想写的数据要少（受TCP拥塞窗口的影响），此时有两种做法，一（阻塞的）是等对方接受并处理
//完数据（滑动TCP窗口）后继续写入，二（非阻塞的）是将多出的数据暂存在Buffer中，下次再写
class Buffer
{
public:
    Buffer():buffer_(BUFFSIZE), writeIndex_(0), readIndex_(0), checkInex_(0){}
    ~Buffer(){}
    int readfd(int fd);
    void append(const char* data, size_t len);
    size_t readableBytes() const { return writeIndex_ - readIndex_; }

    const char* readbegin() const { return buffer_.data() + readIndex_; }
    const char* readend() const { return buffer_.data() + writeIndex_; }
    char* readbegin() { return buffer_.data() + readIndex_; }
    char* readend() { return buffer_.data() + writeIndex_; }
    char* checkPeek() { return buffer_.data() + checkInex_; }
    void checkFinish() { checkInex_ = readIndex_; }

    void retrive(int n){ readIndex_ += n; }
    void retrive_to(char* rsc){ readIndex_ +=  rsc + 1 - readbegin();}
    void init();
    char* findLine();

private:
    static int BUFFSIZE;
    size_t writeableBytes() const { return buffer_.size() - writeIndex_; }
    std::vector<char> buffer_;
    int writeIndex_;
    int readIndex_;
    int checkInex_;
};

