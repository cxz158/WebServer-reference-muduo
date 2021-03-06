/******************************************************************
*File Name: Main.cpp
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Sun 21 Feb 2021 03:07:37 AM PST
***************************************************************/
#include <unistd.h>
#include "net/EventLoop.h"
#include "net/HttpSever.h"
using namespace std;

int main()
{
    EventLoop mainloop;
    HttpSever http(&mainloop, 3);
    mainloop.loop();
    return 0;
}
