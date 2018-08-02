#include "web.h" 
 
using namespace std;
char CThread_web::total_data[30000];
char CThread_web::send_buf[31000];
char CThread_web::ifname[64];
char CThread_web::map_file_csv_path[128];

//share_memory_api snake_map_point search_parameter 区分地图颜色 dis_arr


//升序
static int asc_jpg_file ( const void *a , const void *b) 
{          
	return (*(remove_jpg_pkg *)a).id > (*(remove_jpg_pkg *)b).id ? 1 : -1; 
}  

static int NumDescSort(const void * a,const void * b)  
{  
    return *(float *)a > *(float *)b ? 1 : -1;
}  	
	
CThread_web::CThread_web()
{   //内存空间初始化，sizeof文件或数据占的内存
	memset(&web_ctrl_info,0,sizeof(Web_Ctrl_Struct));
    memset(total_data,0,sizeof(total_data));
    memset(snake_map_point,0,sizeof(snake_map_point));
    memset(send_buf,0,sizeof(send_buf));
	memset(ifname,0,sizeof(ifname));
	memset(map_file_csv_path,0,sizeof(map_file_csv_path));
	
            
    SM_KEY web_shm_key= WEB_SHM_KEY;
    web_shm_handler = ShareMemApi::CreateShareMem(web_shm_key, sizeof(web_shmu));
    pweb_shm = (web_shmu *)ShareMemApi::MapShareMem(web_shm_handler);
    pweb_shm->head = 0;
    pweb_shm->tail = 0;
    pweb_shm->full = 0;
    pweb_shm->empty = 1;
    pweb_shm->read_index = 0;
    
    
    key_t web_sem_key = WEB_SEM_KEY;
    this->web_sem_id = semget((key_t)web_sem_key, 1, 0666 | IPC_CREAT);
    //程序第一次被调用，初始化信号量
    if(!ShareMemApi::set_semvalue(this->web_sem_id))
    {
        //         fprintf(stderr, "PLATE: Failed to initialize semaphore\n");
        cerr << "PLATE: Failed to initialize semaphore, line:" << __LINE__ << endl;
        exit(EXIT_FAILURE);
    }
    
    
}

CThread_web::~CThread_web()
{
	
}

//删除图片
int CThread_web::remove_all_jpg_file(const char *path)
{
	char remove_path[64]={0};
	//dirent结构体d_name表示文件名
	struct dirent  *ent;
	DIR *pDir;	
	//opendir打开参数指定的目录，返回DIR*形态的目录流，
	//接下来对目录的搜索和读取都要使用该返回值
    pDir=opendir(path);
    if(pDir  == NULL)
	{
		return 0;
	}
    while((ent=readdir(pDir)))
	{
        if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
			continue; 
		//d_type表示档案类型
        if(ent->d_type ==8)
		{	//数值连接
        	sprintf(remove_path,"%s/%s",path,ent->d_name);
        	remove(remove_path);
        }
    }
    closedir(pDir);
	return 0;	
}

//判断平台并获取ip地址
void* CThread_web::process_CThread_web(int mode)
{
	if(mode==0)
	{//R16平台
		strcpy(ifname,"wlan0");
		strcpy(map_file_csv_path,"/tmp/data/map_file.csv");
	}
	else if(mode==1)
	{//ubuntu本机调试
		strcpy(ifname,"lo");
		strcpy(map_file_csv_path,"map_file.csv");
	}
	char ip[64]={0};
	//1	获取wlan0网卡ip地址
	while(1)
	{
		get_server_ip(ip,ifname);	
		if(0!=strcmp(ip,""))
		{
			break;
		}
		sleep(1);
		printf("sleep 1 s	try to find ip\n");
	}
	printf("Sankobot vslam ip=%s\n",ip);
	remove("/tmp/data/Segmentation_Fault_log");
	remove_all_jpg_file("/tmp/data/mapjpg");
	//2	不断的接受，发送post数据
	post_messages(ip,8878);		
	return 0;
}


int CThread_web::make_send_buf(char parameter[][64])
{
	char msg_lenth[64];
	char fun[64]={0};
	//初始化内存
	memset(msg_lenth, 0, sizeof(msg_lenth));
	memset(total_data, 0, sizeof(total_data));
	if(search_parameter(parameter,"Fun"))
	{
		//将获取的Fun函数传给fun
		strcpy(fun,search_parameter(parameter,"Fun"));
			
		//显示slam栅格图
		if(0 == strcmp("add_slam_point",fun))
			add_slam_point(parameter);
			
		//red led map and grid map show
		if(0 == strcmp("show_red_led",fun))
			show_red_led(parameter);
			
		//控制creater行走：前后左右停
		if(0 == strcmp("ctrl_cleanner_robot",fun))
		{
			ctrl_cleanner_robot(parameter);
			//进入临界区
            if(!ShareMemApi::semaphore_p_no_wait(this->web_sem_id))
                return -1;
            else
            {
				//复制web_ctrl_info到pweb_shm
                memcpy(&pweb_shm->web_ctrl_data, &web_ctrl_info, sizeof(web_ctrl_info));
                //printf("\n\nctrl_mode=%d\n",pweb_shm->web_ctrl_data.ctrl_mode);
                pweb_shm->web_ctrl_data.newfg = 1;
                ShareMemApi::semaphore_v(this->web_sem_id);
            }
		}	
		//显示可视化界面
		if(0 == strcmp("play_video",fun))
			play_video(parameter);	
		
		//获取配置文件
		if(0 == strcmp("get_config_val",fun))
			get_config_val(parameter);	
			
		//初始化地图文件(为加速做准备)
		if(0 == strcmp("init_map_file",fun))
			init_map_file(parameter);	
			
		//计算vslam精度
		if(0 == strcmp("calculate_point",fun))
			calculate_point(parameter);	
		
		//decode1024*1024地图
		if(0 == strcmp("get_map_file",fun))
			get_map_file(parameter);	
		
		
		//decode201*201地图
		if(0 == strcmp("get_small_map_file",fun))
			get_small_map_file(parameter);	
		
		//生成手画边界地图
		
		if(0 == strcmp("make_hand_draw_map",fun))
			make_hand_draw_map(parameter);	
		
		
		//draw ring in snake
		if(0 == strcmp("draw_route_in_snake",fun))
			draw_route_in_snake(parameter);
			
		//保存手画地图  
		if(0 == strcmp("save_map_file",fun))
			save_map_file(parameter);
			
		//结束本进程  
		if(0 == strcmp("stop_web_ui_process",fun))
			stop_web_ui_process(parameter);
			
		//初始化调试地图(为加速做准备)  
		if(0 == strcmp("init_debug_map",fun))
			init_debug_map(parameter);
			
		//显示调试地图  
		if(0 == strcmp("add_debug_point",fun))
			add_debug_point(parameter);
		
		//show_door  
		if(0 == strcmp("show_door",fun))
			show_door(parameter);		
			
	}	
	memset(send_buf,0,sizeof(send_buf));
	sprintf(msg_lenth,"Content-Length: %d\r\n\r\n",(int)strlen(total_data));
	//strcat将后面的字符串追加给前面的字符串
	strcat(send_buf,"HTTP/1.1 200 OK\r\n");
	strcat(send_buf, msg_lenth);
	strcat(send_buf, total_data);
	//printf("%s\n",send_buf);
	return 0;
}

//漂移计算
int CThread_web::caculate(char *result)
{
	int pre_x=0;
	int pre_y=0;
	int pre_theta=0;
	long pre_timestamp=0;
	long now_timestamp=0;
	int now_theta=0;
	int now_x=0;
	int now_y=0;
	int dis_xy=0;
	int max_dis_xy=0;
	long sum_dis_xy=0;
	int max_theta=0;
	long sum_theta=0;
	int dis_theta=0;
	int total_point_count=0;
	int dis_unusual_count=0;
	int theta_unusual_count=0;
	char tempRow[8][32];
	char tempRead[128];
	memset(tempRead,0,sizeof(tempRead));
	memset(tempRow,0,sizeof(tempRow));
	FILE *fp_r= NULL;
	fp_r = fopen("/tmp/data/map_points.csv", "r");
    if(!fp_r)
	{
        printf("read map_points file error\n"); 
        return -1;
    }
    
    FILE *fp_w1 = NULL;
    fp_w1 = fopen("/tmp/data/dis_unusual_count.csv", "w");
	if(!fp_w1)
	{
        printf("write map_test_result file error\n");
        return -1;
    }
    FILE *fp_w2 = NULL;
    fp_w2 = fopen("/tmp/data/theta_unusual_count.csv", "w");
	if(!fp_w2)
	{
        printf("write map_test_result file error\n");
        return -1;
    }
    
	while(fgets(tempRead, sizeof(tempRead), fp_r)) 
	{
        while(tempRead[strlen(tempRead) - 1] == '\n' || tempRead[strlen(tempRead) - 1] == ' ') 
        	tempRead[strlen(tempRead) - 1] = '\0';
        if(tempRead[0] == '\0') 
		{
            continue;
        }
        if(++total_point_count>20)
		{
	        sscanf(tempRead, "%s%s%s%s%s%s%s%s",tempRow[0],tempRow[1],
			tempRow[2],tempRow[3],tempRow[4], tempRow[5],tempRow[6], tempRow[7]);
	        if(total_point_count==21)
			{
				pre_x=atoi(tempRow[1]);
				pre_y=atoi(tempRow[2]);
				pre_theta=atoi(tempRow[3]);
				pre_timestamp=atol(tempRow[7]);
			}
			else
			{
				now_x=atoi(tempRow[1]);
				now_y=atoi(tempRow[2]);
				now_theta=atoi(tempRow[3]);
				now_timestamp=atol(tempRow[7]);
				int deta_timestamp=(int)((now_timestamp-pre_timestamp)/1000);
				//printf("deta=%d\n",deta_timestamp);
				dis_xy=sqrt((now_x-pre_x)*(now_x-pre_x)+(now_y-pre_y)*(now_y-pre_y));
				if(pre_theta-now_theta>18000)
				{
					dis_theta=pre_theta-now_theta-36000>0?pre_theta-now_theta-36000:now_theta-pre_theta+36000;
				}
				else if(pre_theta-now_theta<-18000)
				{
					dis_theta=pre_theta-now_theta+36000>0?pre_theta-now_theta+36000:now_theta-pre_theta-36000;
				}else
				{
					dis_theta=pre_theta-now_theta>0?pre_theta-now_theta:now_theta-pre_theta;
				}
				if(dis_xy>max_dis_xy)
					max_dis_xy=dis_xy;
				if(dis_theta>max_theta)
					max_theta=dis_theta;
					sum_dis_xy+=dis_xy;
					sum_theta+=dis_theta;
				if(dis_xy>5*deta_timestamp&&deta_timestamp!=0)
				{
					++dis_unusual_count;
					fputs(tempRead,fp_w1);	
					fputs("\n",fp_w1);	
				}
				if(dis_theta>10*deta_timestamp&&deta_timestamp!=0)
				{
					++theta_unusual_count;
					fputs(tempRead,fp_w2);	
					fputs("\n",fp_w2);	
				}
				pre_x=atoi(tempRow[1]);
				pre_y=atoi(tempRow[2]);
				pre_theta=atoi(tempRow[3]);
				pre_timestamp=atol(tempRow[7]);
			}
		}
    }
	fclose(fp_r);
	fclose(fp_w1); 
	fclose(fp_w2);	
	sprintf(result,",\"average_dis_xy\":\"%d\",\"average_theta\":\"%.3f\",\"max_dis_xy\":\"%d\",\"max_theta\":\"%d\",\"total_point_count\":\"%d\",\"dis_unusual_count\":\"%d\",\"theta_unusual_count\":\"%d\"",
	(int)(sum_dis_xy/(long)(total_point_count-21)),(float)((double)sum_theta/(double)(total_point_count-21)),max_dis_xy,max_theta,total_point_count-20,dis_unusual_count,theta_unusual_count);
	return 0;
}

//差值划线连接手画图
int CThread_web::draw_middle_snake_point(int x,int y,int pre_x,int pre_y)
{
	int deta_x=x-pre_x>0?x-pre_x:pre_x-x;
	int deta_y=y-pre_y>0?y-pre_y:pre_y-y;
	if(deta_x>1||deta_y>1)
	{
		snake_map_point[(pre_y+y)/2][(pre_x+x)/2].status=0x93;
		snake_map_point[(pre_y+y)/2][(pre_x+x)/2+1].status=0x93;
		snake_map_point[(pre_y+y)/2][(pre_x+x)/2+2].status=0x93;
		snake_map_point[(pre_y+y)/2+1][(pre_x+x)/2].status=0x93;
		snake_map_point[(pre_y+y)/2+1][(pre_x+x)/2+1].status=0x93;
		snake_map_point[(pre_y+y)/2+1][(pre_x+x)/2+2].status=0x93;
		snake_map_point[(pre_y+y)/2-1][(pre_x+x)/2].status=0x93;
		snake_map_point[(pre_y+y)/2-1][(pre_x+x)/2+1].status=0x93;
		snake_map_point[(pre_y+y)/2-1][(pre_x+x)/2+2].status=0x93;
		draw_middle_snake_point(x,y,(pre_x+x)/2,(pre_y+y)/2);
		draw_middle_snake_point((pre_x+x)/2,(pre_y+y)/2,pre_x,pre_y);
	}		
	return 0;
}

//生成手画边界地图
int CThread_web::make_hand_draw_map(char parameter[][64])
{
	int x=0;
	int y=0;
	int i=0;
	int j=0;
	if(search_parameter(parameter,"x"))
		x=atoi(search_parameter(parameter,"x"));
	else	
		printf("can not get x in fun make_hand_draw_map line=%d\n",__LINE__);
	if(search_parameter(parameter,"y"))
		y=atoi(search_parameter(parameter,"y"));
	else	
		printf("can not get y in fun make_hand_draw_map line=%d\n",__LINE__);
			
	if(search_parameter(parameter,"where"))
	{
		if(0==strcmp("head",search_parameter(parameter,"where")))
		{
			memset(snake_map_point,0,sizeof(snake_map_point));
			for(i=0;i<1024;++i)
			{
				for(j=0;j<11;++j)
				{
					snake_map_point[j][i].status=0xf3;
					snake_map_point[i][j].status=0xf3;
				}
				for(j=0;j<12;++j)
				{
					snake_map_point[1012+j][i].status=0xf3;
					snake_map_point[i][j+1012].status=0xf3;
				}
			}
			//printf("head	x=%d	y=%d\n",x,y);
			if(x>=11&&x<=1011&&y>=11&&y<=1011)
			{
				snake_map_point[y][x].status=0x93;
				snake_map_point[y][x+1].status=0x93;
				snake_map_point_pre_x=x;
				snake_map_point_pre_y=y;
			}
		}
		else if(0==strcmp("body",search_parameter(parameter,"where")))
		{
			//printf("body	x=%d	y=%d\n",x,y);
			//printf("body	px=%d	py=%d\n",snake_map_point_pre_x,snake_map_point_pre_y);
			snake_map_point[y][x].status=0x93;
			snake_map_point[y][x+1].status=0x93;
			snake_map_point[y][x+2].status=0x93;
			snake_map_point[y+1][x].status=0x93;
			snake_map_point[y+1][x+1].status=0x93;
			snake_map_point[y+1][x+2].status=0x93;
			snake_map_point[y-1][x].status=0x93;
			snake_map_point[y-1][x+1].status=0x93;
			snake_map_point[y-1][x+2].status=0x93;
			if(x-snake_map_point_pre_x>1||x-snake_map_point_pre_x<-1||y-snake_map_point_pre_y>1||y-snake_map_point_pre_y<-1)
			{
				draw_middle_snake_point(x,y,snake_map_point_pre_x,snake_map_point_pre_y);
			}
			snake_map_point_pre_x=x;
			snake_map_point_pre_y=y;
		}
		else if(0==strcmp("tail",search_parameter(parameter,"where")))
		{
			printf("make_hand_draw_map tail\n");
			FILE *fp = NULL;
		    fp = fopen(map_file_csv_path, "w");
			if(!fp)
			{
		        printf("write  file error in fun make_hand_draw_map\n");
		        strcat(total_data,"1"); 
		        return -1;
		    }
		    int i=0;
		    for(i=0;i<1024;++i)
				//指向数据的指针，每个数据的大小，数据个数，文件指针
		    	fwrite(&snake_map_point[i][0],sizeof(uint32_t),1024,fp);
		    fclose(fp);
		    strcat(total_data,"{\"status\":\"SAVE_OK\"}"); 
			return 0;	
		}
	}
	strcat(total_data,"1"); 
	return 0;	
}

//区分地图颜色
int CThread_web::decode_map_color(uint8_t state,char *color)
{
	if(0==(state&0x0F))
	{
		//未知区域	白色
		sprintf(color,"#FFFFFF");
	}
	else if(1==(state&0x0F))
	{
		//未清扫	青色
		sprintf(color,"#408080");
	}
	else if(2==(state&0x0F))
	{
		//已清扫	绿色
		sprintf(color,"#82d900");
	}
	else if(3==(state&0x0F))
	{
		//障碍		黑色//没有体现
		int depth=((state&0xF0)>>4);//黑色的深度
		//printf("depth=%d\n",depth);
		if(depth>=8)
			sprintf(color,"#000000");
		if(depth==7)
			sprintf(color,"#272727");
		if(depth==6)
			sprintf(color,"#3C3C3C");
		if(depth==5)
			sprintf(color,"#4F4F4F");
		if(depth==4)
			sprintf(color,"#5B5B5B");
		if(depth==3)
			sprintf(color,"#6C6C6C");
		if(depth==2)
			sprintf(color,"#7B7B7B");
		if(depth==1)
			sprintf(color,"#8E8E8E");
		if(depth==0)
			sprintf(color,"#8E8E8E");
	}
	else if(0x0E==(state&0x0F))
	{
		//线条	紫色
		sprintf(color,"#6C3365");
	}	
	return 0;
}


int CThread_web::str_to_json(MapPoints *tempRead,FILE *fp_w,int line_count)
{
	char unit_data[128]={0};
	uint8_t state=tempRead[0].status;
	char color[32]={0};
	int i=0;
	int begin=0;
	while(1)
	{
		++i;
		if(tempRead[i].status!=state||i==line_count)
		{
			decode_map_color(state,color);
			sprintf(unit_data,",{\"val\":\"%d$%s\"}",i-begin,color);
			fputs(unit_data,fp_w);
			if(i==line_count)
				break;
			begin=i;
			state=tempRead[i].status;
		}
	}
	return 0;
}


int CThread_web::decode_one_line_map_data(FILE* fp_r,FILE* fp_w)
{
	MapPoints tempRead[256]={0};
	int i=0;
    for(i=0;i<4;++i)
    {
    	fread(tempRead,sizeof(uint32_t),256,fp_r);
    	str_to_json(tempRead,fp_w,256);
    }
    return 0;
}

//获取地图文件
int CThread_web::get_map_file(char parameter[][64])
{	
	//判断有没有json文件
	make_json_dir();
	char unit_data[256]={0};
	FILE *fp_r= NULL;
	fp_r = fopen(map_file_csv_path,"r");
    if(!fp_r)
	{
        sprintf(unit_data,"{\"status\":\"EMPTY MAP\"}"); 
		strcat(total_data,unit_data);   
        return 0;
    }
    
    FILE *fp_w = NULL;
    fp_w = fopen("/tmp/data/json/map_file.json","w");
	if(!fp_w)
	{
        printf("write map_file.json file error\n");
        sprintf(unit_data,"{\"status\":\"err2\"}");
		strcat(total_data,unit_data);   
        return 0;
    }
    
    fputs("{\"Results\":[{\"val\":\"head\"}",fp_w);
    int i=0;
    for(i=0;i<1024;++i)
    {
    	decode_one_line_map_data(fp_r,fp_w);
	}
	sprintf(unit_data,"]}"); 
	fputs(unit_data,fp_w);	
	fclose(fp_r);
	fclose(fp_w);
	sprintf(unit_data,"{\"status\":\"ok\"}");
	strcat(total_data,unit_data); 
	return 0;
}

//小地图文件
int CThread_web::get_small_map_file(char parameter[][64])
{
	int theta_x=0;//theta_x<450
	int theta_y=0;//theta_y<450
	int offset=512;
	if(search_parameter(parameter,"theta_x"))
		theta_x=atoi(search_parameter(parameter,"theta_x"));
	else	
		printf("can not get theta_x in fun get_small_map_fil line=%d\n",__LINE__);
	if(search_parameter(parameter,"theta_y"))
		theta_y=atoi(search_parameter(parameter,"theta_y"));
	else	
		printf("can not get theta_y in fun get_small_map_fil line=%d\n",__LINE__);
	
	//获取json文件
	make_json_dir();
	char unit_data[256]={0};
	FILE *fp_r= NULL;
	//fp_r = fopen("/tmp/data/map_file.csv","r");
    fp_r = fopen(map_file_csv_path,"r");
    if(!fp_r)
	{
        sprintf(unit_data,"{\"status\":\"EMPTY MAP\"}"); 
		strcat(total_data,unit_data);   
        return 0;
    }
    
    FILE *fp_w = NULL;
    fp_w = fopen("/tmp/data/json/small_map_file.json","w");
	if(!fp_w)
	{
        printf("write map_file.json file error\n");
        sprintf(unit_data,"{\"status\":\"err2\"}");
		strcat(total_data,unit_data);   
        return 0;
    }
    
    fputs("{\"Results\":[{\"val\":\"head\"}",fp_w);
    int i=0;
    int j=0;
    MapPoints tempRead[256]={0};
    for(i=0;i<offset-50+theta_y;++i)
    {
    	for(j=0;j<4;++j)
		{
    		fread(tempRead,sizeof(uint32_t),256,fp_r);
    	}
    }
    
    int front_hundred_i=(offset-50+theta_x)/200;
    int front_ten_i=(offset-50+theta_x)%200;
    int back_hundred_i=(offset-51-theta_x)/200;
    int back_ten_i=(offset-51-theta_x)%200;
    for(i=0;i<101;++i)
    {
    	for(j=0;j<front_hundred_i;++j)
		{
    		fread(tempRead,sizeof(uint32_t),200,fp_r);
    	}
    	fread(tempRead,sizeof(uint32_t),front_ten_i,fp_r);
    	
    	fread(tempRead,sizeof(uint32_t),101,fp_r);
    	str_to_json(tempRead,fp_w,101);
    	
    	for(j=0;j<back_hundred_i;++j)
		{
    		fread(tempRead,sizeof(uint32_t),200,fp_r);
    	}
    	fread(tempRead,sizeof(uint32_t),back_ten_i,fp_r);
    	
	}
	sprintf(unit_data,"]}"); 
	fputs(unit_data,fp_w);	
	fclose(fp_r);
	fclose(fp_w);
	sprintf(unit_data,"{\"status\":\"ok\"}");
	strcat(total_data,unit_data); 
	return 0;	
}

//画细线
int CThread_web::draw_small_line(int x1,int y1,int x2,int y2,int x_offset,int y_offset)
{
	if(x1-x2>1||x1-x2<-1||y1-y2>1||y1-y2<-1)
	{
		snake_map_point[(y1+y2)/2+y_offset][(x1+x2)/2+x_offset].status=0x93;
		draw_small_line(((x1+x2)/2),((y1+y2)/2),x2,y2,x_offset,y_offset);
		draw_small_line(x1,y1,((x1+x2)/2),((y1+y2)/2),x_offset,y_offset);
	}
	return 0;
}


float CThread_web::get_diff(int x1,int y1,int x2,int y2,float mx,float my)
{
	float a=sqrt(((float)x1-mx)*((float)x1-mx)+((float)y1-my)*((float)y1-my));
	float b=sqrt((mx-(float)x2)*(mx-(float)x2)+(my-(float)y2)*(my-(float)y2));
	return a-b>0.0?a-b:b-a;
}


int CThread_web::draw_big_line_direction(int x1,int y1,int x2,int y2,int dir,int x_offset,int y_offset)
{
	if(dir==0)
		draw_small_line(x1-1,y1-1,x2-1,y2-1,x_offset,y_offset);
	if(dir==1)
		draw_small_line(x1,y1-1,x2,y2-1,x_offset,y_offset);
	if(dir==2)
		draw_small_line(x1+1,y1-1,x2+1,y2-1,x_offset,y_offset);
	if(dir==3)
		draw_small_line(x1+1,y1,x2+1,y2,x_offset,y_offset);
	if(dir==4)
		draw_small_line(x1+1,y1+1,x2+1,y2+1,x_offset,y_offset);
	if(dir==5)
		draw_small_line(x1,y1+1,x2,y2+1,x_offset,y_offset);
	if(dir==6)
		draw_small_line(x1-1,y1+1,x2-1,y2+1,x_offset,y_offset);
	if(dir==7)
		draw_small_line(x1-1,y1,x2-1,y2,x_offset,y_offset);
	return 0;	
}

//画粗线
int CThread_web::draw_big_line(int x1,int y1,int x2,int y2,int x_offset,int y_offset)
{
	float mx=(float)(x1+x2)/2.0;
	float my=(float)(y1+y2)/2.0;
	float dis_arr[8]={0};
	float flag_arr[8]={0};
	dis_arr[0]=get_diff(x1,y1,x2,y2,mx-1.0,my-1.0);
	dis_arr[1]=get_diff(x1,y1,x2,y2,mx,my-1.0);
	dis_arr[2]=get_diff(x1,y1,x2,y2,mx+1.0,my-1.0);
	dis_arr[3]=get_diff(x1,y1,x2,y2,mx+1.0,my);
	dis_arr[4]=get_diff(x1,y1,x2,y2,mx+1.0,my+1.0);
	dis_arr[5]=get_diff(x1,y1,x2,y2,mx,my+1.0);
	dis_arr[6]=get_diff(x1,y1,x2,y2,mx-1.0,my+1.0);
	dis_arr[7]=get_diff(x1,y1,x2,y2,mx-1.0,my);
	flag_arr[0]=get_diff(x1,y1,x2,y2,mx-1.0,my-1.0);
	flag_arr[1]=get_diff(x1,y1,x2,y2,mx,my-1.0);
	flag_arr[2]=get_diff(x1,y1,x2,y2,mx+1.0,my-1.0);
	flag_arr[3]=get_diff(x1,y1,x2,y2,mx+1.0,my);
	flag_arr[4]=get_diff(x1,y1,x2,y2,mx+1.0,my+1.0);
	flag_arr[5]=get_diff(x1,y1,x2,y2,mx,my+1.0);
	flag_arr[6]=get_diff(x1,y1,x2,y2,mx-1.0,my+1.0);
	flag_arr[7]=get_diff(x1,y1,x2,y2,mx-1.0,my);
	int f1=0;
	int f2=0;
	int hate_you=0;
	qsort(dis_arr,8,sizeof(float),NumDescSort);	
	int i=0;
	for(i=0;i<8;++i)
	{
		if(dis_arr[0]!=dis_arr[1])
		{
			if(flag_arr[i]==dis_arr[0])
			{
				f1=i;
			}
			if(flag_arr[i]==dis_arr[1])
			{
				f2=i;
			}
		}
		else
		{
			if(hate_you==0&&flag_arr[i]==dis_arr[0])
			{
				f1=i;
				hate_you=1;
			}
			if(hate_you==1&&flag_arr[i]==dis_arr[0])
			{
				f2=i;
			}
		}
	}
	draw_big_line_direction(x1,y1,x2,y2,f1,x_offset,y_offset);
	draw_big_line_direction(x1,y1,x2,y2,f2,x_offset,y_offset);	
	return 0;
}

//画矩形
int CThread_web::draw_fang(int x1,int y1,int x2,int y2,int x_offset,int y_offset)
{
	int i=0;
	for(i=0;i<x2-x1+5;++i)
	{
		snake_map_point[y1+2+y_offset][x1-2+i+x_offset].status=0x93;
		snake_map_point[y1+1+y_offset][x1-2+i+x_offset].status=0x93;
		snake_map_point[y1+y_offset][x1-2+i+x_offset].status=0x93;
		snake_map_point[y2-2+y_offset][x1-2+i+x_offset].status=0x93;
		snake_map_point[y2-1+y_offset][x1-2+i+x_offset].status=0x93;
		snake_map_point[y2+y_offset][x1-2+i+x_offset].status=0x93;
	}
	for(i=0;i<y1-y2+2;++i)
	{
		snake_map_point[y1-i+y_offset][x1-2+x_offset].status=0x93;
		snake_map_point[y1-i+y_offset][x1-1+x_offset].status=0x93;
		snake_map_point[y1-i+y_offset][x1+x_offset].status=0x93;
		snake_map_point[y1-i+y_offset][x2+2+x_offset].status=0x93;
		snake_map_point[y1-i+y_offset][x2+1+x_offset].status=0x93;
		snake_map_point[y1-i+y_offset][x2+x_offset].status=0x93;
		
	}
	return 0;	
}

//保存地图(手画地图)
int CThread_web::save_map_file(char parameter[][64])
{
	char unit_data[256]={0};
	int fun_code=0,x1=0,y1=0,x2=0,y2=0,x_offset=0,y_offset=0;
	if(search_parameter(parameter,"fun_code"))
		fun_code=atoi(search_parameter(parameter,"fun_code"));
	else	
		printf("can not get fun_code in fun save_map_file line=%d\n",__LINE__);
	
	if(search_parameter(parameter,"x1"))
		x1=atoi(search_parameter(parameter,"x1"));
	else	
		printf("can not get x1 in fun save_map_file line=%d\n",__LINE__);
	
	if(search_parameter(parameter,"y1"))
		y1=atoi(search_parameter(parameter,"y1"));
	else	
		printf("can not get y1 in fun save_map_file line=%d\n",__LINE__);
	
	if(search_parameter(parameter,"x2"))
		x2=atoi(search_parameter(parameter,"x2"));
	else	
		printf("can not get x2 in fun save_map_file line=%d\n",__LINE__);
	
	if(search_parameter(parameter,"y2"))
		y2=atoi(search_parameter(parameter,"y2"));
	else	
		printf("can not get y2 in fun save_map_file line=%d\n",__LINE__);
	
	if(search_parameter(parameter,"x_offset"))
		x_offset=atoi(search_parameter(parameter,"x_offset"));
	else	
		printf("can not get x_offset in fun save_map_file line=%d\n",__LINE__);
	
	if(search_parameter(parameter,"y_offset"))
		y_offset=atoi(search_parameter(parameter,"y_offset"));
	else	
		printf("can not get y_offset in fun save_map_file line=%d\n",__LINE__);
	
	//printf("x1=%d y1=%d x2=%d y2=%d\n",x1,y1,x2,y2);
	FILE *fp= NULL;
	fp = fopen(map_file_csv_path,"r");
    if(!fp)
	{
        sprintf(unit_data,"{\"status\":\"EMPTY MAP\"}"); 
		strcat(total_data,unit_data);   
        return 0;
    }
    memset(snake_map_point,0,sizeof(snake_map_point));
    int i=0;
    for(i=0;i<1024;++i)
    	fread(&snake_map_point[i][0],sizeof(uint32_t),1024,fp);
    fclose(fp);
    fp=NULL;
    
    //画点
	if(fun_code==1)
	{
		snake_map_point[y1+y_offset][x1+x_offset].status=0x93;
	}
	//画细线
	if(fun_code==2)
	{
		snake_map_point[y1+y_offset][x1+x_offset].status=0x93;
		snake_map_point[y2+y_offset][x2+x_offset].status=0x93;
		draw_small_line(x1,y1,x2,y2,x_offset,y_offset);
	}
	//画粗线
	if(fun_code==3)
	{
		snake_map_point[y1+y_offset][x1+x_offset].status=0x93;
		snake_map_point[y2+y_offset][x2+x_offset].status=0x93;
		draw_small_line(x1,y1,x2,y2,x_offset,y_offset);
		draw_big_line(x1,y1,x2,y2,x_offset,y_offset);	
			    			
	}
	//画矩形
	if(fun_code==4)
	{
		snake_map_point[y1+y_offset][x1+x_offset].status=0x93;
		snake_map_point[y2+y_offset][x2+x_offset].status=0x93;
		draw_fang(x1,y1,x2,y2,x_offset,y_offset);
			    			
	}
	
	//清除点
	if(fun_code==5)
	{
		snake_map_point[y1+y_offset][x1+x_offset].status=0x00;
	}
	
	//清除矩形
	if(fun_code==6)
	{
		int j=0;
		for(i=0;i<x2-x1+1;++i)
		{
			for(j=0;j<y1-y2+1;++j)
			{
				snake_map_point[y1+y_offset-j][x1+x_offset+i].status=0x00;
			}	
		}
	}
	
	//保存
	fp = fopen(map_file_csv_path, "w");
	if(!fp)
	{
        sprintf(unit_data,"{\"status\":\"SAVE ERR\"}"); 
		strcat(total_data,unit_data);   
        return 0;
    }
    for(i=0;i<1024;++i)
    	fwrite(&snake_map_point[i][0],sizeof(uint32_t),1024,fp);
    fclose(fp);
	sprintf(unit_data,"{\"status\":\"ok\"}");
	strcat(total_data,unit_data);
	return 0;	
}

//判断有没有json目录，如果没有，建立目录,then delete all json file before
int CThread_web::make_json_dir()
{
	char path[64]={0};
	DIR *pDir;
	struct dirent  *ent;
    sprintf(path,"/tmp/data/json");
	pDir=opendir(path);
    if(pDir  == NULL)
	{
    	DIR *pDir_tmp=opendir("/tmp/data");
		if(pDir_tmp)
		{
			mkdir(path,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
			closedir(pDir_tmp);
		}
		else
		{	
			//文件地址及权限
			mkdir("/tmp/data",S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH); 
			mkdir(path,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);  
		}
    }
	else
	{
		while((ent=readdir(pDir)))
		{
	        if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
				continue; 
	        if(ent->d_type ==8)
			{
	        	char rm_cmmond[32]={0};
	        	sprintf(rm_cmmond,"/tmp/data/json/%s",ent->d_name);
	        	remove(rm_cmmond);
	        }
	    }
   		closedir(pDir);
    }
	return 0;	
}


//画点
int CThread_web::draw_one_point(int x1,int y1)
{
	int i=0;
	int j=0;
	for(i=0;i<5;++i)
	{
		for(j=0;j<5;++j)
		{
			(snake_map_point[y1+j-2][x1+i-2].status&=0xF0)|=0x0E;
		}
	}
	return 0;
}

//画出障碍的边界
int CThread_web::draw_route_in_snake(char parameter[][64])
{
	char unit_data[256]={0};
	int PA_x=0;
	int PA_y=0;
	int PB_x=0;
	int PB_y=0;
	if(search_parameter(parameter,"PA_x"))
		PA_x=atoi(search_parameter(parameter,"PA_x"));
	else	
		printf("can not get PA_x in fun draw_route_in_snake line=%d\n",__LINE__);
	if(search_parameter(parameter,"PA_y"))
		PA_y=atoi(search_parameter(parameter,"PA_y"));
	else	
		printf("can not get PA_y in fun draw_route_in_snake line=%d\n",__LINE__);
	if(search_parameter(parameter,"PB_x"))
		PB_x=atoi(search_parameter(parameter,"PB_x"));
	else	
		printf("can not get PB_x in fun draw_route_in_snake line=%d\n",__LINE__);
	if(search_parameter(parameter,"PB_y"))
		PB_y=atoi(search_parameter(parameter,"PB_y"));
	else	
		printf("can not get PB_y in fun draw_route_in_snake line=%d\n",__LINE__);
	
	printf("target:Ax=%d	Ay=%d	Bx=%d	By=%d\n",PA_x,PA_y,PB_x,PB_y);
	//加载地图
	FILE *fp= NULL;
	fp = fopen(map_file_csv_path,"r");
    if(!fp)
	{
        sprintf(unit_data,"{\"status\":\"EMPTY MAP\"}"); 
		strcat(total_data,unit_data);   
        return 0;
    }
    memset(snake_map_point,0,sizeof(snake_map_point));
	
    int i=0;
    for(i=0;i<1024;++i)
    	fread(&snake_map_point[i][0],sizeof(uint32_t),1024,fp);
    fclose(fp);
    fp=NULL;
	
	
	
	AB_point store_line[2000];
	AB_point first_point;
	memset(&first_point,0,sizeof(AB_point));
	memset(store_line,0,sizeof(store_line));
	
	//画直线AB
	Draw_line *draw_step1=new Draw_line(snake_map_point);
	draw_step1->run_this_line(PA_x,PA_y,PB_x,PB_y,1);
	draw_step1->get_first_P(&first_point);
	
	
	
	AB_point store_line_left[2000];
	AB_point store_line_right[2000];
	AB_point store_line_circle[2000];
	memset(store_line_circle,0,sizeof(store_line_circle));
	memset(store_line_left,0,sizeof(store_line_left));
	memset(store_line_right,0,sizeof(store_line_right));

	Draw_direction *draw_step2=new Draw_direction(snake_map_point);
	draw_step2->run_this_direction(first_point.x,first_point.y,PB_x,PB_y);
//	int store_len_circle=draw_step2->get_line_circle(store_line_circle);
//	for(i=0;i<store_len_circle;++i){
//		(snake_map_point[store_line_circle[i].y][store_line_circle[i].x].status&=0xF0)|=0x0E;
//	}


	int store_len_left=draw_step2->get_line_left(store_line_left);
	for(i=0;i<store_len_left;++i)
	{
		(snake_map_point[store_line_left[i].y][store_line_left[i].x].status&=0xF0)|=0x0E;
	}
	
	int store_len_right=draw_step2->get_line_right(store_line_right);
	printf("store_len_right=%d  left=%d\n",store_len_right,store_len_left);
	
	for(i=0;i<store_len_right;++i)
	{
		(snake_map_point[store_line_right[i].y][store_line_right[i].x].status&=0xF0)|=0x0E;
	}
	
	//画A和B
	draw_one_point(PA_x,PA_y);
	draw_one_point(PB_x,PB_y);
	//int store_len=draw_step1.get_line(store_line);
	int store_len=draw_step1->get_first_line(store_line);
	for(i=0;i<store_len;++i)
	{
		//printf("store:x=%d y=%d\n",store_line[i].y,store_line[i].x);
		(snake_map_point[store_line[i].y][store_line[i].x].status&=0xF0)|=0x0E;
	}
	draw_one_point(first_point.x,first_point.y);
	
	
	
	//printf("first x=%d y=%d dis=%d\n",first_point.x,first_point.y,first_point.dis);
	
	fp = fopen(map_file_csv_path, "w");
	if(!fp)
	{
        printf("write  file error in fun draw_route_in_snake\n");
        strcat(total_data,"1"); 
        return -1;
    }
    for(i=0;i<1024;++i)
    	fwrite(&snake_map_point[i][0],sizeof(uint32_t),1024,fp);
    fclose(fp);
	sprintf(unit_data,"{\"status\":\"R=%d L=%d %s\"}",store_len_right,store_len_left,store_len_right<store_len_left?"RIGHT":"LEFT");
	//sprintf(unit_data,"{\"status\":\"R=%d L=%d %s\"}",1,1,1<1?"RIGHT":"LEFT");
	strcat(total_data,unit_data);
	//strcat(total_data,"1");
	delete(draw_step1);
	delete(draw_step2);
	return 0;	
}


//初始化调试地图(为加速做准备)
int CThread_web::init_debug_map(char parameter[][64])
{
	make_json_dir();
	char tempRow[12][32];
	char tempRead[128];
	char unit_data[512];
	memset(unit_data,0,sizeof(unit_data));
	memset(tempRead,0,sizeof(tempRead));
	memset(tempRow,0,sizeof(tempRow));
	
	//文件拆分
	int file_id=0;
    char file_path[32]={0};
	FILE *fp_map= NULL;
	fp_map = fopen("/tmp/data/debug_test.csv","r");
    if(!fp_map)
	{
        printf("read debug_test file error line = %d\n",__LINE__);   
        sprintf(unit_data,"{\"status\":\"err1\"}");
		strcat(total_data,unit_data);   
        return 0;
    }
    
    FILE *fp = NULL;
    sprintf(file_path,"/tmp/data/json/debug%d.json",file_id);
	fp = fopen(file_path, "w");
	if(!fp)
	{
        printf("write debug.json file error line=%d\n",__LINE__);   
        sprintf(unit_data,"{\"status\":\"err2\"}");
		strcat(total_data,unit_data);   
        return 0;
    }
    char val[128]={0};
    int total_point_count=0;		//地图点的总个数
    int effective_point_count=0;	//有效的地图点个数，比如有40000个点，有效点可能只有3000个
    int pre_x=0;
	int pre_y=0;
	int pre_status=0;
	int now_x=0;
	int now_y=0;
	int now_status=0;
	
	while(fgets(tempRead, sizeof(tempRead), fp_map)) 
	{
        while(tempRead[strlen(tempRead) - 1] == '\n' || tempRead[strlen(tempRead) - 1] == ' ') 
        	tempRead[strlen(tempRead) - 1] = '\0';
        if(tempRead[0] == '\0') 
		{
            continue;
        }
        ++total_point_count;
        sscanf(tempRead, "%s%s%s%s%s%s%s%s%s%s%s%s",tempRow[0],tempRow[1],tempRow[2],tempRow[3],
		tempRow[4],tempRow[5],tempRow[6],tempRow[7],tempRow[8],tempRow[9],tempRow[10],tempRow[11]);
        
		if(total_point_count==1)
		{
			++effective_point_count;
			sprintf(val,"{\"Results\":[{\"val\":\"%d$%d$%d\"}",atoi(tempRow[1]),atoi(tempRow[2]),atoi(tempRow[4]));
			fputs(val,fp);
			pre_x=atoi(tempRow[1]);
			pre_y=atoi(tempRow[2]);
			pre_status=atoi(tempRow[4]);
		}
		else
		{
			if(0==effective_point_count%1000)
			{
				++effective_point_count;
				sprintf(val,",{\"val\":\"%d$%d$%d\"}]}",atoi(tempRow[1]),atoi(tempRow[2]),atoi(tempRow[4]));
				fputs(val,fp);	
				fclose(fp);
				fp=NULL;
				sprintf(file_path,"/tmp/data/json/debug%d.json",++file_id);
				fp = fopen(file_path, "w");
				if(!fp)
				{
			        printf("write debug.json file error\n");   
			        sprintf(unit_data,"{\"status\":\"err3\"}");
					strcat(total_data,unit_data);   
			        return 0;
			    }	
				sprintf(val,"{\"Results\":[{\"val\":\"%d$%d$%d\"}",atoi(tempRow[1]),atoi(tempRow[2]),atoi(tempRow[4]));
				fputs(val,fp);
				pre_x=atoi(tempRow[1]);
				pre_y=atoi(tempRow[2]);
				pre_status=atoi(tempRow[4]);
			}
			else
			{
				now_x=atoi(tempRow[1]);
				now_y=atoi(tempRow[2]);
				now_status=atoi(tempRow[4]);
				if(now_x==pre_x&&now_y==pre_y&&pre_status==now_status)
				{
					;
				}
				else
				{
					++effective_point_count;
					sprintf(val,",{\"val\":\"%d$%d$%d\"}",atoi(tempRow[1]),atoi(tempRow[2]),atoi(tempRow[4]));
					fputs(val,fp);	
					pre_x=atoi(tempRow[1]);
					pre_y=atoi(tempRow[2]);
					pre_status=atoi(tempRow[4]);	
				}
			}
		}
    }
    if(0==total_point_count)
	{
    	sprintf(val,"{\"Results\":[{\"val\":\"null\"}]}");
    	fputs(val,fp);	
    	fclose(fp);
    	strcat(total_data,"{\"status\":\"null\"}");
    	return 0; 
	}
	else
	{
    	sprintf(val,",{\"val\":\"%d$%d$%d\"}]}",atoi(tempRow[1]),atoi(tempRow[2]),atoi(tempRow[4]));
	}
	fputs(val,fp);	
    fclose(fp);
    int file_num=effective_point_count/1000+1; 
    long fp_map_offset = ftell(fp_map);
    fclose(fp_map);
    
    sprintf(unit_data,"{\"status\":\"ok\",\"file_num\":\"%d\",\"fp_map_offset\":\"%ld\"}",file_num,fp_map_offset);
	strcat(total_data,unit_data); 
	return 0;	
    
}



//初始化地图文件(为加速做准备)
int CThread_web::init_map_file(char parameter[][64])
{
	make_json_dir();
    char tempRow[8][32];
	char tempRead[128];
	char unit_data[512];
	memset(unit_data,0,sizeof(unit_data));
	memset(tempRead,0,sizeof(tempRead));
	memset(tempRow,0,sizeof(tempRow));
	
    //文件拆分
    int file_id=0;
    char val[128]={0};
    char file_path[32]={0};
    FILE *fp_map = NULL;
	fp_map = fopen("/tmp/data/map_points.csv", "r");
    if(!fp_map)
	{
        printf("read map_points file error\n");   
        sprintf(unit_data,"{\"status\":\"err1\"}");
		strcat(total_data,unit_data);   
        return 0;
    }
    
    FILE *fp = NULL;
    sprintf(file_path,"/tmp/data/json/map%d.json",file_id);
	fp = fopen(file_path, "w");
	if(!fp)
	{
        printf("write map.json file error\n");   
        sprintf(unit_data,"{\"status\":\"err2\"}");
		strcat(total_data,unit_data);   
        return 0;
    }
    int total_point_count=0;		//地图点的总个数
    int effective_point_count=0;	//有效的地图点个数，比如有40000个点，有效点可能只有3000个
    int pre_x=0;
	int pre_y=0;
	int now_x=0;
	int now_y=0;
	int dis_xy=0;
    
	while(fgets(tempRead, sizeof(tempRead), fp_map)) 
	{
        while(tempRead[strlen(tempRead) - 1] == '\n' || tempRead[strlen(tempRead) - 1] == ' ') 
        	tempRead[strlen(tempRead) - 1] = '\0';
        if(tempRead[0] == '\0') 
		{
            continue;
        }
        ++total_point_count;
        sscanf(tempRead, "%s%s%s%s%s%s",tempRow[0],tempRow[1],tempRow[2],tempRow[3],tempRow[4],tempRow[5]);
        if(0==strcmp(tempRow[4],"00000000000000000"))
		{
        	strcpy(tempRow[4],"n");	
        }
		if(total_point_count==1)
		{
			++effective_point_count;
			sprintf(val,"{\"Results\":[{\"val\":\"%d$%d$%d$%s$%d\"}",atoi(tempRow[1]),atoi(tempRow[2]),atoi(tempRow[3]),tempRow[4],atoi(tempRow[5]));
			fputs(val,fp);
			pre_x=atoi(tempRow[1]);
			pre_y=atoi(tempRow[2]);
		}
		else
		{
			if(0==effective_point_count%1000)
			{
				++effective_point_count;
				sprintf(val,",{\"val\":\"%d$%d$%d$%s$%d\"}]}",atoi(tempRow[1]),atoi(tempRow[2]),atoi(tempRow[3]),tempRow[4],atoi(tempRow[5]));
				fputs(val,fp);	
				fclose(fp);
				fp=NULL;
				sprintf(file_path,"/tmp/data/json/map%d.json",++file_id);
				fp = fopen(file_path, "w");
				if(!fp)
				{
			        printf("write map.json file error\n");   
			        sprintf(unit_data,"{\"status\":\"err3\"}");
					strcat(total_data,unit_data);   
			        return 0;
			    }	
				sprintf(val,"{\"Results\":[{\"val\":\"%d$%d$%d$%s$%d\"}",atoi(tempRow[1]),atoi(tempRow[2]),atoi(tempRow[3]),tempRow[4],atoi(tempRow[5]));
				fputs(val,fp);
				pre_x=atoi(tempRow[1]);
				pre_y=atoi(tempRow[2]);
			}
			else
			{
				now_x=atoi(tempRow[1]);
				now_y=atoi(tempRow[2]);
				dis_xy=sqrt((now_x-pre_x)*(now_x-pre_x)+(now_y-pre_y)*(now_y-pre_y));
				if(dis_xy>1)
				{
					++effective_point_count;
					sprintf(val,",{\"val\":\"%d$%d$%d$%s$%d\"}",atoi(tempRow[1]),atoi(tempRow[2]),atoi(tempRow[3]),tempRow[4],atoi(tempRow[5]));
					fputs(val,fp);	
					pre_x=atoi(tempRow[1]);
					pre_y=atoi(tempRow[2]);	
				}
			}
		}
    }
    if(0==total_point_count)
	{
    	sprintf(val,"{\"Results\":[{\"val\":\"%d$%d$%d$%s$%d\"}]}",atoi(tempRow[1]),atoi(tempRow[2]),atoi(tempRow[3]),tempRow[4],atoi(tempRow[5]));
	}
	else
	{
    	sprintf(val,",{\"val\":\"%d$%d$%d$%s$%d\"}]}",atoi(tempRow[1]),atoi(tempRow[2]),atoi(tempRow[3]),tempRow[4],atoi(tempRow[5]));
	}
	fputs(val,fp);	
    fclose(fp);
    int file_num=effective_point_count/1000+1; 
    long fp_map_offset = ftell(fp_map);
    fclose(fp_map);
    char get_result[256]={0};
    if(-1==caculate(get_result))
	{
    	sprintf(unit_data,"{\"status\":\"err4\",\"file_num\":\"%d\",\"fp_map_offset\":\"%ld\"}",file_num,fp_map_offset);
    }
	else
	{
    	sprintf(unit_data,"{\"status\":\"ok\",\"file_num\":\"%d\",\"fp_map_offset\":\"%ld\"%s}",file_num,fp_map_offset,get_result);
	}
    strcat(total_data,unit_data); 
    printf("total_point_count=%d\n",total_point_count); 
	return 0;	
}

//控制creater行走：前后左右停
int CThread_web::ctrl_cleanner_robot(char parameter[][64])
{
	if(search_parameter(parameter,"ctrl_mode"))
	{
		ctrl_mode = atoi(search_parameter(parameter,"ctrl_mode"));
	}
	else
	{
		printf("can not get parameter ctrl_mode in fun ctrl_cleanner_robot line=%d\n",__LINE__);
	}
	
	if(search_parameter(parameter,"which_robot"))
	{
		which_robot = atoi(search_parameter(parameter,"which_robot"));
	}
	else
	{
		printf("can not get parameter which_robot in fun ctrl_cleanner_robot line=%d\n",__LINE__);
	}
	
	if(search_parameter(parameter,"line_speed"))
	{
		line_speed = atoi(search_parameter(parameter,"line_speed"));
	}
	else
	{
		printf("can not get parameter Line in fun ctrl_cleanner_robot line=%d\n",__LINE__);
	}
	 
	if(ctrl_mode==10)
	{
		if(search_parameter(parameter,"goto_x"))
		{
			goto_x = atoi(search_parameter(parameter,"goto_x"));
		}
		else
		{
			printf("can not get parameter goto_x in fun ctrl_cleanner_robot line=%d\n",__LINE__);
		}
		if(search_parameter(parameter,"goto_y"))
		{
			goto_y = atoi(search_parameter(parameter,"goto_y"));
		}
		else
		{
			printf("can not get parameter goto_y in fun ctrl_cleanner_robot line=%d\n",__LINE__);
		}
		printf("ctrl_mode = %d	line_speed = %d	goto_x = %d	goto_y = %d	which_robot=%d\n",ctrl_mode,line_speed,goto_x,goto_y,which_robot);
		web_ctrl_info.goto_x=goto_x;
		web_ctrl_info.goto_y=goto_y;
		
	}
	else
	{
		if(search_parameter(parameter,"radius"))
		{
			radius = atoi(search_parameter(parameter,"radius"));
		}
		else
		{
			printf("can not get parameter angv in fun ctrl_cleanner_robot line=%d\n",__LINE__);
		}
		printf("ctrl_mode = %d	line_speed = %d	radius = %d	which_robot=%d\n",ctrl_mode,line_speed,radius,which_robot);
		web_ctrl_info.radius=radius;
	}
	
	web_ctrl_info.ctrl_mode=ctrl_mode;	
	web_ctrl_info.which_robot=which_robot;	
	web_ctrl_info.line_speed=line_speed;
	strcat(total_data,"{\"status\":\"ok\"}"); 	
	return 0;
}

//显示slam栅格图
int CThread_web::add_slam_point(char parameter[][64])
{
	
	int i=0;
	int Hz=20;
	char unit_data[1024];
	char tempRow[12][32];
	char tempRead[128];
	char val[128]={0};
	long fp_offset_pre=0;
	long fp_offset_now=0;
	long fp_map_offset=0;
	memset(tempRead,0,sizeof(tempRead));
	memset(tempRow,0,sizeof(tempRow));
	memset(unit_data,0,sizeof(unit_data)); 
	if(search_parameter(parameter,"Hz"))
		Hz=atoi(search_parameter(parameter,"Hz"));
	else	
		printf("can not get Hz in fun add_slam_point line=%d\n",__LINE__);
	if(search_parameter(parameter,"fp_map_offset"))
		fp_map_offset=atol(search_parameter(parameter,"fp_map_offset"));
	else	
		printf("can not get fp_map_offset in fun add_slam_point line=%d\n",__LINE__);
	int action=0;
	int mode=0;
	int step=0;
	FILE *fp = NULL;
	fp = fopen("/tmp/data/map_points.csv", "r");
   	if(!fp)
	{
		printf("read  map_points error, line = %d\n",__LINE__);
        sprintf(unit_data,"{\"status\":\"%d\"}",-2);
        strcat(total_data,unit_data);   
        return -1;
    }
	//把fp有关的文件位置指针放到一个指定位置
	//将指针从文件末向前移动len(fp_map_offset)个位置
    fseek(fp,fp_map_offset,SEEK_SET);
	while(fgets(tempRead, sizeof(tempRead), fp)) 
	{
        while(tempRead[strlen(tempRead) - 1] == '\n' || tempRead[strlen(tempRead) - 1] == ' ') 
        	tempRead[strlen(tempRead) - 1] = '\0';//
        if(tempRead[0] == '\0') 
		{
            continue;
        }
		++i;
		//															No		x			y			theta	block		status		action		mode		step
 		sscanf(tempRead, "%s	%s	%s	%s	%s %s	%s	%s	%s",tempRow[0],tempRow[1],tempRow[2],tempRow[3],tempRow[4], tempRow[5],tempRow[6],tempRow[7], tempRow[8]);
        if(0==strcmp(tempRow[4],"00000000000000000"))
		{
        	strcpy(tempRow[4],"n");	
        }
		
 		if(i==1)
		{
			action=atoi(tempRow[6]);
			mode=atoi(tempRow[7]);
			step=atoi(tempRow[8]);
			sprintf(val,"%d$%d$%d$%s$%s",atoi(tempRow[1]),atoi(tempRow[2]),atoi(tempRow[3]),tempRow[4],tempRow[5]);
			sprintf(unit_data,"{\"Results\":[{\"val\":\"%s\"}",val);
			strcat(total_data,unit_data);
		}
		else if(i<=Hz)
		{
			sprintf(val,"%d$%d$%d$%s$%s",atoi(tempRow[1]),atoi(tempRow[2]),atoi(tempRow[3]),tempRow[4],tempRow[5]);
			sprintf(unit_data,",{\"val\":\"%s\"}",val);
			strcat(total_data,unit_data);
			fp_offset_pre=fp_offset_now;
			fp_offset_now= ftell(fp);
		}
		else
		{
			break;	
		}
		
	}
	fclose(fp);
	if(i!=1&&i!=2&&fp_offset_pre!=0)
	{
		fp_map_offset=fp_offset_pre;
	}
	else
	{
		sprintf(unit_data,"{\"status\":\"%d\"}",-3);
		strcat(total_data,unit_data);
		return 0;
	}
		
    if(total_data[0]!='{')
	{
		// printf("$$$$$$$$$$$	empty	$$$$$$$$$$$$$$$$$\n");
		sprintf(unit_data,"{\"status\":\"%d\"}",-2);
		strcat(total_data,unit_data);
	}
	else
	{
		sprintf(unit_data,"],\"fp_map_offset\":\"%ld\",\"action\":\"%d\",\"mode\":\"%d\",\"step\":\"%d\"}",fp_map_offset,action,mode,step);
		strcat(total_data,unit_data);
	}
	return 0;	
}

//显示调试地图
int CThread_web::add_debug_point(char parameter[][64])
{
	int i=0;
	int Hz=20;
	char unit_data[1024];
	char tempRow[16][32];
	char tempRead[128];
	char val[128]={0};
	long fp_offset_pre=0;
	long fp_offset_now=0;
	long fp_map_offset=0;
	memset(tempRead,0,sizeof(tempRead));
	memset(tempRow,0,sizeof(tempRow));
	memset(unit_data,0,sizeof(unit_data)); 
	if(search_parameter(parameter,"Hz"))
		Hz=atoi(search_parameter(parameter,"Hz"));
	else	
		printf("can not get Hz in fun add_debug_point line=%d\n",__LINE__);
	if(search_parameter(parameter,"fp_map_offset"))
		fp_map_offset=atol(search_parameter(parameter,"fp_map_offset"));
	else	
		printf("can not get fp_map_offset in fun add_debug_point line=%d\n",__LINE__);
	
	FILE *fp = NULL;
	fp = fopen("/tmp/data/debug_test.csv", "r");
   	if(!fp)
	{
		printf("read  debug_test error, line = %d\n",__LINE__);
        sprintf(unit_data,"{\"status\":\"%d\"}",-2);
        strcat(total_data,unit_data);   
        return -1;
    }
    fseek(fp,fp_map_offset,SEEK_SET);
	while(fgets(tempRead, sizeof(tempRead), fp)) 
	{
        while(tempRead[strlen(tempRead) - 1] == '\n' || tempRead[strlen(tempRead) - 1] == ' ') 
        	tempRead[strlen(tempRead) - 1] = '\0';
        if(tempRead[0] == '\0') 
		{
            continue;
        }
		++i;
		//1   2   3    4         5        6       7      8     9    10   11    12
		//index	x	y	theta	status	bump+drop wall01 wall02 ir01 ir02 ir03 stamp
 		sscanf(tempRead, "%s%s%s%s%s%s%s%s%s%s%s%s",tempRow[0],tempRow[1],tempRow[2],tempRow[3],tempRow[4],tempRow[5],tempRow[6],tempRow[7],tempRow[8],tempRow[9],tempRow[10],tempRow[11]);
        
		
 		if(i==1)
		{
			sprintf(val,"%d$%d$%.1f$%d",atoi(tempRow[1]),atoi(tempRow[2]),atof(tempRow[3]),atoi(tempRow[4]));
			sprintf(unit_data,"{\"Results\":[{\"val\":\"%s\",\"bus\":\"%s$%d$%d$%d$%d$%d\"}",val,tempRow[5],atoi(tempRow[6]),atoi(tempRow[7]),atoi(tempRow[8]),atoi(tempRow[9]),atoi(tempRow[10]));
			strcat(total_data,unit_data);
		}
		else if(i<=Hz)
		{
			sprintf(val,"%d$%d$%.1f$%d",atoi(tempRow[1]),atoi(tempRow[2]),atof(tempRow[3]),atoi(tempRow[4]));
			if(i%5==1)
			{
				sprintf(unit_data,",{\"val\":\"%s\",\"bus\":\"%s$%d$%d$%d$%d$%d\"}",val,tempRow[5],atoi(tempRow[6]),atoi(tempRow[7]),atoi(tempRow[8]),atoi(tempRow[9]),atoi(tempRow[10]));
			}
			else
			{
				sprintf(unit_data,",{\"val\":\"%s\",\"bus\":\"null\"}",val);
			}
			strcat(total_data,unit_data);
			fp_offset_pre=fp_offset_now;
			fp_offset_now= ftell(fp);
		}
		else
		{
			break;	
		}
		
	}
	fclose(fp);
	if(i!=1&&i!=2&&fp_offset_pre!=0)
	{
		fp_map_offset=fp_offset_pre;
	}
	else
	{
		sprintf(unit_data,"{\"status\":\"%d\"}",-3);
		strcat(total_data,unit_data);
		return 0;
	}
		
    if(total_data[0]!='{')
	{
		// printf("$$$$$$$$$$$	empty	$$$$$$$$$$$$$$$$$\n");
		sprintf(unit_data,"{\"status\":\"%d\"}",-2);
		strcat(total_data,unit_data);
	}
	else
	{
		sprintf(unit_data,"],\"fp_map_offset\":\"%ld\"}",fp_map_offset);
		strcat(total_data,unit_data);
	}
	return 0;	
}

int CThread_web::show_door(char parameter[][64])
{
	int i=0;
	int Hz=20;
	char unit_data[1024];
	char tempRow[8][32];
	char tempRead[128];
	char val[128]={0};
	long fp_item_offset=0;
	memset(tempRead,0,sizeof(tempRead));
	memset(tempRow,0,sizeof(tempRow));
	memset(unit_data,0,sizeof(unit_data)); 
	if(search_parameter(parameter,"fp_item_offset"))
		fp_item_offset=atol(search_parameter(parameter,"fp_item_offset"));
	else	
		printf("can not get fp_item_offset in fun add_slam_point line=%d\n",__LINE__);
	FILE *fp = NULL;
	fp = fopen("/tmp/data/door.csv", "r");
   	if(!fp){
		printf("read   door.csv error, line = %d\n",__LINE__);
        sprintf(unit_data,"{\"status\":\"%d\"}",-2);
        strcat(total_data,unit_data);   
        return -1;
    }
    fseek(fp,fp_item_offset,SEEK_SET);
	while(fgets(tempRead, sizeof(tempRead), fp)) {
        while(tempRead[strlen(tempRead) - 1] == '\n' || tempRead[strlen(tempRead) - 1] == ' ') 
        	tempRead[strlen(tempRead) - 1] = '\0';
        if(tempRead[0] == '\0') {
            continue;
        }
        ++i;
        								//id		x			y		theta		item
        sscanf(tempRead, "%s%s%s%s%s",tempRow[0],tempRow[1],tempRow[2],tempRow[3],tempRow[4]);
        if(i==1)
		{
			sprintf(val,"%d$%d$%d$%d",atoi(tempRow[1]),atoi(tempRow[2]),atoi(tempRow[3]),atoi(tempRow[4]));
			sprintf(unit_data,"{\"Results\":[{\"val\":\"%s\"}",val);
			strcat(total_data,unit_data);
			fp_item_offset= ftell(fp);
		}
		else if(i<=Hz)
		{
			sprintf(val,"%d$%d$%d$%d",atoi(tempRow[1]),atoi(tempRow[2]),atoi(tempRow[3]),atoi(tempRow[4]));
			sprintf(unit_data,",{\"val\":\"%s\"}",val);
			strcat(total_data,unit_data);
			fp_item_offset= ftell(fp);
		}else{
			break;	
		}
		
	}
	fclose(fp);
	if(i==0){
		sprintf(unit_data,"{\"status\":\"%d\"}",-3);
		strcat(total_data,unit_data);
		return 0;
	}
	if(total_data[0]!='{')
	{
		// printf("$$$$$$$$$$$	empty	$$$$$$$$$$$$$$$$$\n");
		sprintf(unit_data,"{\"status\":\"%d\"}",-2);
		strcat(total_data,unit_data);
	}
	else
	{
		sprintf(unit_data,"],\"status\":\"ok\",\"fp_item_offset\":\"%ld\"}",fp_item_offset);
		strcat(total_data,unit_data);
	}
	return 0;	
}


int CThread_web::show_red_led(char parameter[][64])
{
	make_json_dir();
	char unit_data[256]={0};
	FILE *fp_r= NULL;
	//fp_r = fopen("/tmp/data/map_points.csv","r");
    fp_r = fopen("map_points.csv","r");
    if(!fp_r){
        printf("read map_points.csv  error\n"); 
        sprintf(unit_data,"{\"status\":\"map_points.csv not exist!\"}"); 
		strcat(total_data,unit_data);   
        return 0;
    }
    
    FILE *fp_w = NULL;
    fp_w = fopen("/tmp/data/json/map_points.json","w");
	if(!fp_w){
        printf("write map_points.json file error\n");
        sprintf(unit_data,"{\"status\":\"err2\"}");
		strcat(total_data,unit_data);   
        return 0;
    }
    
    fputs("{\"Results\":[{\"val\":\"head\"}",fp_w);
    
	char tempRow[6][16];
	char tempRead[128];
	memset(tempRead,0,sizeof(tempRead));
	memset(tempRow,0,sizeof(tempRow));
	while(fgets(tempRead, sizeof(tempRead), fp_r)) 
	{
        while(tempRead[strlen(tempRead) - 1] == '\n' || tempRead[strlen(tempRead) - 1] == ' ') 
        	tempRead[strlen(tempRead) - 1] = '\0';
        if(tempRead[0] == '\0') 
		{
            continue;
        }
    	sscanf(tempRead, "%s%s%s%s%s",tempRow[0],tempRow[1],tempRow[2],tempRow[3],tempRow[4]);
    	sprintf(unit_data,",{\"val\":\"%d$%d$%d\"}",atoi(tempRow[1]),atoi(tempRow[2]),atoi(tempRow[3]));
    	fputs(unit_data,fp_w);
		
    }
	fputs("]}",fp_w);	
	fclose(fp_r);
	fclose(fp_w);
	fp_r=NULL;
	fp_w=NULL;
	
	fp_r = fopen("led_points.csv","r");
    if(!fp_r)
	{
        printf("read led_points.csv  error\n"); 
        sprintf(unit_data,"{\"status\":\"led_points.csv not exist!\"}"); 
		strcat(total_data,unit_data);   
        return 0;
    }
    
    fp_w = fopen("/tmp/data/json/led_points.json","w");
	if(!fp_w)
	{
        printf("write led_points.json file error\n");
        sprintf(unit_data,"{\"status\":\"err2\"}");
		strcat(total_data,unit_data);   
        return 0;
    }
    
    fputs("{\"Results\":[{\"val\":\"head\"}",fp_w);
    
	memset(tempRead,0,sizeof(tempRead));
	memset(tempRow,0,sizeof(tempRow));
	while(fgets(tempRead, sizeof(tempRead), fp_r)) 
	{
        while(tempRead[strlen(tempRead) - 1] == '\n' || tempRead[strlen(tempRead) - 1] == ' ') 
        	tempRead[strlen(tempRead) - 1] = '\0';
        if(tempRead[0] == '\0') 
		{
            continue;
        }
    	sscanf(tempRead, "%s%s%s%s",tempRow[0],tempRow[1],tempRow[2],tempRow[3]);
    	sprintf(unit_data,",{\"val\":\"%d$%d$%d\"}",atoi(tempRow[1]),atoi(tempRow[2]),atoi(tempRow[3]));
    	fputs(unit_data,fp_w);
		
    }
	fputs("]}",fp_w);	
	fclose(fp_r);
	fclose(fp_w);
	
	sprintf(unit_data,"{\"status\":\"ok\"}");
	strcat(total_data,unit_data); 
		
	return 0;	
}

//获取图片地址
int CThread_web::get_jpg_path(char *jpg_name,char*out_path)
{
	int max=0;
	char id[32]={0};
	char path[64]={0};
	DIR *pDir;
    struct dirent  *ent;
    sprintf(path,"/tmp/data/image");
	strcpy(out_path,"/slampic/image");
    pDir=opendir(path);
    if(pDir  == NULL){
		DIR *pDir_tmp=opendir("/tmp/data");
		if(pDir_tmp)
		{
			mkdir(path,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
			closedir(pDir_tmp);   
		}
		else
		{
     		mkdir("/tmp/data",S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH); 
     		mkdir(path,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);  
     	}
         pDir=opendir(path);
    }
    while((ent=readdir(pDir)))
	{
        if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
			continue; 
        if(ent->d_type ==8)
		{
        	if(ent->d_name[0]=='r'&&ent->d_name[1]=='e')
				continue;
            sprintf(id,"%c%c%c%c%c",ent->d_name[5],ent->d_name[6],ent->d_name[7],ent->d_name[8],ent->d_name[9]);
            if(atoi(id)>max)
			{
        		max=atoi(id);
        		sprintf(jpg_name,"%s",ent->d_name);
            }
        	
        }
    }
    closedir(pDir);
	return  max;
}

//删除图片
int CThread_web::remove_some_jpg_file(const char *path)
{
	int	i=0;
	char id[32]={0};
	char remove_path[64]={0};
	remove_jpg_pkg store[256];
	memset(store,0,sizeof(store)); 
	DIR *pDir;
	struct dirent  *ent;
    pDir=opendir(path);
    if(pDir  == NULL)
	{
		return 0;
	}
    while((ent=readdir(pDir)))
	{
        if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
			continue; 
        if(ent->d_type == 8)
		{
        	if(ent->d_name[0]=='r'&&ent->d_name[1]=='e')
				continue;
            sprintf(id,"%c%c%c%c%c",ent->d_name[5],ent->d_name[6],ent->d_name[7],ent->d_name[8],ent->d_name[9]);
            store[i].id=atoi(id);
            strcpy(store[i].jpg,ent->d_name);
            if(++i>250)
			{
        		break;
            }
        }
    }
    closedir(pDir);
    
    //升序
    qsort(store,i,sizeof(remove_jpg_pkg),asc_jpg_file);
    
	int dele_i=i-15;
	for(i=0;i<dele_i;++i)
	{
		sprintf(remove_path,"%s/%s",path,store[i].jpg);
		remove(remove_path);
	}
	return 0;	
}


//获取图片地址
int CThread_web::get_mapjpg_path(char *jpg_name,char*out_path)
{
	//remove_some_jpg_file("/tmp/data/mapjpg");
	int max=0;
	char id[32]={0};
	char path[64]={0};
	DIR *pDir;
    struct dirent  *ent;
    sprintf(path,"/tmp/data/mapjpg");
	strcpy(out_path,"/slampic/mapjpg");
    pDir=opendir(path);
    if(pDir  == NULL)
	{
		DIR *pDir_tmp=opendir("/tmp/data");
		if(pDir_tmp)
		{
			mkdir(path,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
			closedir(pDir_tmp);   
		}
		else
		{
     		mkdir("/tmp/data",S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH); 
     		mkdir(path,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);  
     	}
         pDir=opendir(path);
    }
    while((ent=readdir(pDir)))
	{
        if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
			continue; 
        if(ent->d_type ==8)
		{
        	if(ent->d_name[0]=='r'&&ent->d_name[1]=='e')
				continue;
            sprintf(id,"%c%c%c%c%c",ent->d_name[5],ent->d_name[6],ent->d_name[7],ent->d_name[8],ent->d_name[9]);
            if(atoi(id)>max)
			{
        		max=atoi(id);
        		sprintf(jpg_name,"%s",ent->d_name);
            }
        }
    }
    closedir(pDir);
	return  max;
} 

int CThread_web::play_video(char parameter[][64])
{
	char jpg_name[64]={0};
	char path[64]={0};
	char map_jpg_name[64]={0};
	char map_path[64]={0};
	char unit_data[128]={0};
	get_jpg_path(jpg_name,path);
	get_mapjpg_path(map_jpg_name,map_path);
	if(0==strcmp(jpg_name,""))
	{
		sprintf(unit_data,"{\"path\":\"null\",\"map\":\"null\"}");
	}
	else
	{
		sprintf(unit_data,"{\"path\":\"%s/%s\",\"map\":\"%s/%s\"}",path,jpg_name,map_path,map_jpg_name);
	}
	strcat(total_data,unit_data);
	return 0;	
} 

//计算vslam精度
int CThread_web::calculate_point(char parameter[][64])
{
	int slam_x=0;
	int slam_y=0;
	int no=0;
	float theta=0.0;
	char unit_data[256]={0};
	char line_data[256]={0};
	if(search_parameter(parameter,"slam_x")&&search_parameter(parameter,"slam_y")&&search_parameter(parameter,"theta"))
	{
		slam_x=atoi(search_parameter(parameter,"slam_x"));
		slam_y=atoi(search_parameter(parameter,"slam_y"));
		theta=atof(search_parameter(parameter,"theta"));
	}
	else
	{
		printf("can not get slam_x slam_y theta in fun calculate_point line=%d\n",__LINE__);
		sprintf(unit_data,"{\"status\":\"err1\"}");
		strcat(total_data,unit_data);
	}
	
	if(search_parameter(parameter,"no"))
	{
		no=atoi(search_parameter(parameter,"no"));
	}
	else
	{
		printf("can not get no in fun calculate_point line=%d\n",__LINE__);
		sprintf(unit_data,"{\"status\":\"err2\"}");
		strcat(total_data,unit_data);
	}
	
	if(1==no)
	{
		FILE *fp = NULL;
	    fp = fopen("/tmp/data/calculate_point.csv", "w");
		if(!fp)
		{
	        printf("write calculate_point file error\n");   
	        sprintf(unit_data,"{\"status\":\"err3\"}");
			strcat(total_data,unit_data);   
	        return 0;
	    }
	    sprintf(line_data,"no	x	y	theta	dis\n1	%d	%d	%.2f	0\n",slam_x,slam_y,theta);
	    fputs(line_data,fp);
	    fclose(fp);
	    sprintf(unit_data,"{\"status\":\"ok\",\"dis\":\"0\"}");
		strcat(total_data,unit_data);
		return 0; 
	}
	else
	{
		int pre_x=0;
		int pre_y=0;
		FILE *fp = NULL;
	    fp = fopen("/tmp/data/calculate_point.csv", "r");
		if(!fp)
		{
	        printf("read calculate_point file error\n");   
	        sprintf(unit_data,"{\"status\":\"err4\"}");
			strcat(total_data,unit_data);   
	        return 0;
	    }
	    char tempRow[6][16];
		char tempRead[128];
		memset(tempRead,0,sizeof(tempRead));
		memset(tempRow,0,sizeof(tempRow));
		while(fgets(tempRead, sizeof(tempRead), fp)) 
		{
	        while(tempRead[strlen(tempRead) - 1] == '\n' || tempRead[strlen(tempRead) - 1] == ' ') 
	        	tempRead[strlen(tempRead) - 1] = '\0';
	        if(tempRead[0] == '\0') 
			{
	            continue;
	        }
	        if(tempRead[0] == 'n'&&tempRead[1] == 'o') 
			{
	            continue;
	        }
        	sscanf(tempRead, "%s%s%s%s%s",tempRow[0],tempRow[1],tempRow[2],tempRow[3],tempRow[4]);
        }
		fclose(fp);
		fp=NULL;
		pre_x=atoi(tempRow[1]);
		pre_y=atoi(tempRow[2]);
	    fp = fopen("/tmp/data/calculate_point.csv", "a");
		if(!fp)
		{
	        printf("write calculate_point file error\n");   
	        sprintf(unit_data,"{\"status\":\"err5\"}");
			strcat(total_data,unit_data);   
	        return 0;
	    }
	    int dis=sqrt((slam_x-pre_x)*(slam_x-pre_x)+(slam_y-pre_y)*(slam_y-pre_y));
	    sprintf(line_data,"%d	%d	%d	%.2f	%d\n",no,slam_x,slam_y,theta,dis);
	    fputs(line_data,fp);
	    fclose(fp);
	    sprintf(unit_data,"{\"status\":\"ok\",\"dis\":\"%d\"}",dis);
		strcat(total_data,unit_data);
		return 0; 	
		
	}
	
	
	return 0;
}	

//获取配置文件
int CThread_web::get_config_val(char parameter[][64])
{
	char unit_data[128]={0};
	char val[128]={0};
	if(search_parameter(parameter,"name")&&search_parameter(parameter,"path"))
	{
		if(-1==search_config(search_parameter(parameter,"path"),search_parameter(parameter,"name"),val))
		{
			sprintf(unit_data,"{\"val\":\"null\"}");
		}
		else
		{
			sprintf(unit_data,"{\"val\":\"%s\"}",val);
		}
	}
	else
	{
		printf("can not get parameter in fun get_config_val line=%d\n", __LINE__);
		sprintf(unit_data,"{\"val\":\"null\"}");
	}
	strcat(total_data,unit_data);
	return 0;	
}

//停止系统
int CThread_web::stop_web_ui_process(char parameter[][64])
{
	strcat(total_data,"1");
	return 0;	
}

//解析url
int CThread_web::decode_url(char *url,char *path)
{
	int tail = 0;
	if(url[strlen(url)-3]=='%'&&url[strlen(url)-2]=='2'&&url[strlen(url)-1]=='F')
	{
    	tail=1;
    }
	char *p;
	if(NULL==strstr(url,"%2F"))
	{
		strcpy(path,url);
		return 0;
	}
	p = strtok(url,"%2F");
	if(p)
	{
		strcat(path,"/");
		strcat(path,p);
	}
	
	while((p = strtok(NULL, "%2F")))
    {
    	strcat(path,"/");
    	strcat(path,p);
    }
    if(tail)
	{
    	strcat(path,"/");	
    }

	return 0;	
}

//查询配置文件
int CThread_web::search_config(char* path,char* data,char *val)
{
	int i = 0;
	char *p;//要查询的参数的值
	char tempRead[128];
	memset(tempRead,0,sizeof(tempRead));
	char decode_path[128]={0};
	decode_url(path,decode_path);
	FILE *fp = NULL;
	fp = fopen(decode_path, "r");
   	if(!fp)
	{
		printf("read  file error path=%s\n",decode_path);
		return -1;
    }
	while(fgets(tempRead, sizeof(tempRead), fp)) 
	{
        while(tempRead[strlen(tempRead) - 1] == '\n' || tempRead[strlen(tempRead) - 1] == ' ') 
        	tempRead[strlen(tempRead) - 1] = '\0';
        if(tempRead[0] == '\0') 
		{
            continue;
        }
        p=strstr(tempRead,"=");
        *p='\0';
        if(0==strcmp(data,tempRead))
		{
        	strcpy(val,p+1);
        	fclose(fp);
        	return 0;
        }
		if(++i>16)
		{
			break;	
		}
	} 
	fclose(fp);
	return -1;	
}

void CThread_web::exit(int)
{
	pthread_exit(0);
}

//获取本机ip地址（eth0）
int CThread_web::get_server_ip(char*ip,const char*network_name)
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
    int i=(ifconf.ifc_len/sizeof(struct ifreq));
    for (; i>0; i--)
    {
        if(ifreq->ifr_flags == AF_INET){ //for ipv4
            if(0 == strcmp(network_name,ifreq->ifr_name))
            {
				//将后面的字符串复制给前面
            	strcpy(ip ,inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr));
            	return 0;	
        	}
        	ifreq++;
        }
    }
    close(sockfd);
	return 0;	
}

//建立客户套接字
int CThread_web::creat_server_socket(char*ip,int port)
{
	int server_socket = 0;	
	//1	
	server_socket = socket(AF_INET,SOCK_STREAM,0);
	int reuse = 1;
	int retval  = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)); 
	if(retval<0)
	{
		
		printf("setsockopet error ret=%d\n",retval);
		return -1;
	}  
	
	//2
	struct sockaddr_in  server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);
	retval =  bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr));
	if(retval<0)
	{	
											
		printf("ret=%d error：%s \n",retval,strerror(errno)); 
		
		return -1;
	}
	//3
	retval = listen(server_socket,10);
	if(retval ==-1)
	{
		close(server_socket);
		while(1)
		{
			sleep(1);
			printf("listen failed in fun creat_server_socke\n");
		}
	}

	
	return server_socket;	
}


//在二维数组中搜索字段名为data的数据
char *CThread_web::search_parameter(char parameter[][64],const char*data)
{
	int i = 0;
	for(;i<16;++i)
	{
		if(0 == strcmp(parameter[2*i],data))
			return parameter[2*i+1];
		if(parameter[2*i][0]==0)
			return NULL;
	}	
	return NULL;
}

//解析post请求的数据data，保存到二维数组parameter
int CThread_web::decode_post_data(char* read_buf,char parameter[][64])
{
	//判断后面的字符串是否是前面的子串
	char *p = strstr(read_buf,"\r\n\r\n");
	char *q;
	char *a;
	char *b;
	int i = 0;
	if(p+4)
	{
		//以&作为切割点，切割字符串
		a = strtok(p+4,"&");
		b = strstr(a,"=");
		if(b&&a&&b+1)
		{
			*b='\0';
			strcpy(parameter[0],a);
			strcpy(parameter[1],b+1);
			while((q = strtok(NULL, "&")))
	        {
	        	i+=2;
	        	b = strstr(q,"=");
	        	if(b&&q&&b+1)
				{
	        		*b='\0';
	        		strcpy(parameter[i],q);
					strcpy(parameter[i+1],b+1);
				}
	        }	
			return i+2;	
		}
	}
	return 0;	
}

//post表单递交
int CThread_web::post_messages(char*ip,int port)
{
	char read_buf[1024];
	
	memset(read_buf,0,sizeof(read_buf));
	memset(parameter, 0, sizeof(parameter));
	struct sockaddr_in client_socket;
	socklen_t client_addrlen;
	//第一步：建立客户端套接字
	int server_socket = creat_server_socket(ip,port);
	
	while(1)
	{
		memset(&client_socket, 0, sizeof(struct sockaddr_in));  
	    client_addrlen = 160;  
		int client_fd = accept(server_socket,(struct sockaddr*)&client_socket,&client_addrlen);
		if(client_fd==-1)
		{
			printf("client_fd=-1 web over\n");
			return 0;
		}
		memset(read_buf,0,sizeof(read_buf));
		memset(parameter,0,sizeof(parameter));
		int read_len=read(client_fd,read_buf,sizeof(read_buf));
		printf("line=%d,read_buf=%s\n",__LINE__,read_buf);
		if(read_len>1022)
		{
			printf("read:\n%s\nread>1022\n",read_buf);
			return 0;
		}
		else
		{
			//printf("read:\n%s\n",read_buf);
			decode_post_data(read_buf,parameter);
			make_send_buf(parameter);
		}
		printf("line=%d,send_buf=%s\n",__LINE__,send_buf);
		write(client_fd,send_buf,strlen(send_buf));
		close(client_fd);
		if(search_parameter(parameter,"Fun"))
		{
			if(0==strcmp("stop_web_ui_process",search_parameter(parameter,"Fun")))
			{
				close(server_socket);
				cout << "\n\n\nWeb was stop good by command stop_web_ui_process\n "<< endl;
				return 0;
			}
		}

	}
	close(server_socket);
	return 0;
}


bool CThread_web::get_ctrl_info(Web_Ctrl_Struct *curr_ctrl_msg)
{
    {     
        std::unique_lock<std::mutex> lock(mMutexWebCtrlMsg);
        memcpy(curr_ctrl_msg, &web_ctrl_info, sizeof(web_ctrl_info));
    }
    return true;
}
