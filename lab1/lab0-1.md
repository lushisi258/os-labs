### lab0
**1. 说明RICV-V硬件加电后的几条指令在哪里？完成了哪些功能？**
- 最初加电时的指令在```0x1000```，为```auipc   t0,0x0```
- 然后执行到```0x1014```位置的命令时，通过```jr      t0```跳转到了OpenSBI的程序入口```0x80000000```处，执行OpenSBI的引导程序
- 随后跳转到```0x80200000```处，执行操作系统的内核启动命令：
```
Breakpoint 2, kern_entry () at kern/init/entry.S:7
7           la sp, bootstacktop
(gdb) x/i $pc 
=> 0x80200000 <kern_entry>:     auipc   sp,0x3
```

### lab1
**1. 阅读```kern/init/entry.S```内容代码，结合操作系统内核启动流程，说明指令```la sp, bootstacktop```完成了什么操作，目的是什么？```tail kern_init```完成了什么操作，目的是什么？**
- ```la sp, bootstacktop```完成的操作是将符号地址```bootstacktop```加载到栈指针```sp```处，目的是设置内核启动过程中栈的起始位置
- ```tail kern_init```完成的操作是尾调用优化```kern_init```，即直接跳转到内核启动函数，目的是跳转到真正的内核启动函数```kern_init```

**2. 定时器的实现过程和中断处理的流程**