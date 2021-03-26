### Lab notes

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