* 编译
> cd nqueens/GO--version  
设置相关参数(./src/nqueens/const.go)  
> bash ./build.sh  

* 运行  
> ./bin/main  

> ./bin/otherPc  


***
## 实验结果  
18皇后的计算时间

| 方法          | 时间 |  
|:---|:---|  
| 不开协程         | 413s |  
| 1个协程          | 459s |  
| 6个协程          | 82s |  
| 两台机器6协程    | 38s |  

在写GO版本的过程中发现写和c++一样的代码（nqueens/Go--version/src/nqueens/nqueens.go的Cal_task函数和nqueens/nqueens.cpp的test函数），GO运行时间比较短，取消GO编译器优化后，时间基本是一样的，尝试过进行内存分配分析（go run -gcflags "-m -m -m-l"），发现不管有没有优化的内存分配都是一样的，所以应该是其他地方进行了优化，后续有时间再探索。  

GO默认使用编译器优化，我在编译c++时使用命令：g++ nqueens.cpp 是没使用编译器优化的，后来使用编译器优化选项 -O1 -O2 -O3 的17皇后时间分别为 48s, 46s, 45s, GO语言的则为 54s 
