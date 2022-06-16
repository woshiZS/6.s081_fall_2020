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