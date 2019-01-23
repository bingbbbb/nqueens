if [ ! -d bin ] # 判断是否有bin这个目录
then
    mkdir bin
fi

if [ ! -d pkg ] # 判断是否有pkg这个目录
then
    mkdir pkg
fi

export GOPATH=`pwd`
go install -gcflags "-m" main
go install -gcflags "-m" otherPc
#./bin/otherPc