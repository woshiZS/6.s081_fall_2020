### Lab notes

#### mac下的环境配置

因为之前很久做了一半，所以现在重新开始做的时候要再次配置环境，mac上配置20的环境是异常痛苦的，这里特意做一下说明。

* 安装riscv-tool-chain

这里不推荐使用brew安装，因为时间太久容易连接中断或者卡死。这里建议两种方法：

1. 下载源码编译，可以通过git clone，也可以通过百度云盘源码下载
2. 下载预编译好的版本（实际上这种也是可以，可以说是最快的一种方法，因为后面引发的错误其实是qemu版本过高引起的问题）

**需要注意将riscv-gnu-toolchain加入到PATH中**

* 安装qemu

这一步分问题就比较多了，如果使用```brew install qemu```的话，默认版本在启动的时候会卡死，需要下载4.1.0的版本。

```shell
wget https://download.qemu.org/qemu-4.1.0.tar.xz
tar xf qemu-4.1.0.tar.xz
cd qemu-4.1.0
./configure # 默认的bin路径会添加到/usr/local/bin下面，所以无需额外再添加路径
make && make install # 需要等待大概一个小时左右
```

之后按照官网上的步骤确认gnu-toolchain和qemu的版本即可。

在20仓库进行编译之前需要先```make clean```再make或者```make qemu```, 经测试，qemu可以正常运行。

### xv6 chapter1读书笔记

* hart: 表示的是hardware的执行context，与软件层面的execution context区分开来 
* process包括instruction, data和执行栈，以及寄存器状态，当进程不执行的时候，cpu将寄存器状态保存，然后切换到其他进程，然后切换回去的时候又会将这些寄存器的值回复到cpu中。每个进程由一个pid关联。
* 这里有提到fork和exec要分离开，后面提到的copy-on-write技术会具体阐述分开的原因，这里提到一个原因可能是执行时候所需要的内存空间会更大，fork的时候只需要把parent memory copy过来即可。
* file descriptor是对文件，管道以及设备（devices）的抽象。**file descriptor对于每个进程来说是一个file descriptor table里面的index(So every process has a private space of file descriptors starting at zero.**
* A newly allocated file descriptor is always the lowestnumbered unused descriptor of the current process, **利用这个特性可以很容易的实现IO重定向，大概就是close(0); open("input.txt", O_RDONLY);这样就把0当做默认的输入**
* fork和dup复制的file descriptor都是共享文件off set的
* Pipes: 暴露给process的kernel buffer，拥有两个file descriptor，一个读，一个写
* pipes的写管道除了对应的process之外其他都要关闭，因为read只有检测到所有write file descriptor都关闭之后才会return。
* pipes对比临时文件有四个优点：
  * 使用完之后pipe会自动清理
  * pipe可以传递任意长度的信息，而临时文件需要磁盘有足够多的空间（同时，写磁盘也是一个比较耗时的操作）
  * pipes允许并行执行，然而temp files只能顺序执行
  * pipes的read write阻塞模型相较于non-blocking files semantics更有效率（**虽然没懂后面啥意思**）
* File system：将所有文件，目录以及硬件设施都抽象成为file descriptor的思想，细节没有讲。

#### sleep

主要是熟悉了一下用户程序的基本结构，由sh解析，fork之后（cd除外）由子程序执行，user.h包含系统调用的user接口声明和一些用户空间实现utility function，types.h主要是定义一些类型别名。

另外RISC-V和MIPS还是有很多相似的地方，后期可以做一个对比表格（MIPS, RISCV, X8664)

#### ping pong

> need to add space or output format would not fit the criteria.

#### prime

* Use a specific prime sieve. When confirming one number is a prime, other number which is divisible by that prime is not a prime and we do not need to transport it to the next process. When 

> question: After parent process exits, child process is still alive?

* Yes, after parent process died, the child process become the child of the **init** process.

* Read pipe and write pipe are different cause without multiple pipes, processes will mess up one single pipe.

> will grand process wait for grandson process?

* No, look at the example below.

![image-20210325114446507](./pics/code_snippet.png)

process information:

![image-20210325114833917](./pics/ps_before.png)

after kill child process.

![image-20210325114912855](./pics/ps_after.png)

之后测试的时候有一个很脑残的错误，写管道的时候，写的是原来的管道，原来的gdb还有内部bug，肉眼找bug花了好久时间。

* find

大致思路应该还是枚举和筛选，先找出本目录下的文件进行筛选，如果是文件就进行比对，如果是目录就进行递归调用。

讲一下出现的几个问题，照搬ls里面的fmtname函数，这里的问题主要是函数会返回一个定长的字符串指针，文件名不够的地方用空格补全，这样strcmp肯定不会返回0，所以删去补齐空格的memset函数就好，最后还要再末尾加上一个串尾符。

另外就是不要递归回上级目录和本机目录，至于比较的话可直接使用dirent结构里面的name。

* xargs

这个我不是太熟，看了一下manual手册，感觉就是把管道左边的写入的内容作为管道右边命令的参数。

具体步骤就是先读取原有的命令参数，然后再从标准输入中读取其他参数（遇到换行执行一次）

最后执行的指令（一个字符串数组）最后一个位置要加0，原因如下。

![image-20210326161652331](./pics/exec.png)

* 期间还试过一次按照字典序排列头文件，结果有声明依赖...
* 其实还是比较好奇：xargs怎么忽略掉前面的指令的（得去看sh.c的代码）