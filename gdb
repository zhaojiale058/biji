xorg流程
 
初始化后，执行dispatch函数
 
 
  dispatch流程
 
 
 
关键代码：
result = (*client->requestVector[client->majorOp]) (client);
 
/*对于每个client的request，xorg   server对应的 处理函数是一个table。
*这个数组的名字ProcVector。索引是client->majorOp
*/
 
/**
(gdb) print  client->requestVector
查看整个数组
**/
/**
**查看数组某一项
* (gdb)  print  client->requestVector[133]
*  $10 = (int (*)(ClientPtr)) 0x4cd6a0 <ProcBigReqDispatch>
 
*/
 
/**
如何根据client得到client对应的pid呢，查看代码，找到了。
print  client->clientIds->pid
(gdb) print  client->clientIds->pid
$11 = 6680
*/
/**
gdb 根据pid 断点
b dispatch.c:473 if  client->clientIds->pid == 17004
**/
/**
*根据pid和客户端的request 类型断点
*b dispatch.c:473 if  client->clientIds->pid == 8162  && client->requestVector[client->majorOp] == ProcXvDispatch
*
*/
 
/**
由于xorg是单线程执行，一断点处停止，所有事件都不响应。
如果app和xorg一起调试，又不知道app代码。
针对这个情况，对调试的app，在gdb 环境下，ctrl +c 暂停，然后让xorg正常运行。
xorg的断点是pid为app才停止。所以xorg不会断。正常运行。
这样就可以操作桌面。
*/
 
 
运行mpv
export DISPLAY=:0
/**
sudo apt-get install mpv
sudo apt-get install mpv-dbgsym
*/
设置参数  set args  
 
(gdb) file mpv
(gdb) set args  -vo=xv  /home/wwh/Desktop/dde-introduction.mp4
(gdb)  b main
(gdb) r
 
gdb  -p  pid
handle SIGPIPE nostop
/***************************************************************************************************************************************/
主要是进度条的处理上，在xv模式下，进度条会走x11处理，造成卡顿，x11模式下，
进度条和视频处理都是一起，所以不会卡
 
/***************************************************************************************************************************************/
 
 
 
 
 
/************************************************************************************************************************/
gdb上面挂钩子的办法
b dispatch.c:473 if  client->clientIds->pid == 19542  
commands 1
        silent
        printf "requestVector = %d\n", client->majorOp
        continue
        end
/************************************************************************************************************************/
现在龙芯系统上，mpv播放器播放慢的问题，我通过gdb挂钩子观察。
 
在普通情况下，是151 请求在ProcVector数组中。
(gdb) print ProcVector[151]
$120 = (int (*)(ClientPtr)) 0x4da9d0 <ProcXvDispatch>
对应的函数是ProcXvDispatch
在鼠标移动到播放器上，视频会卡顿，播放器发送的请求是：
ProcChangeWindowAttributes,  /*2*/
ProcAllocNamedColor,                /* 85 */
ProcCreatePixmap,              /*53*/
ProcCreateGC,                       /* 55 */
ProcPutImage,                      /*72*/
ProcFreeGC,                         /* 60 */
ProcCreateCursor,            /*93*/
ProcChangeWindowAttributes,  /*2*/
ProcFreeCursor,                     /* 95 */
ProcFreePixmap,                  /*54*/
ProcFreeColors,                     /*88*/ 
是xorg 处理这些请求花费太多时间，造成xv播放卡顿/？
还是客户端为了发送这些请求，花费太多时间，造成播放卡顿。
需要进一步调查。
 
同时
播放器为发送请求的时间打印出来。
xv处理完这些request的时间都打印出来。
xv最终显示图片的时间都打印出来。
在这个时间流上，看哪些东西是卡点，也可以跟x86对比
 
这个kernel shark的思维方式：如果你期待的是一个横轴是时间，纵轴是每个函数、CPU在某个时间段里面在干什么的图。这个思维方式确实很有用。
/************************************************************************************************************************/
 
对于arm上面，709所显卡驱动
ProcCreatePixmap -> exaCreatePixmap_driver
     |-> fbCreatePixmap
     |   -> AllocatePixmap(pScreen, datasize)
     |        ->pPixmap = malloc(pScreen->totalPixmapSize + pixDataSize);
     |->CsmCreatePixmap 
      /*CsmCreatePixmap  是通过 print pExaScr->info->CreatePixmap看到*/
 
/**
*对于CsmCreatePixmap的创建，我们没有源代码/
*参考radeon驱动的。
* screen->CreatePixmap = radeon_glamor_create_pixmap;
*    ->radeon_glamor_create_pixmap(ScreenPtr screen, int w, int h, int depth, unsigned usage)
          ->radeon_alloc_pixmap_bo
*              ->radeon_bo_open /*如果不是glamor模式，走这里*/
*           
*/
/**
在liddrm中
radeon_bo_open 
    ->bo_open
          -> r = drmCommandWriteRead(bom->fd, DRM_RADEON_GEM_CREATE,
                                &args, sizeof(args));
*/
 
/*从此我们看出了pixmap的来源。他是一个申请的bo。bo是用shemem 还是drmbuf，还是什么。*/
 
 
/************************************************************************************************************************/
/************************************************************************************************************************/
gdb使用
info 功能
info b 查看breakpoint状态
 
info sources -- Source files in the program  
/*
这个功能很重要，
比方对于xorg的那些so的用户态module，就可以通过file  *.so , 然后 info sources 查看组成的c文件.。
如果attach上正在运行的xorg，则可以看到xorg和所有的so的源文件组成。只是有些格式拥挤，可以拷贝到记事本查找。
同时，用这个功能，查看libc.so看到memcpy是由c还是s文件编译而成的。
*/
 
info source -- Information about the current source file
/*只是显示当前断点函数所在的文件*/
 
info address -- Describe where symbol SYM is stored
info symbol -- Describe what symbol is at location ADDR
/*
地址和符号名字互相查找
*/
 
/*
比方想知道一个局部指针指向的地址是符号表的什么变量
方法一：
(gdb) print client->requestVector
$9 = (int (**)(ClientPtr)) 0x62f6c0 <ProcVector>
(gdb) info symbol 0x62f6c0
ProcVector in section .data of /usr/lib/xorg/Xorg
方法二：
(gdb) info symbol client->requestVector
ProcVector in section .data of /usr/lib/xorg/Xorg
*/
 
info sharedlibrary
/*查看所有的共享库*/
 
info threads
/*查看当前程序所产生的所有线程*/
 
info variables -- All global and static variable names
/*加上库的，确实内容有点多。仅仅file一个可执行程序不大*/
/*所在文件和行数都能看到呢*/
 
info functions -- All function names
/*跟上面同理哦*/
 
/********************************************************************************************************************/
/********************************************************************************************************************/
gdb 多线程调试 /*多进程也可以attach来调试*/
/* //显示进程间的关系*/
/*有{ 表示线程哦*/
/*
 ├─lightdm,13871
  │   ├─{lightdm},13872
  │   ├─{lightdm},13874
  │   ├─Xorg,13877 -background none :0 -seat seat0 -auth /var/run/lightdm/root/:0 -nolisten tcp vt1 -   novtswitch
  │   │   ├─{Xorg},13880
  │   │   ├─{Xorg},13882
  │   │   ├─{Xorg},13883
  │   │   ├─{Xorg},13884
  │   │   ├─{Xorg},13885
  │   │   ├─{Xorg},13886
  │   │   ├─{Xorg},13887
  │   │   ├─{Xorg},13889
  │   │   └─{Xorg},13895
  │   └─lightdm,14096 --session-child 12 19
 
*/
 
 
/********************************************************************************************************************/
/********************************************************************************************************************/
多进程调试
注意，是lightdm产生各种子进程。
但是sudo gdb
(gdb) attach  13871
(gdb) info inferiors
  Num  Description       Executable
* 1    process 13871     /usr/sbin/lightdm
/*这里没有显示各种子进程*/
 
(gdb) info threads
  Id   Target Id                                   Frame
* 1    Thread 0x7f2a53833880 (LWP 13871) "lightdm" 0x00007f2a53b28829 in __GI___poll (fds=0x7f2a480097f0, nfds=2,
    timeout=-1) at ../sysdeps/unix/sysv/linux/poll.c:29
  2    Thread 0x7f2a53832700 (LWP 13872) "lightdm" 0x00007f2a53b28829 in __GI___poll (fds=0x5569633029d0, nfds=1,
    timeout=-1) at ../sysdeps/unix/sysv/linux/poll.c:29
  3    Thread 0x7f2a527b8700 (LWP 13874) "lightdm" 0x00007f2a53b28829 in __GI___poll (fds=0x7f2a44010e40, nfds=2,
    timeout=-1) at ../sysdeps/unix/sysv/linux/poll.c:29
 
同时，我们也能另外开一个gdb，attach到 xorg
/**********************************************************************************************************/
使用follow-fork-mode 的方法调试多进程
 
1.启动gdb 
2设置set follow-fork-mode [child/parent] 想调试child就使用child参数，如调试parent则使用parent
3启动程序文件 file ./hello ----------当然路径名自己要搞对哦
4.break num -----------如调试子进程则num是子进程代码的行数,
 
还有个小技巧就是break fork也就是在fork函数处设置断点,
 
 
/**********************************************************************************************************/
 
多线程调试
 
/*gdb默认是同时stop，同时continue*/
/*在 all-stop 模式下，当某一线程暂停执行时，GDB 调试器会自行将其切换为当前线程；*/
/*gdb好像也不支持开多个终端，attach到不同的子线程。*/
(gdb) attach 13877
(gdb) info threads
(gdb) thread 2
/*进入第二个线程*/
 
/*******************************************************/
针对同时stop模式下，自动切换线程的实验
 b inputthread.c:340
/*在InputThreadDoWork(void *arg) 的while 循环里面*/
(gdb) c
Continuing.
[Switching to Thread 0x7fcc5adc7700 (LWP 9160)]
/*******************************************************/
 
 
 
/*******************************************************************************************/
如果想设置线程异步停止
/*xorg 上调试，可能因为sockect阻塞等原因，造成桌面功能不正常*/
set target-async 1
set pagination off
set non-stop on
 
 b inputthread.c:340  10
 
(gdb) info threads
thread apply 4 2 1 continue
 
(gdb)b  mt.c:56  
/*所有的线程有效*/
 
(gdb) thread apply 3 c
/*thread apply 3 c&*/
/*thread apply ID1 ID2 command*/
 
(gdb) break frik.c:13 thread 28 
/*针对某一个线程的断点*/
 
/*thread apply 3 c&
* 没有这个&这一招，ctrl + c 不能控制哪个interrupt的 
*
*/
 
/**********************************************************************************************************/
(gdb) info functions InputThreadDoWork
All functions matching regular expression "InputThreadDoWork":
 
File ../../../../os/inputthread.c:
313:    static void *InputThreadDoWork(void *);
 
/*gdb 查询函数*/
 
/**********************************************************************************************************/
 
 
 
dix  设备不依赖层 device  independ
 
x协议：
客户端请求
服务端回应
当客户端程序有请求到来时，X Window通过
请求号从函数表ProcVector中找到对应的服务函数，然后调用它，服务函数执行完成后返回结果。
 
还 有一个函数表，叫作SwappedProcVector，它与ProcVector中的函数功能上基本是对应的，
特殊之处在于，在调用真正的服务前，先预 处理一下请求的数据，客户端程序可能运行在不同字节顺序的机器上，这时候要交换一下数据的字节顺序
/**********************************************************************************************************/
xcb 学习
﻿https://www.x.org/releases/X11R7.6/doc/libxcb/tutorial/index.html#winop﻿
﻿https://www.x.org/releases/X11R7.6/doc/libxcb/tutorial/index.html#DisplayCells﻿
 
/***********************************************************************************************************************/
缩略语清单
BDF     Bitmap Distribution Format
DBE     Double Buffer Extension
DIX      Device Independent
DDX     Device Dependent
ICCCM The Inter-Client Communication Conventions Manual
DMX distributed multihead X system
DMPS Display Power Management Signaling
DPS Display Postscript
DRI Direct Rendering Interface
EVI Extended Visual Information
FS Font Service
ICE Inter-Client Exchange
RX remote execute
CUP Colormap Utilization Policy and Extension
DMCP Display Manager Control Protocol
XIM X Input Method Protocol
XI X11 Input Extension Protocol
XSMP X Session Management Protocol
XP X Print Service
XPM X PixMap Format
XTrans X Transport Interface
GC Graphic context
 
最小系统
TinyX 和 kdrive关系 /*如何做最小系统*/
 
/***********************************************************************************************/
go 语言得到时间t0 := time.Now()
 
startTime := time.Now()
endTime := time.Now()
duration := endTime.Sub(startTime)
/***********************************************************************************************/
 
info file 或者info  files
查看attach或者file上的程序
 
 
 
 
 
 
 
 
 
/***********************************************************************************************************************/
gdb最全面的文档，还是gdb手册
﻿https://sourceware.org/gdb/onlinedocs/gdb/﻿
 
比方说Background Execution
c& 我就是在手册里面查到的。
后台执行，对于多线程很重要。
 
需要中断执行：
interrupt
interrupt -a
 
/*对于全stop mode ，没有区别
对于 non-stop mode ，一个是当前，一个所有线程*/
/******************************************************************************************************/
blit    位块传输
 
/******************************************************************************************************/
GDB查看结构体定义-------ptype