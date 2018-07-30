/*
 * 
 * 
 * 
 * 
 */

#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>


#include "web.h"

using namespace std;


void creat_daemon(void)
{
    pid_t pid;
    pid = fork();
    if( pid == -1)
        ERR_EXIT("fork error");
    if(pid > 0 )
        exit(EXIT_SUCCESS);
    if(setsid() == -1)
        ERR_EXIT("SETSID ERROR");
    chdir("/");
    int i;
    for( i = 0; i < 3; ++i)
    {
        close(i);
        open("/dev/null", O_RDWR);
        dup(0);
        dup(0);
    }
    umask(0);
    return;
}


int is_which_environment()
{
	char *p=NULL;
	char tempRead[128];
	memset(tempRead,0,sizeof(tempRead));
	FILE *fp = NULL;
	fp = fopen(".config", "r");
   	if(!fp){
		return 0;
    }
	while(fgets(tempRead, sizeof(tempRead), fp)) {
        while(tempRead[strlen(tempRead) - 1] == '\n' || tempRead[strlen(tempRead) - 1] == ' ') 
        	tempRead[strlen(tempRead) - 1] = '\0';
        if(tempRead[0] == '\0') {
            continue;
        }
        p=strstr(tempRead,"=");
        *p='\0';
        if(p+1){
	        if(0==strcmp("environment",tempRead)&&0==strcmp("ubuntu",p+1)){
	        	return 1;
	        }else{
				return 0;
			}
		}
	} 
	fclose(fp);
	return 0;	
}


int main(int argc, char **argv)
{
	
	if(argv[1]){
		if(0==strcmp("-h",argv[1])){
			printf("\n\n	1----use ssh run command \"vslam.sh\" for running web process\n\
							2----Enter the IP address in the browser, open the web page\n\
							3----Wait for a few seconds until the left top image appears\n\
							4----The mouse clicks in the non image area, and the web page gets the keyboard event\n\
							5----Control instructions----|\n\
				     |\n\
				     |\n\
				     |----8(up)		front\n\
				     |----2(down)	back\n\
				     |----4(left)	left	rotate\n\
				     |----6(right)	right	rotate\n\
				     |----5(space)	stop\n\
				     |----A		Visual interface becomes larger and smaller\n\
				     |----H		Hidden trajectory\n\
				     |----T		Hidden walk area\n\
				     |----W		Wall\n\
				     |----Z		Auto\n\n\n");	
			return 0;
		}
	}
	//creat_daemon();
	CThread_web web_thread;
    web_thread.process_CThread_web(is_which_environment());

    
}

