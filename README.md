### Copy on Write

* 因为需要在kalloc里面记录每个对应物理页的refcnt，并且在fork的时候也需要对refCnt进行修改，所以需要extern标一下。（稍微会议下extern的用法，总结起来就是extern告诉你去其他地方找这个变量/函数的定义，但是extern也可以直接加定义或者初始化。然后就是function的extern是默认的）
* 关于新标记PTE_COW的设定和取消：考虑到引发write page fault on COW之后，原来的COW位应该被取消，有可能有两个process ref一个old page，都引发这个page fault之后，老的page就无人回收了，所以在trap那里应该check一下old page的引用次数，如果原来是1减去1之后是0，那么就释放掉，其实也可以在那里调用一次kfree
* **这里发现不能正常gdb，发现其实是没有正常加载init file**，加载完init file之后一切都正常了
* 一些小细节：主要还是关于位运算的，比如说一开始qemu无法正常运行，原因是自己在处理COW标记的时候没有处理好。另外就是cowtest的file那边出现乱序输出，还是原来的pte flag没有处理对。

```c
// 错误写法，原来如果PTE_W是1的话，并不会被清除
*pte |= flags;
//正确写法，应该把10bit flag全部置0然后再和flags进行或运算
uint64 pa = PTE2PA(*pte);
*pte = PA2PTE(pa) | flags;
```

* 整体的思路还是没差，lab的重点在于如何正确的释放物理页，举个例子，parent和child都触发cow之后，原来的物理地址对应页需要被释放掉。每次触发cow，都需要我们手动对原有物理页做一次free，利用free内部做的检查之后再释放。