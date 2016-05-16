#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include "chatf.h"

#define MSG_SIZE 64
#define NAME_SIZE 20

void failcheck(int rv, int line)
{
	if(rv<0)
	{
		fprintf(stderr,"%s: %s (Error in line: %d)\n", __FILE__, strerror(errno) , line);
		exit(-1);
	}
}

int main(int argc, char *argv[])
{
	key_t key;
	int shmid;
	pid_t pid;
	struct shmid_ds buf;
	int rv;
	int *flag1, *flag2, *attach;
	char *name1, *msg1, *name2, *msg2;

	if(argc!=3)
	{
		fprintf(stderr, "%s: Expected format (./chat username user_to_communicate) (Error in line: %d)\n", __FILE__, __LINE__);
		return(-1);
	}

	key=ftok(".", 23);
	failcheck(key, __LINE__-1);

	shmid=shmget(key, 2*(MSG_SIZE+NAME_SIZE+1), IPC_CREAT|IPC_EXCL|0666);
	if(errno!=EEXIST)
		failcheck(shmid, __LINE__-2);
	else if(shmid>0)
	{
		attach=(int *)shmat(shmid,NULL,0);
		failcheck(*attach, __LINE__-1);

		flag1=attach;
		*flag1=0;

		name1=attach+1;
		strcpy(name1, argv[1]);

		msg1=name1+NAME_SIZE;

		flag2=msg1+MSG_SIZE;
		*flag2=0;

		name2=flag2+1;
		strcpy(name2, argv[2]);

		msg2=name2+NAME_SIZE;
	}
	else
	{
		shmid=shmget(key, 0, 0);
		failcheck(shmid, __LINE__-1);

		attach=(int *)shmat(shmid,NULL,0);
		failcheck(*attach, __LINE__-1);

		if(strcmp(name1, argv[1])==0)
		{
			
		}


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
