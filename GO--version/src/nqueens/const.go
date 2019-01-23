package nqueens

const(
	N = 18	  					//n皇后问题
	Task_level = 3				//控制产生任务数量
	Task_num_go = 3				//每次给协程任务数量
	Task_num_otherPc = 36		//每次给其他机器任务数量

	N_goroutine = 6				//主机协程数
	N_otherPc = 1				//除主机外有几台机器
	N_otherPc_goroutine = 6 	//其他机器协程数
)
var Sockets = [N_otherPc]string{"116.57.121.47:8095"}	//其他机器ip及端口