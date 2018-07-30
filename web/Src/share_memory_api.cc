#include "share_memory_api.h"

#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <errno.h>

namespace ShareMemApi {

    SMHandle CreateShareMem(SM_KEY key, SM_SIZE size) {
        

        SMHandle hd = shmget(key, size, IPC_CREAT |  0666);
        return hd;
        
    }

    SMHandle OpenShareMem(SM_KEY key, SM_SIZE Size) {
        
        //char keybuf[64];
        //memset(keybuf,0,64);
        //sprintf(keybuf,"./%d",key);

        //key = ftok(keybuf,'w'); 
        SMHandle hd = shmget(key, Size, 0);
        printf("handle = %d ,key = %d ,error: %d \r\n", hd, key, errno);
        return hd;
        
    }

    void* MapShareMem(SMHandle handle) {
        

        return shmat(handle, 0, 0);
        

    }

    int UnMapShareMem(const void* MemoryPtr) {
        
        return shmdt(MemoryPtr);
        
    }

    int CloseShareMem(SMHandle handle) {
        
        return shmctl(handle, IPC_RMID, 0);
        
    }
    
    
int set_semvalue(int sem_id)
{
    
    //用于初始化信号量，在使用信号量前必须这样做
    union semun sem_union;

    sem_union.val = 1;
    if(semctl(sem_id, 0, SETVAL, sem_union) == -1)
        return -1;
    else
    	return 1;
    
}

void del_semvalue(int sem_id)
{
    
    //删除信号量
    union semun sem_union;

    if(semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
        ;
//     std::cout << "Failed to delete semaphore, " << __FUNCTION__ << "\t"<< __LINE__ << std::endl;
    
}

int semaphore_p(int sem_id)
{
    
    //对信号量做减1操作，即等待P（sv）
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1;//P()
    sem_b.sem_flg = SEM_UNDO;
    if(semop(sem_id, &sem_b, 1) == -1)
    {
//         std::cout << "semaphore_p failed, " << __FUNCTION__ << "\t"<< __LINE__ << std::endl;
        return -1;
    }
    else
	{
		return 1;	
	}    
}


int semaphore_p_no_wait(int sem_id)
{
    
    //对信号量做减1操作，即等待P（sv）
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1;//P()
    sem_b.sem_flg = IPC_NOWAIT;
    if(semop(sem_id, &sem_b, 1) == -1)
    {
//         std::cout << "semaphore_p failed, " << __FUNCTION__ << "\t"<< __LINE__ << std::endl;
//         perror("semop");
        return -1;
    }
    else
    {
    	return 1;
    }
    
}

int semaphore_v(int sem_id)
{
    
    //这是一个释放操作，它使信号量变为可用，即发送信号V（sv）
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;//V()
    sem_b.sem_flg = SEM_UNDO;
    if(semop(sem_id, &sem_b, 1) == -1)
    {
//         std::cout << "semaphore_v failed, " << __FUNCTION__ << "\t"<< __LINE__ << std::endl;
        return -1;
    }
    else
    {
    	return 1;
    }
    
    
}


}
