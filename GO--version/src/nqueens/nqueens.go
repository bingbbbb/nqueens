package nqueens

var Sum int64 = 0
var Upperlim int32 = (1 << N) - 1
var Tasks []int32
func Cal_task(row int32, ld int32, rd int32) {
	if row != Upperlim{
		pos := Upperlim & ^(row | ld | rd)
		
		for pos > 0 {
			p := pos & -pos
			pos -= p
			Cal_task(row + p, (ld + p) << 1, (rd + p) >> 1)
		}
	}else{
		Sum++
	}
}

func Cal_task_go(row int32, ld int32, rd int32, sum *int64) {
	if row != Upperlim{
		pos := Upperlim & ^(row | ld | rd)
		
		for pos > 0 {
			p := pos & -pos
			pos -= p
			Cal_task_go(row + p, (ld + p) << 1, (rd + p) >> 1, sum)
		}
	}else{
		(*sum)++
	}
}

func Gen_tasks(row int32, ld int32, rd int32, task_level int32) {
	if (task_level < Task_level){
		pos := Upperlim & ^(row | ld | rd)
		for pos > 0 {
			p := pos & -pos
			pos -= p
			Gen_tasks(row + p, (ld + p) << 1, (rd + p) >> 1, task_level + 1)
		}
	}else{
		Tasks = append(Tasks, row, ld, rd)
	}
}

func Min(x, y int) int {
    if x < y {
        return x
    }
    return y
}

/**
Cal_task()
16		//8s
17		//54s
18		//404s
go run -gcflags "-m -N" main.go  可禁止编译器优化,结果慢于c++
*/