#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <math.h>
#include <time.h> 
#include <dirent.h>
#include <sys/stat.h>

//接收报文中的参数
typedef struct parameter_unit 
{  
    char field[128];			//字段
    char value[128];			//值
}parameter_unit; 

//接收报文的集合
typedef struct parameter_pkg  
{  
    parameter_unit	pkg[32];	//最多存放32个参数
    int 			count;		//当前存在参数的数量
}parameter_pkg; 

//传输的文件信息
typedef struct file_info  
{  
	int 	id;
    char	name[128];
	char	md5[128];
}file_info; 


//降序
static int desc_file_floder ( const void *a , const void *b) 
{          
	return (*(file_info *)a).id < (*(file_info *)b).id ? 1 : -1; 
}  
int get_server_ip(char*ip,char*network_name);
int post_messages(char*ip,int port);
int creat_server_socket(char*ip,int port);
int make_write_buf(char*read_buf,int read_len,int client_fd);
int get_test_id_by_server(parameter_pkg data,int client_fd);
parameter_pkg extract_parameter(char*read_buf);


int main(int argc, char *argv[])
{
	char ip[64]={0};
	//1	获取网卡ip地址
	get_server_ip(ip,"eth0");	
	printf("aliyun eth0 ip=%s\n",ip);
	post_messages(ip,8890);		//2	不断的接受，发送数据
	return 0;
}

//这里是类似post的方式传递message
int post_messages(char*ip,int port)
{
	char read_buf[1024];
	memset(read_buf,0,sizeof(read_buf));
	struct sockaddr_in client_socket;
	socklen_t client_addrlen;
	
	//第一步：建立客户端套接字
	int server_socket = creat_server_socket(ip,port);
	
	
	while(1)
	{
		memset(&client_socket, 0, sizeof(struct sockaddr_in));  
    	client_addrlen = 160;  
		int client_fd = accept(server_socket,(struct sockaddr*)&client_socket,&client_addrlen);
		if(client_fd==-1){
			printf("client_fd=-1 web over\n");
			return 0;
		}
		memset(read_buf,0,sizeof(read_buf));
		int read_len=read(client_fd,read_buf,sizeof(read_buf));
		if(read_len>0){
			int ret=make_write_buf(read_buf,read_len,client_fd);
			if(ret == -1){
				close(client_fd);
				break;	
			}
		}else{
			printf("read_len=%d,line=%d\n",read_len,__LINE__);
		}
		close(client_fd);
	}
	close(server_socket);
	return 2;
}

int make_write_buf(char*read_buf,int read_len,int client_fd)
{
	if(read_buf[0]=='#'&&read_buf[1]=='$'&&read_buf[2]=='#'){
		//提取参数
		parameter_pkg data=extract_parameter(read_buf);
		if(data.count==0){//如果参数的数量是0就退出
			printf("err in fun: %s line = %d\n",__func__,__LINE__);
			return -1;	
		}
		if(data.pkg[0].field){
			if(0==strcmp(data.pkg[0].field,"Fun")){
				if(0==strcmp(data.pkg[0].value,"get_test_id_by_server")){
					get_test_id_by_server(data,client_fd);
				}
			}else{
				//没有功能函数Fun退出
				printf("data.pkg[0].field=%s; line=%d\n",data.pkg[0].field,__LINE__);
				return -1;	
			}
		}else{
			printf("data.pkg[0].field=NULL; line=%d\n",__LINE__);
			return 0;	
		}
	}else{
		//强制报文头是#$#
		printf("err readbuf 012 =%c%c%c,line=%d\n",read_buf[0],read_buf[1],read_buf[2],__LINE__);
		return -1;
	}
	return 0;	
}



int get_test_id_by_server(parameter_pkg data,int client_fd){
	char write_buf[256]={0};
	int	i=0;
	char id[32]={0};
	
	file_info store[10240];
	memset(store,0,sizeof(store)); 
	DIR *pDir;
	struct dirent  *ent;
    pDir=opendir("data/log");
    if(pDir){
		write(client_fd,"err",3);
		return 0;
	}
    while((ent=readdir(pDir))){
        if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)continue; 
        if(ent->d_type ==4){//如果是文件夹
        	sprintf(id,"%c%c%c%c",ent->d_name[0],ent->d_name[1],ent->d_name[2],ent->d_name[3]);
            store[i].id=atoi(id);
            strcpy(store[i].name,ent->d_name);
            if(++i>9999){
        		break;
            }
        }
    }
    closedir(pDir);
    
    //降序排列
    qsort(store,i,sizeof(file_info),desc_file_floder);
    printf("send new test id=%d to R16\n",store[0].id+1);
    sprintf(write_buf,"%d",store[0].id+1);
	write(client_fd,write_buf,strlen(write_buf));
	
	
	return 0;	
}

//提取参数
			//read_buf<=1024
			//count<=32
			//parameter<=256
parameter_pkg extract_parameter(char*read_buf){
	
	parameter_pkg data;
	memset(&data,0,sizeof(parameter_pkg));
	if(!read_buf){
		printf("fun:%s err line=%d\n",__func__,__LINE__);
		memset(&data,0,sizeof(parameter_pkg));
		return data;	
	}
	
	char line_array[32][256]={0};
	char cp_read_buf[1024]={0};				//复制read_buf
	strcpy(cp_read_buf,read_buf);
	char *p=cp_read_buf;					//指向read_buf,可以移动
	char *head=cp_read_buf;					//指向read_buf的每一行的头部
	int i=0;
	int j=0;
											//把报文拆解成一行一行
	while(1){
		if(p[0]=='\0'){						//报文尾部
			if(data.count==0){
				strcpy(line_array[data.count],head+3);
			}else{
				strcpy(line_array[data.count],head);
			}
			++data.count;
			break;	
		}
		if(p[0]=='&'){						//报文中间拆分点
			p[0]='\0';
			if(data.count==0){
				strcpy(line_array[data.count],head+3);
			}else{
				strcpy(line_array[data.count],head);
			}
			++data.count;
			head=p+1;
		}
		++p;
		++i;
		if(i>1024-1){
			printf("fun:%s err read_buf > 1024 line=%d\n",__func__,__LINE__);
			memset(&data,0,sizeof(parameter_pkg));
			return data;	
		}	
	}
											//把每一行报文再一次拆分
	for(i=0;i<data.count;++i){
		
		while(1){
			if(line_array[i][j]=='='){
				line_array[i][j]='\0';
				strcpy(data.pkg[i].field,line_array[i]);
				strcpy(data.pkg[i].value,&line_array[i][j+1]);
				j=0;
				break;
			}
			if(++j>253){
				printf("can not find '=' line=%d\n",__LINE__);	
				memset(&data,0,sizeof(parameter_pkg));
				return data;	
			}
		}
	}
	
	return data;	
}






int creat_server_socket(char*ip,int port)
{
	int server_socket = 0;	
	//1
	server_socket = socket(AF_INET,SOCK_STREAM,0);
	int reuse = 1;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		perror("setsockopet error\n");
		exit(-1);
	}  
	
	//2
	struct sockaddr_in  server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);
	int retval =  bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr));
	if(retval ==-1)
	{
		perror("connect failed...");
		close(server_socket);
		exit(1);
	}
	//3
	retval = listen(server_socket,16);
	return server_socket;	
}
//获取本机ip地址（eth0）
int get_server_ip(char*ip,char*network_name)
{
	int sockfd;
    struct ifconf ifconf;
    struct ifreq *ifreq;
    char buf[512];
    ifconf.ifc_len =512;
    ifconf.ifc_buf = buf;
    if ((sockfd =socket(AF_INET,SOCK_DGRAM,0))<0)
    {
        perror("socket" );
        exit(1);
    }
    ioctl(sockfd, SIOCGIFCONF, &ifconf); 

    ifreq = (struct ifreq*)ifconf.ifc_buf;
    int i=(ifconf.ifc_len/sizeof (struct ifreq));
    for (; i>0; i--)
    {
        if(ifreq->ifr_flags == AF_INET){ //for ipv4
            if(0 == strcmp(network_name,ifreq->ifr_name))
            {
            	strcpy(ip ,inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr));
            	return 0;	
        	}
        	ifreq++;
        }
    }
    close(sockfd);
	return 0;	
}
