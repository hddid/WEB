#pragma once
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <iomanip>
#include <mutex>
#include <dirent.h>
#include <math.h>
#include <sys/stat.h>
#include <errno.h> 
#include "direction.h"
#include "share_memory_api.h"
#define ERR_EXIT(m) \
do\
{\
    perror(m);\
    exit(EXIT_FAILURE);\
}\
while (0);\

#define WEB_DATA_MAX_COUNT		1    
#define WEB_SHM_KEY 			140012
#define WEB_SEM_KEY 			140010
#define WEB_ZERO_OFFSET 512


typedef struct web_ctrl_struct
{
	bool newfg;
    int16_t line_speed;     	
    int16_t radius;         	
    uint8_t	which_robot;		
								
	int32_t	goto_x;				
	int32_t	goto_y;				
    uint8_t ctrl_mode;      	
								
}Web_Ctrl_Struct;


typedef struct remove_jpg_PKG
{
	int id;
    char jpg[64]; 
}remove_jpg_pkg;


  
typedef struct web_SHMU {
      int32_t head; 
      uint32_t tail; 
      uint32_t read_index; 
      uint32_t size; 
      bool full;
      bool empty;
      Web_Ctrl_Struct web_ctrl_data;
}web_shmu,*pweb_shmu;

class CThread_web {
public:
    CThread_web();
    ~CThread_web();

    void  	exit(int);		  
    void 	*process_CThread_web(int mode);	
    int ctrl_cleanner_robot(char parameter[][64]);
    int add_slam_point(char parameter[][64]);
    int decode_post_data(char* read_buf,char parameter[][64]);
    char *search_parameter(char parameter[][64],const char*data);
    int creat_server_socket(char*ip,int port);
    int get_server_ip(char*ip,const char*network_name);
    int make_send_buf(char parameter[][64]);
    bool get_ctrl_info(Web_Ctrl_Struct *info);
    int get_jpg_path(char *jpg_name,char*out_path);
    int get_mapjpg_path(char *jpg_name,char*out_path);
    int play_video(char parameter[][64]);
    int get_config_val(char parameter[][64]);
	int search_config(char* path,char* data,char *val);
	int decode_url(char *url,char *path);
	int init_map_file(char parameter[][64]); 
	int calculate_point(char parameter[][64]); 
	int get_map_file(char parameter[][64]);
	int get_small_map_file(char parameter[][64]);
	int show_red_led(char parameter[][64]);
	int make_hand_draw_map(char parameter[][64]);
	int draw_route_in_snake(char parameter[][64]);
	int save_map_file(char parameter[][64]);
	int stop_web_ui_process(char parameter[][64]);
	int init_debug_map(char parameter[][64]);
	int add_debug_point(char parameter[][64]);
	int show_door(char parameter[][64]);
private:
    int draw_one_point(int x1,int y1);
    int draw_middle_snake_point(int x,int y,int snake_map_point_pre_x,int snake_map_point_pre_y);
	int snake_map_point_pre_x=0;		//
    int snake_map_point_pre_y=0;
    MapPoints snake_map_point[1024][1024];
    
	int draw_small_line(int x1,int y1,int x2,int y2,int x_offset,int y_offset);
	int draw_big_line(int x1,int y1,int x2,int y2,int x_offset,int y_offset);
	float get_diff(int x1,int y1,int x2,int y2,float mx,float my);
	int draw_big_line_direction(int x1,int y1,int x2,int y2,int dir,int x_offset,int y_offset);
	int draw_fang(int x1,int y1,int x2,int y2,int x_offset,int y_offset);
	
		
	char parameter[32][64];
	static char total_data[30000];
    static char send_buf[31000];
    static char ifname[64];
    static char map_file_csv_path[128];
    
    int post_messages(char*ip,int port);
    int make_json_dir();
	int caculate(char *result);
	int str_to_json(MapPoints *tempRead,FILE *fp_w,int line_count);
	int decode_one_line_map_data(FILE* fp_r,FILE* fp_w);
	int decode_map_color(uint8_t state,char *color);
	int remove_some_jpg_file(const char *path);
    int remove_all_jpg_file(const char *path);
    Web_Ctrl_Struct web_ctrl_info;
    std::mutex mMutexWebCtrlMsg;
    int		ctrl_mode=0;
	int		which_robot=0;
	int		line_speed = 0;
	int		radius = 0;
	int		goto_x = 0;
	int		goto_y = 0;
        
	web_shmu *pweb_shm;
	web_shmu *pweb_shm_read;

	SMHandle web_shm_handler;
      
    int web_sem_id;
    
};

