Name 	                   Explanation
dix/ 	      The device independent parts of X, for example the code used for dispatching requests (see Dispatch() 
                in dix/dispatch.c) and handling resources. main() is located in this directory in main.c.
doc/ 	      Less documentation than would be expected - contains the X server man page and an explanation of the scheduler.
fb/ 	      Code for doing graphical operations on framebuffer surfaces, for example blitting and compositing images.
hw/ 	      Hardware dependent code, driver APIs and configuration file operations.
hw/dmx/ 	  Distributed Multi-Head X code - well documented in hw/dmx/doc/html/index.html.
hw/kdrive/ 	  The kdrive server and associated code.
hw/xfree86/   Code associated with unix-like OSes, such as Linux or BSD.
hw/xquartz/   Mac OS X specific code.
hw/xwin/ 	  Cygwin/X code, for running on Windows machines.
include/ 	  Xserver include files lie here.
mi/ 	      Machine independent code, used for things like high-level graphical operations. 
                Makes calls down to the fb/ code through function pointers in screens, windows or gcs.
os/ 	      Operating system dependent code that does not control hardware, things like authentication, 
                or processing arguments passed to the server (see ProcessCommandLine() in util.c for example).
randr/ 	      Code for the RandR extension.
render/ 	  Code for the Render extension.
Xext/ 	      Code for various extensions, for example Xinerama and Xv.
xtrans/ 	  Code for handling network connections. 


Xorg X11是X Window系统的一个开源实现。它提供了基本的底层功能，可以在此基础上设计完整的图形用户界面(gui)，如 GNOME 和 KDE。

Xnest是一个被实现为普通X应用程序的X服务器。它像其他X应用程序一样在窗口中运行，但它本身是一个X服务器，您可以在其中运行其他软件。
对于那些希望测试应用程序而不希望在真正的X服务器上运行它们的开发人员来说，这是一个非常有用的工具。

Xdmx 是代理X服务器，它为连接到不同机器(每台机器都运行一个典型的X服务器)的多个显示提供多头支持。当 Xinerama 与 Xdm 一起使用时，
多台机器上的多个显示将作为一个统一的屏幕呈现给用户。一个简单的应用程序,对于 Xdmx，可以使用两台桌面机器提供多头支持，每台机器都有一个连接到它的显示设备。
Xdmx 的一个复杂应用程序是将 1280x1024 显示器(每个显示器连接到 16 台计算机中的一台)的 4 × 4 网格统一到统一的 5120x4096 显示器中。

Xvfb (X Virtual Frame Buffer)是一个可以在没有显示硬件和物理输入设备的机器上运行的X服务器。Xvfb 使用虚拟内存模拟 dumb 帧缓冲区。
Xvfb不打开任何设备，但表现为X显示。Xvfb 通常用于测试服务器。


Xephyr 是一个被实现为普通 X 应用程序的 X 服务器。它像其他X应用程序一样在窗口中运行，但它本身是一个 X 服务器，您可以在其中运行其他软件。
对于那些希望测试应用程序而不希望在真正的 X 服务器上运行它们的开发人员来说，这是一个非常有用的工具。与 Xnest 不同，Xephyr 呈现 X 图像，而不是中继 X 协议，
因此支持较新的 X 扩展，如 render 和 composite。







int dixRequestPrivate(resttype type, devprivate_key_t *const key，无符号大小，指针的父);
在screenparent上类型为type的对象上请求私有空间的大小字节数(所有屏幕为NULL)。


指针dixlookuprivate (PrivateRec **private, devprivate_key_t *const key);
返回指向与实例关联的私有数据的指针。


int dixSetPrivate(PrivateRec **private, devprivate_key_t *const key，指针newvalue);
设置指向指定值的私有指针。


int dixRegisterPrivateInitFunc(devprivate_key_t *const key, CallbackProcPtr callbackfunc，指针userdata);
int dixRegisterPrivateDeleteFunc(devprivate_key_t *const key,CallbackProcPtr callbackfunc，指针userdata);
注册回调函数，用于初始化新分配的空间(默认为零)，或在空间被释放之前进行清理。用户不能自己释放私有指针!回调函数的参数是一个包含键和私有指针的结构。



DRM：
DRM 包含两部分，generic DRM core，以及 DRM Driver，DRM Driver 是硬件相关部分。DRM core 提供基础框架，可以由不同的 DRM Driver 注册，同时也提供给用户态一个
最小的 ioctl 调用组，硬件无关。DRM Driver负责硬件相关的ioctl调用。

如何提供接口：
DRM core 主要提供通过 libdrm 包装的系统调用，而 DRM Driver 部分的 ioctl 调用一般封装在 libdrm-driver 里面。一般来说，硬件相关的接口像是内存映射、context 管理、
DMA 操作、AGP 管理、vblink 控制、fence 管理、内存管理和输出管理应该在 libdrm-driver。


权限管理 set master :
DRM 事关显示，需要安全，因此 DRM 有提供 ioctl SET_MASTER，执行过此 syscall 的程序就可以成为唯一的 DRM-Master 显示管理程序，执行 ioctl DROP_MASTER 也可以放弃
DRM-Master 身份，一般 X server 就是 DRM-Master。其他的非 DRM-Master 用户态程序通过 DRM-Auth，具体过程是：执行 ioctl GET_MAGIC 得到一个 32bit 魔数，然后传给
DRM-Master -> DRM-Master 用此 32 位魔术 ioctl AUTH_MAGIC 给 DRM 设备 -> DRM 设备给发起 Auth 的非 DRM-Master 应用持有的与此魔数对应的 fd 授权。


DRM 中的 GEM(Graphics Execution Manager)
从 DRI2/3 的表述，每个渲染实体都开始持有自己的 back buffer，并且从显卡申请的内存也可以 context 切换出去后一直持有，这些被持有的内存就需要 DRM 在内核中管理起来，
GEM就提供这些 API，包括申请、释放这些个 GPU 用的内存(可能集显在内存，独显在显卡)，姑且称显存。


显存在进程间传递：
申请了显存就可以得到一个 32bit 数，把这个数字告知另一个用户态程序即可实现显存的传递、共享，这其实是有安全问题的，任何应用都可以随便猜个数，然后读、写内容。
不过这个问题后来通过引入 DMA-BUF 解决，DMA-BUF 是用于在多设备间共享 DMA 缓存的，比如说 Video4Linux 设备和显示器就可以通过 DMA-BUF 实现 0 复制的数据传输，
任意 linux 设备驱动都可以实现此 API 的生产和/或消费者，DMA-BUF 一开始适用于集显 + 独显的共享 FB，然后用的愈加的多；提供有 ioctl 将 GEM 句柄与 DMA-BUF fd相互转换，
转成 DMA-BUF 的 GEM 对象就可以通过 IPC socket 的 SCM_RIGHTS 完成传递。DRI3 在 client 和 Xserver 间传 buffer、Wayland 都是这样用的。


GPU-CPU间的内存同步：

现代计算机到处都是 cache，cache 对齐和保障 CPU-GPU 内存一致就需要考虑，也就有了对硬件实现的高度依赖，对于集显，没有独立的内存，比较好说，
基于 UMA(Uniform Memory Architecture) 划分内存域进行管理；而独显则大多选择将 GEM 的 API 实现在用户态。


TTM(Translation Table Maps)
TTM 出于 GEM 之前，设计用于管理 GPU 可能访问的不同类型的内存的管理，独立显存(Video RAM 即 VRAM)、通过 IOMMU 访问的系统 RAM (即 GART-Graphics Address Remapping Table)、
以及 CPU 不能直接访问的显存；还有一个功能就是维护内存一致性。用户态图形程序要访问显存时，如向显存中装数据，TTM 需要做映射使 CPU 可访问，或通过 GART 让 GPU 访问系统内存，
这些都需要处理内存一致性问题。另一个重要的 TTM 概念是 fences，内存屏障；这是一个管理 CPU-GPU 内存一致性的基础机制，一块共享内存，GPU 使用完，其他想要使用这块内存的用户态
程序就可以得到提醒，其实就像是互斥锁。
TTM 想把独显、集显用一致的方法统一管起来，但这导致 TTM 太复杂，API 太多；GEM 更简单的、API 比 TTM 更好的；不过 TTM 很适合独显、有 IOMMU 的场合，所以就把 TTM 包
在了 GEM 的 API 下面。AMD 和 NVIDIA 显卡都这么干的。


Render Nodes

一开始DRM设备 /dev/dri/cardX 是特权操作( modsetting / 其他显示控制)和非特权操作(如渲染操作)公共使用的设备节点，致使仅部分可靠的用户态程序(如 X，作为 DRM-Master)
才有全部的 DRM API 的访问权限，其他的用户态程序想要使用 GPU 做渲染就只能先去 DRM-Auth，然后才能执没有特权操作的 DRM API。这一设计需要如 X server这样的程序必须运行，
而很多使用 GPU 渲染的程序并不是为了显示(比如我现在这样的)，不显示就不需要这么关注安全，毕竟只是用用硬件，得个结果，而这个结果不是为了迷惑显示器前的人；再这样的考量下，
Render Node 的概念：把 DRM 用户 API 分成特权和非特权的，分开使用 GPU，也就是添加了设备 /dev/dri/renderDX。这样想要利用 GPU 算力而不是为了显示的兄弟们就只要有此
设备节点的访问权限就能做计算了，而 compositors，X server，以及其他需要 modeset API 或是特权操作的，还是必须使用 /dev/dri/cardX 设备节点，像之前一样干活。
而 Render Node 也就不允许通过 GEM flink 一下就共享 buffer 的操作，需要转成 DMA-BUF fd 来共享、传递。


blob：
A proprietary device driver is a closed-source device driver published only in binary code. In the context of free and open-source software, 
a closed-source device driver is referred to as a blob or binary blob. The term usually refers to a closed-source kernel module loaded into 
the kernel of an open-source operating system, and is sometimes also applied to code running outside the kernel, such as system firmware 
images, microcode updates, or userland programs. The term blob was first used in database management systems to describe 
a collection of binary data stored as a single entity.
专有设备驱动程序是只以二进制代码发布的闭源设备驱动程序。在自由和开源软件的背景下，闭源设备驱动程序称为blob或二进制blob。这个术语通常指装入的闭源内核模块
开源操作系统的内核，有时也应用于运行在内核之外的代码，如系统固件图像、微码更新或用户空间程序。术语blob最初是在数据库管理系统中用来描述的作为单一实体存储的一组二进制数据。