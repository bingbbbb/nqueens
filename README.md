# nqueens
将n皇后问题用多线程、多机器并行运算的方法实现。

***
## 多机器使用方法
* 主机
> git clone https://github.com/bingbbbb/nqueens.git  
> cd nqueens/MultiMachine  
> bash ./build.sh  
设置相关参数(vim ./conf/nqueens.conf)  
> ./bin/mainPc  
* 其他机器
> git clone https://github.com/bingbbbb/nqueens.git  
> cd nqueens/MultiMachine  
> bash ./build.sh  
设置相关参数(vim ./conf/nqueens.conf)  
> ./bin/otherPc  (在mainPc之后运行)  

./conf/nqueens.conf文件的参数设置在文件里面有说明  

***
## 实验结果  
下面是18皇后的计算时间
| 方法 | 时间 |
|:---|:---|
| 单线程       | 578s |
| 3线程        | 196s |
| 6线程        | 100s |
| 单机6进程     | 102s |
| 6线程两台机器  | 56s |