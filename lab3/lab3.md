### 练习

对实验报告的要求：
 - 基于markdown格式来完成，以文本方式为主
 - 填写各个基本练习中要求完成的报告内容
 - 完成实验后，请分析ucore_lab中提供的参考答案，并请在实验报告中说明你的实现与参考答案的区别
 - 列出你认为本实验中重要的知识点，以及与对应的OS原理中的知识点，并简要说明你对二者的含义，关系，差异等方面的理解（也可能出现实验中的知识点没有对应的原理知识点）
 - 列出你认为OS原理中很重要，但在实验中没有对应上的知识点
 
#### 练习0：填写已有实验
本实验依赖实验1/2。请把你做的实验1/2的代码填入本实验中代码中有“LAB1”,“LAB2”的注释相应部分。

#### 练习1：理解基于FIFO的页面替换算法（思考题）
描述FIFO页面置换算法下，一个页面从被换入到被换出的过程中，会经过代码里哪些函数/宏的处理（或者说，需要调用哪些函数/宏），并用简单的一两句话描述每个函数在过程中做了什么？（为了方便同学们完成练习，所以实际上我们的项目代码和实验指导的还是略有不同，例如我们将FIFO页面置换算法头文件的大部分代码放在了`kern/mm/swap_fifo.c`文件中，这点请同学们注意）
 - 至少正确指出10个不同的函数分别做了什么？如果少于10个将酌情给分。我们认为只要函数原型不同，就算两个不同的函数。要求指出对执行过程有实际影响,删去后会导致输出结果不同的函数（例如assert）而不是cprintf这样的函数。如果你选择的函数不能完整地体现”从换入到换出“的过程，比如10个函数都是页面换入的时候调用的，或者解释功能的时候只解释了这10个函数在页面换入时的功能，那么也会扣除一定的分数

 1. `pgfault_handler`: 页错误异常处理函数，调用`do_pgfault`来处理异常，输出相关信息
 2. `do_pgfault`: 处理页错误，完成页面替换
 3. `find_vma`: 返回要处理的这个页的虚拟地址结构体，为页面替换做准备
 4. `get_pte`: 找到当前页面的交换条目
 5. `pgdir_alloc_page`: 如果当前页不存在，分配页内存，设置虚拟地址映射
 6. `alloc_pages`: 分配页内存，如果没有空闲，调用`swap_out`来换出页面
 7. `_fifo_swap_out_victim`: `swap_out`所指向的函数，根据先进先出规则换出最早进入的页
 8. `swap_in`: 读取造成缺页异常的地址的磁盘页的内容，将这个页换入内存
 9. `page_insert`: 将换入的页的内存地址与虚拟地址之间建立映射
 10. `_fifo_map_swappable`: `swap_map_swapppable`所指向的函数，插入页

#### 练习2：深入理解不同分页模式的工作原理（思考题）
get_pte()函数（位于`kern/mm/pmm.c`）用于在页表中查找或创建页表项，从而实现对指定线性地址对应的物理页的访问和映射操作。这在操作系统中的分页机制下，是实现虚拟内存与物理内存之间映射关系非常重要的内容。
 - get_pte()函数中有两段形式类似的代码， 结合sv32，sv39，sv48的异同，解释这两段代码为什么如此相像。
 - 目前get_pte()函数将页表项的查找和页表项的分配合并在一个函数里，你认为这种写法好吗？有没有必要把两个功能拆开？

 1. 代码为什么如此相像：
    - 因为sv32, sv39, sv48都是层级页表结构，层与层之间的关系相似，所以从一级目录索引到二级目录索引和二级目录索引到具体页之间的关系相似，所以这两段代码的形式相似
 2. 写法好吗？有没有必要拆开？
    - 我觉得写法好，没有必要拆开。因为我们操作的是具体的页，所以要求一级目录和二级目录都应该映射正常，这样才能保证正常操作页

#### 练习3：给未被映射的地址映射上物理页（需要编程）
补充完成do_pgfault（mm/vmm.c）函数，给未被映射的地址映射上物理页。设置访问权限 的时候需要参考页面所在 VMA 的权限，同时需要注意映射物理页时需要操作内存控制 结构所指定的页表，而不是内核的页表。
请在实验报告中简要说明你的设计实现过程。请回答如下问题：
 - 请描述页目录项（Page Directory Entry）和页表项（Page Table Entry）中组成部分对ucore实现页替换算法的潜在用处。
 - 如果ucore的缺页服务例程在执行过程中访问内存，出现了页访问异常，请问硬件要做哪些事情？
 - 数据结构Page的全局变量（其实是一个数组）的每一项与页表中的页目录项和页表项有无对应关系？如果有，其对应关系是啥？

##### 代码补全
```c
int do_pgfault(struct mm_struct *mm, uint_t error_code, uintptr_t addr) {
    int ret = -E_INVAL;
    // try to find a vma which include addr
    struct vma_struct *vma = find_vma(mm, addr);

    pgfault_num++;
    // If the addr is in the range of a mm's vma?
    if (vma == NULL || vma->vm_start > addr) {
        cprintf("not valid addr %x, and  can not find it in vma\n", addr);
        goto failed;
    }

    uint32_t perm = PTE_U;
    if (vma->vm_flags & VM_WRITE) {
        perm |= (PTE_R | PTE_W);
    }
    addr = ROUNDDOWN(addr, PGSIZE);

    ret = -E_NO_MEM;

    pte_t *ptep = NULL;

    ptep = get_pte(mm->pgdir, addr, 1); //(1) try to find a pte, if pte's
                                        // PT(Page Table) isn't existed, then
                                        // create a PT.
    if (*ptep == 0) {
        if (pgdir_alloc_page(mm->pgdir, addr, perm) == NULL) {
            cprintf("pgdir_alloc_page in do_pgfault failed\n");
            goto failed;
        }
    } else {
        /*LAB3 EXERCISE 3: 2213601
         * 请你根据以下信息提示，补充函数
         * 现在我们认为pte是一个交换条目，那我们应该从磁盘加载数据并放到带有phy
         * addr的页面， 并将phy
         * addr与逻辑addr映射，触发交换管理器记录该页面的访问情况
         *
         *  一些有用的宏和定义，可能会对你接下来代码的编写产生帮助(显然是有帮助的)
         *  宏或函数:
         *    swap_in(mm, addr, &page) : 分配一个内存页，然后根据
         *    PTE中的swap条目的addr，找到磁盘页的地址，将磁盘页的内容读入这个内存页
         *    page_insert ： 建立一个Page的phy addr与线性addr la的映射
         *    swap_map_swappable ： 设置页面可交换
         */
        if (swap_init_ok) {
            struct Page *page = NULL;

            // 创建一个页，读取造成缺页异常的地址的磁盘页的内容
            swap_in(mm, addr, &page);
            // 将页的物理地址与逻辑地址建立映射
            page_insert(mm->pgdir, page, addr, perm);
            // 设置页面可交换
            swap_map_swappable(mm, addr, page, 1);

            page->pra_vaddr = addr;
        } else {
            cprintf("no swap_init_ok but ptep is %x, failed\n", *ptep);
            goto failed;
        }
    }

    ret = 0;
failed:
    return ret;
}
```

##### 问题
1. 潜在用处：
 - 存储一些标志位，辅助判断，比如合法位，权限位等
2. 硬件做的事情：
 - 根据`stvec`跳转到中断处理程序的地址
 - 根据中断处理程序的逻辑，跳转调用具体的程序处理地址
3. 有对应关系，对应关系为：
 - `Page`中的自定义的`visited`字段对应`PTE_A`标识该页是否被访问过

#### 练习4：补充完成Clock页替换算法（需要编程）
通过之前的练习，相信大家对FIFO的页面替换算法有了更深入的了解，现在请在我们给出的框架上，填写代码，实现 Clock页替换算法（mm/swap_clock.c）。
请在实验报告中简要说明你的设计实现过程。请回答如下问题：
 - 比较Clock页替换算法和FIFO算法的不同。

##### 实现过程
Clock页替换算法：
1. 增加`visited`标识位用来标记页面是否被访问过
2. 在选择换出页时，遍历，如果页被访问过，将标识位改为未被访问过；如果页未被访问，将页换出

##### 不同之处
- Clock页替换算法多了一个标识位`visited`，并在一定程度上优先保留了重要页，FIFO算法没有用到这个标识位
- FIFO算法只是简单的将最早进入的页面换出，Clock算法则参考了`visited`标识位，二者在换出页面选择上的逻辑不同

#### 练习5：阅读代码和实现手册，理解页表映射方式相关知识（思考题）
如果我们采用”一个大页“ 的页表映射方式，相比分级页表，有什么好处、优势，有什么坏处、风险？

1. 好处
 - 页表映射的结构更简单
 - 页表查找速度加快
 - 内存分配更加连续

2. 坏处
 - 虚拟地址空间大小有限
 - 虚拟地址较大的情况下，访问速度下降
 - 容易出现内存碎片
 - 恶意软件可获得的内存范围更大

#### 扩展练习 Challenge：实现不考虑实现开销和效率的LRU页替换算法（需要编程）
challenge部分不是必做部分，不过在正确最后会酌情加分。需写出有详细的设计、分析和测试的实验报告。完成出色的可获得适当加分。

##### LRU 设计
1. **数据结构**：
 - 使用一个双向链表来维护页面的使用顺序。

2. **页面访问**：
 - 每次访问页面时，将该页面移动到链表头部，表示最近使用。

3. **页面插入**：
 - 将新页面插入到链表头部。
 - 如果内存已满，则移除链表尾部的页面（最久未使用）。

4. **页面换出**：
 - 从链表尾部选择页面进行换出。

##### 流程设计
1. **初始化**：创建一个空的链表用于存储页面。
2. **页面插入**：将新页面插入到链表头部，表示最近使用。
3. **页面换出**：从链表尾部选择页面进行换出，表示最久未使用。

##### 代码实现
1. **初始化函数**：
    ```c
    static int _lru_init_mm(struct mm_struct *mm) {
        list_init(&pra_list_head);
        mm->sm_priv = &pra_list_head;
        return 0;
    }
    ```

2. **页面插入函数**：
    ```c
    static int _lru_map_swappable(struct mm_struct *mm, uintptr_t addr,
                                  struct Page *page, int swap_in) {
        list_entry_t *head = (list_entry_t *)mm->sm_priv;
        list_entry_t *entry = &(page->pra_page_link);
        assert(entry != NULL && head != NULL);
        list_add(head, entry);
        return 0;
    }
    ```

3. **页面换出函数**：
    ```c
    static int _lru_swap_out_victim(struct mm_struct *mm, struct Page **ptr_page,
                                    int in_tick) {
        list_entry_t *head = (list_entry_t *)mm->sm_priv;
        assert(head != NULL);
        assert(in_tick == 0);
        list_entry_t *entry = list_prev(head);
        if (entry != head) {
            cprintf("curr_ptr %x\n", &entry);
            list_del(entry);
            *ptr_page = le2page(entry, pra_page_link);
        } else {
            *ptr_page = NULL;
        }
        return 0;
    }
    ```


