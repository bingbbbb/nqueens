package main

import (
    "errors"
    "fmt"
    "log"
    "net"
    "net/http"
    "net/rpc"
	"os"
	"time"
)

// 算数运算结构体
type Arith struct {
}

// 算数运算请求结构体
type ArithRequest struct {
    A int
    B int
}

// 算数运算响应结构体
type ArithResponse struct {
    Pro int // 乘积
    Quo int // 商
    Rem int // 余数
}

// 乘法运算方法
func (this *Arith) Multiply(req ArithRequest, res *ArithResponse) error {
    res.Pro = req.A * req.B
    return nil
}

// 除法运算方法
func (this *Arith) Divide(req ArithRequest, res *ArithResponse) error {
    if req.B == 0 {
        return errors.New("divide by zero")
    }
    res.Quo = req.A / req.B
	res.Rem = req.A % req.B
	fmt.Println("begin")
    time.Sleep(time.Duration(3)*time.Second)
    fmt.Println("end")
    return nil
}

func main() {
    rpc.Register(new(Arith)) // 注册rpc服务
    rpc.HandleHTTP()         // 采用http协议作为rpc载体

    lis, err := net.Listen("tcp", "127.0.0.1:8095")
    if err != nil {
        log.Fatalln("fatal error: ", err)
    }

    fmt.Fprintf(os.Stdout, "%s", "start connection")

    http.Serve(lis, nil)
}