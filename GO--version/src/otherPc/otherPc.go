package main
import (
		"fmt"
		"nqueens"
		"log"
		"net"
		"net/http"
		"net/rpc"
		"os"
)
type Arith struct {
}

type result_go struct{//各个协程返回result_go到主线程
	which_go int32
	sum int64
}
var(
	chan_go_done = make(chan bool)//协程完成发出信息

	chan_result_go = make(chan result_go, 6)//从各个协程收集结果
	chans_go [nqueens.N_otherPc_goroutine]chan int//向各个协程写信息

	tasks []int32
)
func cal_goroutine(i int32){//i用来标记协程,每个协程运行该函数
	var result result_go
	result.which_go = i
	result.sum = 0
	for {
		start := <- chans_go[i]
		end := <- chans_go[i]
		result.sum = 0
		for i := start;i < end; i = i + 3{
			nqueens.Cal_task_go(tasks[i], tasks[i+1], tasks[i+2], &(result.sum))
		}
		chan_result_go <- result
	}
}
func init_func(){
	for i := range chans_go {//
		chans_go[i] = make(chan int)
	}

	var i int32
	for i = 0;i < nqueens.N_otherPc_goroutine;i++ {//启动协程
		go cal_goroutine(i)
	}
}
func (this *Arith) Cal_task_otherPc(tasks_ []int32, sum *int64) error {
	/**len := len(tasks_)
	fmt.Printf("len : %d\n", len)
	for i := 0;i < len; i = i + 3{
		nqueens.Cal_task_go(tasks_[i], tasks_[i+1], tasks_[i+2], sum)
	}*/
	
	fmt.Printf("receive task len : %d\n", len(tasks_))
	tasks = tasks_
	quit := false
	len := len(tasks_)
	start := 0
	undone_go := 0
	var i int32
	for i = 0;i < nqueens.N_otherPc_goroutine;i++ {//分配协程任务
		if start != len {
			chans_go[i] <- start
			start = nqueens.Min(start + 3 * nqueens.Task_num_go, len)
			chans_go[i] <- start
			undone_go++
		}
	}
	for ;quit == false; {
		select{
		case result := <- chan_result_go :
			*sum += result.sum
			if start != len{
				chans_go[result.which_go] <- start
				start = nqueens.Min(start + 3 * nqueens.Task_num_go, len)
				chans_go[result.which_go] <- start 
			}else{
				undone_go--
			}
		}
		if undone_go == 0 {
			break
		}
	}
    return nil
}

func main(){
	init_func()
	rpc.Register(new(Arith)) // 注册rpc服务
    rpc.HandleHTTP()         // 采用http协议作为rpc载体

    lis, err := net.Listen("tcp", nqueens.Sockets[0])
    if err != nil {
        log.Fatalln("fatal error: ", err)
    }

    fmt.Fprintf(os.Stdout, "%s", "start connection")

    http.Serve(lis, nil)
}