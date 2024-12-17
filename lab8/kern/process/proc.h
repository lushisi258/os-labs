#ifndef __KERN_PROCESS_PROC_H__
#define __KERN_PROCESS_PROC_H__

#include <defs.h>
#include <list.h>
#include <trap.h>
#include <memlayout.h>
#include <skew_heap.h>

// process's state in his life cycle
enum proc_state {
    PROC_UNINIT = 0,  // uninitialized
    PROC_SLEEPING,    // sleeping
    PROC_RUNNABLE,    // runnable(maybe running)
    PROC_ZOMBIE,      // almost dead, and wait parent proc to reclaim his resource
};

struct context {
    uintptr_t ra;
    uintptr_t sp;
    uintptr_t s0;
    uintptr_t s1;
    uintptr_t s2;
    uintptr_t s3;
    uintptr_t s4;
    uintptr_t s5;
    uintptr_t s6;
    uintptr_t s7;
    uintptr_t s8;
    uintptr_t s9;
    uintptr_t s10;
    uintptr_t s11;
};

#define PROC_NAME_LEN               15
#define MAX_PROCESS                 4096
#define MAX_PID                     (MAX_PROCESS * 2)

extern list_entry_t proc_list;

struct inode;

struct proc_struct {
    enum proc_state state;                      // Process state，进程状态
    int pid;                                    // Process ID，进程ID
    int runs;                                   // the running times of Proces，进程运行次数
    uintptr_t kstack;                           // Process kernel stack，进程内核栈
    volatile bool need_resched;                 // bool value: need to be rescheduled to release CPU?，是否需要重新调度
    struct proc_struct *parent;                 // the parent process，父进程
    struct mm_struct *mm;                       // Process's memory management field，进程的内存管理结构
    struct context context;                     // Switch here to run process，进程上下文
    struct trapframe *tf;                       // Trap frame for current interrupt，当前中断的中断帧
    uintptr_t cr3;                              // CR3 register: the base addr of Page Directroy Table(PDT)，CR3寄存器：页目录表的基地址
    uint32_t flags;                             // Process flag，进程标志
    char name[PROC_NAME_LEN + 1];               // Process name，进程名
    list_entry_t list_link;                     // Process link list ，进程链表
    list_entry_t hash_link;                     // Process hash list，进程哈希链表
    int exit_code;                              // exit code (be sent to parent proc)，退出码(发送给父进程)
    uint32_t wait_state;                        // waiting state，等待状态
    struct proc_struct *cptr, *yptr, *optr;     // relations between processes，进程间的关系
    struct run_queue *rq;                       // running queue contains Process，运行队列包含进程
    list_entry_t run_link;                      // the entry linked in run queue，链接到运行队列的条目
    int time_slice;                             // time slice for occupying the CPU，占用CPU的时间片
    skew_heap_entry_t lab6_run_pool;            // FOR LAB6 ONLY: the entry in the run pool，仅用于LAB6：运行池中的条目
    uint32_t lab6_stride;                       // FOR LAB6 ONLY: the current stride of the process，仅用于LAB6：进程的当前步幅
    uint32_t lab6_priority;                     // FOR LAB6 ONLY: the priority of process, set by lab6_set_priority(uint32_t)，仅用于LAB6：进程的优先级，由lab6_set_priority(uint32_t)设置
    struct files_struct *filesp;                // the file related info(pwd, files_count, files_array, fs_semaphore) of process，进程的文件相关信息(pwd, files_count, files_array, fs_semaphore)
};

#define PF_EXITING                  0x00000001      // getting shutdown

#define WT_CHILD                    (0x00000001 | WT_INTERRUPTED)
#define WT_INTERRUPTED               0x80000000                    // the wait state could be interrupted

#define WT_CHILD                    (0x00000001 | WT_INTERRUPTED)  // wait child process
#define WT_KSEM                      0x00000100                    // wait kernel semaphore
#define WT_TIMER                    (0x00000002 | WT_INTERRUPTED)  // wait timer
#define WT_KBD                      (0x00000004 | WT_INTERRUPTED)  // wait the input of keyboard

#define le2proc(le, member)         \
    to_struct((le), struct proc_struct, member)

extern struct proc_struct *idleproc, *initproc, *current;

void proc_init(void);
void proc_run(struct proc_struct *proc);
int kernel_thread(int (*fn)(void *), void *arg, uint32_t clone_flags);

char *set_proc_name(struct proc_struct *proc, const char *name);
char *get_proc_name(struct proc_struct *proc);
void cpu_idle(void) __attribute__((noreturn));

//FOR LAB6, set the process's priority (bigger value will get more CPU time)
void lab6_set_priority(uint32_t priority);


struct proc_struct *find_proc(int pid);
int do_fork(uint32_t clone_flags, uintptr_t stack, struct trapframe *tf);
int do_exit(int error_code);
int do_yield(void);
int do_execve(const char *name, int argc, const char **argv);
int do_wait(int pid, int *code_store);
int do_kill(int pid);
int do_sleep(unsigned int time);
#endif /* !__KERN_PROCESS_PROC_H__ */

