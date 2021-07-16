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


typedef struct
{
    int ValidBit;
    int FrameNo;
    int DirtyBit;
    int RequestedId;
    int timeLastAccessed
} page_table_entry;

typedef page_table_entry* page_table_pointer;
page_table_pointer PageTable;
void SigContHandler(int signum){}

void PrintPT(page_table_entry PageTable[],int NumberOfPages)
{

    int i;
    printf("PageNo\tValid\tFrame\tDirty\tRequested\n");
    for (i=0;i<NumberOfPages;i++)
    {
        printf("%d\t%d\t%d\t%d\t%d\n",i,
        PageTable[i].ValidBit,PageTable[i].FrameNo,PageTable[i].DirtyBit,PageTable[i].RequestedId);
    }
}
int main(int argc,char *argv[])
{
    int Key,NumberOfPages,OSPID,SegID,Mode,Page;
    page_table_pointer PageTable;
    if (argc < 2 ||(OSPID = Key = atoi(argv[argc-1])) == 0  ||(NumberOfPages = atoi(argv[1])) == 0)
    {
        printf("Usage:");
        exit(EXIT_FAILURE);
    }
    if ((SegID = shmget(Key,NumberOfPages*sizeof(page_table_entry),0)) == -1 ||(PageTable = (page_table_pointer)shmat(SegID,NULL,0)) == NULL)
    {
         exit(EXIT_FAILURE);
    }
    if (signal(SIGCONT,SigContHandler) == SIG_ERR)
    {
        printf("Could not initialize continue handler\n");
        exit(EXIT_FAILURE);
    }
    printf("Initialized page table:\n");
    PrintPT(PageTable,NumberOfPages);
    printf("\n");
    for (int i=2;i<argc-1;i++)
    {
        Mode=argv[i][0];
        Page=atoi(&argv[i][1]);
        if (Page>=NumberOfPages)
        {
            printf("That page number in %c%d is outside the process\n",Mode,Page);
        }
          else
        {
            printf("Request for page %d in %c mode\n",Page,Mode);
            if (!PageTable[Page].ValidBit)
            {
                printf("Page fault has occured since the requested page is not in RAM\n");
                PageTable[Page].RequestedId = getpid();
                sleep(1);
                if (kill(OSPID,SIGUSR1) == -1)
                {
                    exit(EXIT_FAILURE);
                }
                pause();
                if (!PageTable[Page].ValidBit)
                {
                    printf("ERROR\n");
                }
            }
            else
           {
                printf("It is in RAM\n");
            }
            if (Mode=='W')
            {
                printf("Setting the dirty bit for page %d\n",Page);
                PageTable[Page].DirtyBit = 1;
            }
            PrintPT(PageTable,NumberOfPages);
            printf("\n");
        }
    }
    if (shmdt(PageTable)==-1)
    {
        exit(EXIT_FAILURE);
    }
    printf("MMU has completed\n");
    sleep(1);
    if (kill(OSPID,SIGUSR1) == -1)
    {
        exit(EXIT_FAILURE);
    }
    return(EXIT_SUCCESS);

}



