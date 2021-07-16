#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <time.h>

typedef struct
{
    int ValidBit;
    int FrameNo;
    int DirtyBit;
    int RequestedId;
    int timeLastAccessed
} page_table_entry;

typedef page_table_entry* page_table_pointer;

int pageNo;
int frameNo;
int frameAvailable;
int diskCounter = 0;
int ShmId1;
page_table_pointer PageTable;

int victimAlocate(page_table_pointer pp)
{
        clock_t lastAccessedMinTime = clock();
        int k;
        int hold_k = -1;
        for(k = 0; k < pageNo;k++)
        {
                if(pp[k].ValidBit)
                {
                        if(lastAccessedMinTime > pp[k].timeLastAccessed)
                        {
                                lastAccessedMinTime = pp[k].timeLastAccessed;
                                hold_k = k;
                        }
                }
        }

        printf("Choosing a victim page %d\n",hold_k);
        if(pp[hold_k].DirtyBit)
        {
                printf("The Victim is dirty, write out\n");
                diskCounter++;
                     if(lastAccessedMinTime > pp[k].timeLastAccessed)
                        {
                                lastAccessedMinTime = pp[k].timeLastAccessed;
                                hold_k = k;
                        }
                }

          printf("Choosing a victim page %d\n",hold_k);
        if(pp[hold_k].DirtyBit)
        {
                printf("The Victim is dirty, write out\n");
                diskCounter++;
                sleep(1);
        }
        int ret = pp[hold_k].FrameNo;
        pp[hold_k].ValidBit = 0;
        pp[hold_k].FrameNo = -1;
        pp[hold_k].DirtyBit = 0;
        pp[hold_k].RequestedId = 0;
        return ret;
}
int randomAlocate()
{
        frameAvailable--;
        return (frameNo - frameAvailable - 1);
}

void requestHandler(int pages,page_table_pointer pp)
{
        int frameHold = -1,hold = -1,j = 0;
        for(j = 0 ; j < pages;j++)
        {
                if(pp[j].RequestedId != 0)
                {
                        printf("Process %d has requested page %d\n",pp[j].RequestedId,j);

                        if(frameAvailable)
                        {
                                frameHold = randomAlocate();
                                printf("Put it in free frame %d\n",frameHold);
                        }

                        else
                        {
                                frameHold = victimAlocate(pp);
                                printf("Put in victim's frame %d\n",frameHold);
                        }
                        hold = pp[j].RequestedId;
                        pp[j].ValidBit = 1;
                        pp[j].FrameNo = frameHold;
                        pp[j].DirtyBit = 0;
                        pp[j].RequestedId = 0;
                        break;
                }
        }
         if(hold != -1)
        {
                diskCounter++;
                sleep(1);
                printf("Unblock MMU\n\n");
                kill(hold,SIGCONT);
        }
          else
        {

                if(shmdt(PageTable) == -1)
                {
                        exit(EXIT_FAILURE);
                }
                if(shmctl(ShmId1,IPC_RMID,NULL) == -1)
                {
                        exit(EXIT_FAILURE);
                }
                printf("%d disk accesses required\n",diskCounter);
                exit(EXIT_SUCCESS);
        }
}
void SigUsrHandler(int signum)
{
        if(signum==SIGUSR1)
        {
                requestHandler(pageNo,PageTable);
        }
}
int main(int argc,char *argv[])
{
        pageNo = atoi(argv[1]);
        frameNo = atoi(argv[argc-1]);
        frameAvailable = frameNo;
        key_t key = getpid();
        if(ShmId1 = shmget(key,pageNo * sizeof(page_table_entry),IPC_CREAT | 0660)==-1)
        {
                 exit(EXIT_FAILURE);
        }
          if((PageTable = (page_table_pointer)shmat(ShmId1,NULL,0)) == NULL)
        {
                exit(EXIT_FAILURE);
        }

        printf("The shared memory key is %d\n",key);
        int i;
        for(i = 0; i < pageNo;i++)
        {
                PageTable[i].ValidBit = 0;
                PageTable[i].FrameNo = -1;
                PageTable[i].DirtyBit = 0;
                PageTable[i].RequestedId = 0;
        }

        printf("page table is initialised\n");
        if(signal(SIGUSR1,SigUsrHandler) == SIG_ERR)
        {
                exit(EXIT_FAILURE);
        }
        while(1){

        }
}
