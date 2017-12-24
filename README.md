
# 南京大学编译原理实验

lab1: 词法分析　语法分析

lab2: 语义分析

lab3: 中间代码生成

lab4: 机器代码生成

仅供参考 抄袭后果自负



## java-2017课程第四次作业

### 程序简介:

- 本程序修改自原版程序[concurrency/waxomatic/WaxOMatic.java](https://github.com/njuics/java-2017f/blob/master/examples/Concurrency/src/main/java/concurrency/waxomatic/WaxOMatic.java)

- 首先启动两个打蜡线程WaxOn1, WaxOn2和一个抛光线程WaxOff

- 这三个线程的先后启动顺序是不确定的, 但要最初就保证WaxOn(WaxOn1,WaxOn2中的一个并且只能为其中一个)先执行，所以WaxOff线程启动后先挂起线程.

- 原版程序已经实现了WaxOn和WaxOff的线程同步, 打蜡线程和抛光线程可以交替执行, 现在需要做的是将两个WaxOn线程的互斥.

- 最开始使用this当前对象来互斥, 结果出现问题, 最后改为用this.getClass()来用WaxOn类的Class文件对象来实现互斥.

----

### 命令行中运行步骤:

- 进入到 './20171128/汪值-141270037/' 目录下

- 编译: 'javac concurrency/waxomatic/WaxOMatic.java' 

- 运行: 'java javac concurrency/waxomatic/WaxOMatic' 

某次运行结果:

```
WaxOn1 :WaxOn!
Wax Off! 
WaxOn2 :WaxOn!
Wax Off! 
WaxOn2 :WaxOn!
Wax Off! 
WaxOn1 :WaxOn!
Wax Off! 
WaxOn2 :WaxOn!
Wax Off! 
WaxOn1 :WaxOn!
Wax Off! 
WaxOn1 :WaxOn!
Wax Off! 
WaxOn2 :WaxOn!
Wax Off! 
WaxOn2 :WaxOn!
Wax Off! 
WaxOn1 :WaxOn!
Wax Off! 
WaxOn2 :WaxOn!
Wax Off! 
WaxOn1 :WaxOn!
Wax Off! 
WaxOn1 :WaxOn!
Exiting via interrupt
Ending WaxOn task: WaxOn1
WaxOn2 :WaxOn!
Exiting via interrupt
Ending WaxOn task: WaxOn2
Exiting via interrupt
Ending Wax Off task
```

