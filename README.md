#!/bin/bash

echo "需要的软件包 nasm gcc make 这几个软件包centos6.X自带的官网yum仓库里就有"
echo "直接运行 yum -y install nasm gcc make即可"
echo "其中gcc的版本推荐使用4.4.7版本"
echo ""
echo "先运行./configure 不需要任何参数"
echo "看到 \"configure is ok\" 的时候表示执行成功了"
echo "如果有error则缺少相应的软件包"
echo ""
echo "configure完成以后执行make命令即可"
echo "make之后会生成可执行文件syn"
echo "执行./syn即可"
