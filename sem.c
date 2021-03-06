#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#define MSG_SIZE 64
#define NAME_SIZE 20

void failcheck(int rv, int line)
{
	if(rv<0)
	{
		if(errno==EEXIST)
			return;
		fprintf(stderr,"%s: %s (Error in line: %d)\n", __FILE__, strerror(errno) , line);
		exit(-1);
	}
}

int main(int argc, char *argv[])
{
	key_t key;
	int shmid, semid;
	pid_t pid;
	struct sembuf op;
	struct shmid_ds buf;
	int rv;
	char *name1, *name2, *msg_re, *msg_wr, *flag_re, *flag_wr, *attach;

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
		flag_wr=attach;
		*flag_wr=0;

		name1=flag_wr+1;
		strcpy(name1, argv[1]);

		msg_wr=name1+NAME_SIZE;

		/*********************/

		flag_re=msg_wr+MSG_SIZE;
		*flag_re=0;

		name2=msg_wr+MSG_SIZE+1;
		strcpy(name2, argv[2]);

		msg_re=name2+NAME_SIZE;
	}
	else
	{
		flag_wr=attach;
		name1=flag_wr+1;
		msg_wr=name1+NAME_SIZE;

		/*********************/

		flag_re=msg_wr+MSG_SIZE;
		name2=msg_wr+MSG_SIZE+1;
		msg_re=name2+NAME_SIZE;

		if(strcmp(name2, argv[1])==0)
		{
			flag_wr=msg_wr+MSG_SIZE;
			msg_wr=name2+NAME_SIZE;

			/*********************/

			flag_re=attach;
			msg_re=name1+NAME_SIZE;
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
	
	errno=0;
	semid=semget(key, 2, IPC_CREAT|IPC_EXCL|0666);
	
	if((errno!=EEXIST)&&(semid<0))
		failcheck(semid, __LINE__-2);
	if(errno==EEXIST)
	{
		semid=semget(key, 2, 0);
		failcheck(shmid, __LINE__-1);
	}
	else
	{
		rv=semctl(semid, 0, SETVAL, 0);
		failcheck(rv, __LINE__-1);
		rv=semctl(semid, 1, SETVAL, 0);
		failcheck(rv, __LINE__-1);
	}

	pid=fork();
	failcheck(pid, __LINE__-1);

	if(pid==0)
	{	
		do
		{
			if(strcmp(argv[1], name2)==0)
			{
				op.sem_num=0;
				op.sem_op=-1;
				op.sem_flg=0;
			
				semop(semid, &op, 1);
				printf("reduced sem0\n");
			}
			else if(strcmp(argv[1], name1)==0)
			{
				op.sem_num=1;
				op.sem_op=-1;
				op.sem_flg=0;
			
				semop(semid, &op, 1);
				printf("reduced sem1\n");
			}
			
			if(strcmp(name1, argv[1])==0)
				printf("%s: %s", name2, msg_re);
			else
				printf("%s: %s", name1, msg_re);
		}while(1);

		_exit(0);
	}
	
	int flag=0;
	do
	{	
		fgets(msg_wr, MSG_SIZE, stdin);
		if(strcmp(msg_wr, "quit\n")==0)
		{
			rv=kill(pid, SIGINT);
			failcheck(rv, __LINE__-1);
				
			rv=kill(getpid(), SIGINT);
			failcheck(rv, __LINE__-1);
				
				
// 			rv=shmctl(shmid,  IPC_RMID, NULL);
// 			failcheck(rv, __LINE__-1);
		}
		
		rv=shmctl(shmid, IPC_STAT, &buf);
		failcheck(rv, __LINE__-1);
		
		if((buf.shm_nattch==2)&&(flag==0))
		{
			if(strcmp(argv[1], name1)==0)
			{
				op.sem_num=0;
				op.sem_op=-1;
				op.sem_flg=0;
			
				semop(semid, &op, 1);
				printf("increased sem0\n");
			}
			else if(strcmp(argv[1], name2)==0)
			{
				op.sem_num=1;
				op.sem_op=-1;
				op.sem_flg=0;
			
				semop(semid, &op, 1);
			
				printf("increased sem1\n");
			}
			flag=1;
		}
		if(buf.shm_nattch==4)
		{
			if(flag==1)
			{
				if(strcmp(argv[1], name1)==0)
				{
					op.sem_num=1;
					op.sem_op=1;
					op.sem_flg=0;
			
					semop(semid, &op, 1);
					printf("increased sem0\n");
				}
				else if(strcmp(argv[1], name2)==0)
				{
					op.sem_num=0;
					op.sem_op=1;
					op.sem_flg=0;
			
					semop(semid, &op, 1);
			
					printf("increased sem1\n");
				}
				flag=2;
			}
			if(strcmp(argv[1], name1)==0)
			{
				op.sem_num=0;
				op.sem_op=1;
				op.sem_flg=0;
			
				semop(semid, &op, 1);
				printf("increased sem0\n");
			}
			else if(strcmp(argv[1], name2)==0)
			{
				op.sem_num=1;
				op.sem_op=1;
				op.sem_flg=0;
			
				semop(semid, &op, 1);
			
				printf("increased sem1\n");
			}
		}
	}while(1);
	
	return 0;
}