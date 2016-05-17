#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <unistd.h>

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
	char *name1, *name2, *msg_Re, *msg_Wr, *flag_Re, *flag_Wr, *attach;

	if(argc!=3)
	{
		printf("Expected format: ./chat username user_to_communicate\n");
		return(-1);
	}

	key=ftok(".", 23);
	failcheck(key, __LINE__-1);

	shmid=shmget(key, 2*(MSG_SIZE+NAME_SIZE+1), IPC_CREAT|IPC_EXCL|0666);
	if((errno!=EEXIST)&&(shmid<0))
		failcheck(shmid, __LINE__-2);

	if(errno==EEXIST)
	{
		shmid=shmget(key, 0, 0);
		failcheck(shmid, __LINE__-1);
	}

	attach=(char *)shmat(shmid,NULL,0);
	failcheck(*attach, __LINE__-1);

	if((shmid>0)&&(errno!=EEXIST))
	{
		//printf("Hi1\n");

		flag_Wr=attach;
		*flag_Wr='0';

		name1=flag_Wr+1;
		strcpy(name1, argv[1]);

		msg_Wr=name1+NAME_SIZE;
		
		/*********************/
		
		flag_Re=msg_Wr+MSG_SIZE;
		*flag_Re='0';
		
		name2=msg_Wr+MSG_SIZE+1;
		strcpy(name2, argv[2]);
		
		msg_Re=name2+NAME_SIZE;
	}
	else
	{
		//printf("Hi2\n");

		flag_Wr=attach;

		name1=flag_Wr+1;

		msg_Wr=name1+NAME_SIZE;
		
		/*********************/

		flag_Re=msg_Wr+MSG_SIZE;

		name2=msg_Wr+MSG_SIZE+1;
		
		msg_Re=name2+NAME_SIZE;

		if(strcmp(name2, argv[1])==0)
		{
			
			printf("Hi2\n");
			
			flag_Wr=msg_Wr+MSG_SIZE;

			msg_Wr=name2+NAME_SIZE;
			
			/*********************/
		
			flag_Re=attach;
			
			msg_Re=name1+NAME_SIZE;
		}
		else if(strcmp(name1, argv[1])!=0)
		{
			printf("%s is currently chatting with someone else\n", argv[2]);
			return(-1);
		}

		if(strcmp(argv[2],name1)!=0 && strcmp(argv[2],name2)!=0)
		{
			if(strcmp(argv[1],name1)==0)
			{
				printf("You already have a chat open with %s\n", name2);
				return(-1);
			}
			printf("You already have a chat open with %s\n", name1);
			return(-1);
		}
	}

	pid=fork();
	failcheck(pid, __LINE__-1);

	if(pid==0)
	{
		rv=shmctl(shmid, IPC_STAT, &buf);
		failcheck(rv, __LINE__-1);
		
		while(buf.shm_nattch!=2)
		{
			;
		}
		
		while(*flag_Re=='0')
		{
			if(*flag_Re=='1')
			{
				if(strcmp(name1, argv[1])==0)
					printf("%s: %s", name2, msg_Re);
				else
					printf("%s: %s", name1, msg_Re);
				
				*flag_Re='0';	
			}
		}
		
		printf("Hi3\n");
		
		_exit(0);
	}

	getchar();

	return 0;
}
