package main
import (
		"fmt"
		"time"
		"nqueens"
)
type result_go struct{//各个协程返回result_go到主线程
	which_go int32
	sum int64
}
var(
	chan_result_go = make(chan result_go, 6)//从各个协程收集结果
	chan_go_end = make(chan bool)//协程结束发出信息
	chans [nqueens.N_goroutine]chan int
)
func Cal_goroutine(i int32){//i用来标记协程
	var result result_go
	result.which_go = i
	result.sum = 0
	for {
		chan_result_go <- result
		start := <- chans[i]
		end := <- chans[i]
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
func init_func() {
	for i := range chans {//
		chans[i] = make(chan int)
	}

	var i int32
	for i = 0;i < nqueens.N_goroutine;i++ {//启动协程
		go Cal_goroutine(i)
	}
}
func Min(x, y int) int {
    if x < y {
        return x
    }
    return y
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
		lived_go = nqueens.N_goroutine
		quit = false
		len = len(nqueens.Tasks)
	)
	init_func()
	for ;quit == false; {
		select {
		case result := <- chan_result_go :
			nqueens.Sum += result.sum
			if start != len{
				chans[result.which_go] <- start
				start = Min(start + 3 * nqueens.Task_num, len)
				chans[result.which_go] <- start 
			}else{
				chans[result.which_go] <- start
				chans[result.which_go] <- start
			}
			//fmt.Printf("case 1\n")
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
	fmt.Printf("cost_time = [%s]\n",cost_time)	
	fmt.Printf("result : %d\n", nqueens.Sum)
}

/**
Cal_task_go  N = 18
不开协程	6m53
1个协程		7m39
6个协程		1m22
*/