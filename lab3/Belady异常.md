# Belady 异常

## 什么是 Belady 异常
Belady 异常是指在某些页面置换算法中，增加分配给进程的页面数反而导致缺页率增加的现象。通常情况下，我们期望增加页面数能够减少缺页率，但 Belady 异常表明在某些情况下，情况恰恰相反。

## 为什么会出现 Belady 异常
Belady 异常主要出现在 FIFO（先进先出）页面置换算法中。FIFO 算法简单且直观，但它并不考虑页面的使用频率和未来的使用情况，只是简单地将最早进入内存的页面置换出去。这种策略可能会导致一些频繁使用的页面被置换出去，从而增加缺页率。