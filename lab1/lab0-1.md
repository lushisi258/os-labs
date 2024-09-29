## lab0

#### 说明RICV-V硬件加电后的几条指令在哪里？完成了哪些功能？
- 最初加电时的指令在```0x1000```，为```auipc   t0,0x0```
- 然后执行到```0x1014```位置的命令时，通过```jr      t0```跳转到了OpenSBI的程序入口```0x80000000```处，执行OpenSBI的引导程序
- 随后跳转到```0x80200000```处，执行操作系统的内核启动命令：
    ```
    Breakpoint 2, kern_entry () at kern/init/entry.S:7
    7           la sp, bootstacktop
    (gdb) x/i $pc 
    => 0x80200000 <kern_entry>:     auipc   sp,0x3
    ```



## lab1

#### 阅读```kern/init/entry.S```内容代码，结合操作系统内核启动流程，说明指令```la sp, bootstacktop```完成了什么操作，目的是什么？```tail kern_init```完成了什么操作，目的是什么？

1. ```la sp, bootstacktop```完成的操作是将符号地址```bootstacktop```加载到栈指针```sp```处，目的是设置内核启动过程中栈的起始位置

2. ```tail kern_init```完成的操作是尾调用优化```kern_init```，即直接跳转到内核启动函数，目的是跳转到真正的内核启动函数```kern_init```

#### 定时器的实现过程和中断处理的流程

1. **定时器的实现：**
   - 实现内容：使操作系统每遇到100次时钟中断后，调用print_ticks子程序，向屏幕上打印一行文字”100 ticks”，在打印完10行后调用sbi.h中的shut_down()函数关机
   - 实现代码：
        ```
        // 设置下次时钟中断
        clock_set_next_event();
        // 计数器ticks加一
        ticks++;
        // 当计数器ticks加到100时，输出消息并增加num计数器
        if (ticks == 100) {
            print_ticks();
            ticks = 0; // 重置ticks计数器
            num++;
        }
        // 判断打印次数num，当num达到10时，调用关机函数
        if (num == 10) {
            sbi_shutdown();
        }
        ```

2. **中断处理的流程**
   - 当中断产生时，首先把CPU的寄存器内容（也就是上下文context，在这里是以trapFrame结构体的形式：包含32个通用寄存器和4个和中断相关的CSR）保存到内存上
   - ```trapentry.S```将中断入口点的上下文保存起来，并送到中断处理程序```trap.c```处
   - ```trap.c```执行中断处理程序，实现定时器功能
   - 当中断结束时，将CPU的寄存器内容从内存上加载回来