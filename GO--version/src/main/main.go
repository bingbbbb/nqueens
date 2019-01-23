package main
import (
		"fmt"
		"time"
		"nqueens"
		"log"
    	"net/rpc"
)
type result_go struct{//各个协程返回result_go到主线程
	which_go int32
	sum int64
}
var(
	chan_go_end = make(chan bool)//协程结束发出信息

	chan_result_go = make(chan result_go, 6)//从各个协程收集结果
	chans_go [nqueens.N_goroutine]chan int//向各个协程写信息

	chan_result_otherPc = make(chan result_go, 6)//从其他机器收集结果
	chans_go_otherPc [nqueens.N_otherPc]chan int//向其他机器写信息

)
func cal_goroutine(i int32){//i用来标记协程,每个协程运行该函数
	var result result_go
	result.which_go = i
	result.sum = 0
	for {
		chan_result_go <- result
		start := <- chans_go[i]
		end := <- chans_go[i]
		if start == end{
			break
		}
		result.sum = 0
		for i := start;i < end; i = i + 3{
			nqueens.Cal_task_go(nqueens.Tasks[i], nqueens.Tasks[i+1], nqueens.Tasks[i+2], &(result.sum))
		}
	}
	chan_go_end <- true
}
func go_otherPc(i int32){//调用rpc会阻塞，所以开辟协程
	conn, err := rpc.DialHTTP("tcp", nqueens.Sockets[i])
    if err != nil {
        log.Fatalln("dailing error: ", err)
    }
	var result result_go
	result.which_go = i
	result.sum = 0
	for {
		chan_result_otherPc <- result
		start := <- chans_go_otherPc[i]
		end := <- chans_go_otherPc[i]
		if start == end{
			break
		}
		result.sum = 0
		err = conn.Call("Arith.Cal_task_otherPc", nqueens.Tasks[start:end], &(result.sum))
	}
	chan_go_end <- true
}
func init_func() {
	for i := range chans_go {//
		chans_go[i] = make(chan int)
	}

	var i int32
	for i = 0;i < nqueens.N_goroutine;i++ {//启动协程
		go cal_goroutine(i)
	}

	for i := range chans_go_otherPc {//
		chans_go_otherPc[i] = make(chan int)
	}
	for i = 0;i < nqueens.N_otherPc;i++ {//启动协程
		go go_otherPc(i)
	}
}


func main() {
	
	start_time := time.Now()
	//nqueens.Cal_task(0, 0, 0)

	nqueens.Gen_tasks(0, 0, 0, 0)

	/**
	var sum int64;
	sum = 0
	for i := 0;i < len(nqueens.Tasks); i = i + 3{
		nqueens.Cal_task_go(nqueens.Tasks[i], nqueens.Tasks[i+1], nqueens.Tasks[i+2], &sum)
	}
	nqueens.Sum = sum
	*/

	///**
	var(
		start = 0
		lived_go = nqueens.N_goroutine + nqueens.N_otherPc
		quit = false
		len = len(nqueens.Tasks)
	)
	init_func()
	for ;quit == false; {
		select {
		case result := <- chan_result_go :
			nqueens.Sum += result.sum
			if start != len{
				chans_go[result.which_go] <- start
				start = nqueens.Min(start + 3 * nqueens.Task_num_go, len)
				chans_go[result.which_go] <- start 
			}else{
				chans_go[result.which_go] <- start
				chans_go[result.which_go] <- start
			}
			//fmt.Printf("case 1\n")
		case result := <- chan_result_otherPc :
			nqueens.Sum += result.sum
			if start != len{
				chans_go_otherPc[result.which_go] <- start
				start = nqueens.Min(start + 3 * nqueens.Task_num_otherPc, len)
				chans_go_otherPc[result.which_go] <- start 
			}else{
				chans_go_otherPc[result.which_go] <- start
				chans_go_otherPc[result.which_go] <- start
			}
		case <- chan_go_end :
			lived_go--
			//fmt.Printf("case 2\n")
		}
		//fmt.Printf("finish select\n")
		if lived_go == 0{
			break
		}
	}
	//*/




	cost_time := time.Since(start_time)
	fmt.Printf("cost_time = [%s]\n", cost_time)	
	fmt.Printf("result : %d\n", nqueens.Sum)
}

/**
Cal_task_go  N = 18
不开协程		6m53
1个协程			7m39
6个协程			1m22
两台机器6个协程  38s
*/