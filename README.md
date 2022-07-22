### page tables（第三章笔记）

页表用于给每个进程提供自己独立的地址空间和内存，保证了进程间的地址隔离和虚拟化。

* xv6采用的39bit虚拟地址， 高27位作为三层页表的pte index.
* walk用于找到指定虚拟地址的PTE
* mappages用于将新的PTE写入新的虚拟地址和物理页的映射关系。
* kvm打头用于操作内核页表，uvm开头的用于操作用户页表
* 在boot阶段，main函数调用kvminit，在kvminit中创建内核页表。
* 内核页表各部分意义解释
  * UART registers：universal async registers/transmitters，简而言之就是将外设传送给cpu的并行数据转化为串行数据。
  * Virtio mmio disk: 简而言之就是虚拟化（virtual machine）映射外设的一种方式，mmio指的是内存映射IO
  * 之后就是CLINT和PLIC:目前大致知道这个是负责RISC-V终端的两个控制器就好了，RISC-V暂时不支持嵌套中断，所以中断方式还是比较明朗的，到后面仔细阅读RISC-V手册，可以把这些内容给补上
  * 之后都是相同的对应映射
* 之后调用kvmmap将对应部分从虚拟地址映射到实际的物理地址上，这个时候因为还没有开启分页机制，所以其实虚拟地址和物理地址都是相等的映射。
* kvmmap具体的流程
  * 调用mapmapges，将va到va + sz的每个整数倍页大小起点映射到pa开始的物理地址（PGROUNDDOWN), 每次去va对应的9位得到分层页表的PPN(物理页号)， 此时因为没有开启分页机制，所以得到物理页内容可以直接当做虚拟地址来用，最后返回最后一级页表中的pte。
  * 加入对应pte的valid flag没有设置，那么说明这个物理页是无效的，如果调用参数允许分配，就在该位置重新分配一个物理页作为新页表。
* main流程调用玩kvminit之后，需要调用kvminithart，具体作用是将内核页表的位置写入satp寄存器，然后刷新TLB的cache
* 之后就是在main函数中调用procinit，为每一个进程都分配一个物理页，映射到虚拟地址的高位，虚拟地址由KSTACK和进程在进程数组中的index决定。这一页的内容暂时还没有在这个方法里面进行写操作，之后需要进行写操作的时候，直接按照进程的index获取kernel stack的虚拟地址即可获得对应的物理地址。
* 调用完procinit之后需要再次执行kvminithart，因为之前的pte可能会失效，访问到其他进程所占用的物理页就不太好了（当然需要进程间的通信就另外一说）。
* 物理内存的分配：xv6的物理内存分配是由一个链表来管理的，分配出去一个页，就将这一页从空闲链表中移除，释放掉一个页，就将这个页加入到链表中。
* 初始化的时候首先初始化kernel end到PHYSTOP之间的物理页到空闲链表中，起始点定义如题：

![image-20220716193542413](./img/kstart.png)

这个end，定义在kernel.ld文件中，ld文件格式是由GNU linker command所构成的脚本，实际上end的数值就是kernel.symbol中定义的数值。

![image-20220716231716770](./img/kernelsymbolend.png)

* 进程的地址空间，就是从0开始到MAXVA(由RISCV的39位决定)，在代码段和数据段的上方，再加上一个guard page, 栈大小刚好为一个page size，stack自顶向下增长，溢出则会报page fault，但是在实际的操作系统中，可能会分配更多的物理内存给栈。

**sbrk系统调用**

sbrk系统调提供给进程，用于增长或者减小其内存，调用层级关系是```sbrk -> uvmalloc/uvmdealloc -> kalloc/(walk -> kfree)```, 进程页表除了作为映射虚拟地址到物理地址的关系，还记录了每个进程分配了多少物理页。

**Exec系统调用**

exec可以用于创建用户态的地址空间，它从一个磁盘上的二进制文件中创建地址空间。首先对文件的ELF格式进行检查。格式检查没有问题，之后给新的进程分配一个页表，最开始的时候只对trampoline和trampframe两个物理页做了映射。然后调用loadseg加载所有的program header。然后分配两个页，一个用于user stack，另一页用于guard page。完成这些工作之后，将页表赋值给新创建进程的页表，并释放之前对应的页表。

**内存分配**

xv6只支持按页来分配内存，但是在现在更加复杂的操作系统中，是需要支持像malloc这样的可分配任意大小内存的分配器。

关于课后练习，可以等到将lab完成之后再来做。

#### print a pagetable

* 这个比较简单，递归一下，递归出口在叶子page table的时候为出口，外部再调用这个递归即可。然后注意一下pte转物理页地址，就是右移10位再左移12位。

#### kernel page table per process

* 首先在proc中添加对应数据结构

```c
struct proc {
  struct spinlock lock;

  // p->lock must be held when using these:
  enum procstate state;        // Process state
  struct proc *parent;         // Parent process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  int xstate;                  // Exit status to be returned to parent's wait
  int pid;                     // Process ID

  // these are private to the process, so p->lock need not be held.
  uint64 kstack;               // Virtual address of kernel stack
  uint64 sz;                   // Size of process memory (bytes)
  pagetable_t pagetable;       // User page table
  pagetable_t kpgtbl;          // Each process's kernel page table
  struct trapframe *trapframe; // data page for trampoline.S
  struct context context;      // swtch() here to run process
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
};
```



* 之后就是在allocproc的时候，应该对进程的内核页表进行初始化，关于per-process kernel table的初始化，**之后再copy in里面有提到目前用户进程的虚拟地址是不能超过PLIC,所以我们只需要重新映射低于PLIC地址的部分即可，因为高于PLIC部分不会被用户修改到，所以就可以直接复制kernel table的1-511item，因为PLIC的一级页表index是0，只有第0个的pte以及下面对应的次级页表和leaf页表才需要被重新映射**

```c
pagetable_t
proc_kerneltable_init(){
  pagetable_t kpgtbl = uvmcreate();
  if(kpgtbl == 0)
    return 0;

  for(int i = 1; i < 512; ++i)
    kpgtbl[i] = kernel_pagetable[i];

  uvmmap(kpgtbl, UART0, UART0, PGSIZE, PTE_R | PTE_W);
  uvmmap(kpgtbl, VIRTIO0, VIRTIO0, PGSIZE, PTE_R | PTE_W);
  uvmmap(kpgtbl, CLINT, CLINT, 0x10000, PTE_R | PTE_W);
  uvmmap(kpgtbl, PLIC, PLIC, 0x400000, PTE_R | PTE_W);
  return kpgtbl;
}

void
uvmmap(pagetable_t pgtbl, uint64 va, uint64 pa, uint64 sz, int perm){
  if(mappages(pgtbl, va, sz, pa, perm) != 0)
    panic("uvmmap");
}
```

* hint的第三点有提到kernel stack的映射需要存在这个进程的kernel pagetable，因为kernel stack的位置位于高位，在我们之前复制页表项的时候就已经包含了，所以这种写法不用做额外处理。

* 另外就是进程调度的时候，需要切换对应的内核页表，其实就是加载satp寄存器，然后刷新TLB即可。这里需要注意的就是切换页表的位置，应该在切换context之前，因为切换完context之后PC存储的值就是将要执行进程的指令了，下次切换回来就得等进程自己切回来了。切换回来之后重新进入内核，再将页表换为唯一内核页表即可。
* 另外就是释放一个进程的时候需要将进程的内核页表也回收掉，但是不能clear 物理内存，不然内核就G了

```c
void uvmfreekpgtbl(pagetable_t pgtbl){
  pagetable_t midlevelpgtbl = (pagetable_t)PTE2PA(pgtbl[0]);

  for(int i = 0; i < 512; ++i){
    pte_t pte = midlevelpgtbl[i];
    if(pte & PTE_V){
      kfree((void*)PTE2PA(pte));
    }
    midlevelpgtbl[i] = 0;
  }
  kfree((void*)midlevelpgtbl);
  kfree((void*)pgtbl);
}
```

#### simplify copy in

相对来说比较简单，仔细看copyin_new，就是把之前通过页表找物理地址的部分给去掉了，所以只要把用户页表的映射给弄到process 的kernel页表中就好了。**主要是hint中已经把所有需要更新的地方都告诉我们了，所以只需要将同步页表的function在这几个地方进行调用即可。**

```c
void
copypagetable(pagetable_t src_pagetable, pagetable_t dst_pagetable, uint64 start, uint64 end){
  if(end > PLIC)
    panic("user virtual addr is higher than PLIC");
  // end must be page aligned.
  if(start < end){
    uint64 currva = PGROUNDUP(start);
    for(; currva <= end; currva += PGSIZE){
      // search startva address and map that in dst_pagetable
      pte_t *srcpte = walk(src_pagetable, currva, 0);
      if(srcpte == 0 || !(*srcpte & PTE_V))
        continue;
      // mappages(dst_pagetable, currva, PGSIZE, (uint64)PTE2PA(*srcpte), perm);
      // 这里不能用这个，因为可能会发生remap
      pte_t* dstpte = 0;
      if((dstpte = walk(dst_pagetable, currva, 1)) == 0)
        panic("kalloc for dst pgtable failed");
      *dstpte = *srcpte & (~(PTE_U | PTE_W | PTE_X));
    }
  }
  else{
      if(PGROUNDUP(end) < PGROUNDUP(start)){
      int npages = (PGROUNDUP(start) - PGROUNDUP(end)) / PGSIZE;
      uvmunmap(dst_pagetable, PGROUNDUP(end), npages, 0);
    }
  }
}
```

growproc向下增长时候我们应该同样取消对应虚拟地址在kernel table中的mapping，在网上看到许多做法都没有取消mapping，感觉这种做法还是不太合理，可能因为user 系统调用传进来的参数不会指向那个地址，所以不清理也没有关系，但是在系统设计里面，这种就属于只吃不擦，add和clear的时候应该都做同步才对。