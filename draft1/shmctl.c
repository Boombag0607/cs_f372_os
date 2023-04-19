#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>

int main()
{

    for (int shmid1 = 1500000; shmid1 < 2000000; shmid1++)
        shmctl(shmid1, IPC_RMID, NULL);
}