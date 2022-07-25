### XV6 book traps笔记

* 三种事件会打断cpu令正常执行去处理这些事件，系统调用，指令引发的异常，以及外部硬件的中断。
* xv6用trap统一代替代替以上提到的这三种情况，我们希望被中断的执行代码对于自己的执行被traps中断这件事是不知情的（透明的）
* 大致流程：
  * trap transfer control to kernel
  * kernel saved registers and states need to resumed later
  * kernel execute handler code
  * kernel restored saved state and register, finally return from trap.
  * the original code resume executing
* traps一般分为三类：user space发生的，kernel space发生的以及时钟trap。
* RISC-V与trap有关的寄存器：
  * stvec: trap handler的地址，由kernel写入。RISC-V汇编就会跳转到这个寄存器指向的地址进行trap处理。
  * sepc：之前被中断程序pc指向的位置，从trap中返回时，kernel会将sepc的值写入pc寄存器
  * scause: 存储发生trap的原因标号
  * sscratch: 没有说具体，书上只提到了trap handle最开始的时候会用到。
  * sstatus: 含有各种标记位，SIE表示是否允许硬件中断，SPP标识trap是来自于user mode还是supervisor mode.