#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include "chatf.h"

#define MES_SIZE 64
#define NAME_SIZE 20

int main(int argc, char *argv[])
{
	key_t key;
	int shmid;
	pid_t pid;
	struct shmid_ds buf;
	int rv;
	int *wflag;
	int *attach;
	char *name, *mes;
	
	if(argc!=3)
	{
		fprintf(stderr, "%s: Expected format (./chat username user_to_communicate) (Error in line: %d)\n", __FILE__, __LINE__);
		return(-1);
	}
	
	key=ftok(".", 23);
	failcheck(key, __LINE__-1);
	
	shmid=shmget(key, 2*(MES_SIZE+NAME_SIZE+1), IPC_CREAT|IPC_EXCL|0666);
	if(errno!=EEXIST)
		failcheck(shmid, __LINE__-2);
	else if(shmid>0)
	{
		attach=(int *)shmat(shmid,NULL,0);
		failcheck(*attach, __LINE__-1);
		
		wflag=attach;
		name=attach+1;
		mes=name+20;
	}
	else
	{
		shmid=shmget(key, 2*(MES_SIZE+NAME_SIZE+1), 0);
		failcheck(shmid, __LINE__-1);
		r
	}
	
	pid=fork();
	failcheck(pid, __LINE__-1);
	
	if(pid==0)
	{
		
		rv=shmctl(shmid, IPC_STAT, &buf); 
		failcheck(rv, __LINE__-1);
		do
		{
			if(buf.shm_nattch==2)
			{
				fprintf(
			}
		}while(buf.shm_nattch!=2);
	}
	
	
	
	
	
	
	
	return 0;
}
