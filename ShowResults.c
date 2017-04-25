#include <math.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include <unistd.h> 
#include <fcntl.h>

#define FIFO_PERM (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define FIFO_MODES (O_RDONLY | O_WRONLY)


int serverpid=0;
char result[256];

void sigHandler(int signalNo)
{
	FILE *output;
	char bufPid[15];
	
	time_t t;
    time(&t);
    unlink("showserverpid");
	output=fopen("log/showResultLog.log","a");
	fprintf(output, "CTRL + C sinayli geldi %s\n",ctime(&t) );
	fclose(output);
	kill((pid_t)serverpid, SIGINT);
	exit(signalNo);
}

/*sinyal belirleme*/
void initializeSignals(){

  signal(SIGINT,sigHandler);

}

pid_t r_wait(int *stat_loc) {
   int retval;
   while (((retval = wait(stat_loc)) == -1) && (errno == EINTR)) ;
   return retval;
}

int main(int argc, char *argv[]) {

	FILE *output;
	int fdClientShow,fdres1,fdres2, pidShow;
	double result1,result2;
	char bufPid[8];
	pid_t child;
	char fifoClientShow[10]="ShowResult";
	
	if(argc == 1)
	{
		initializeSignals();
		remove("log/showResultLog.log");
		pidShow=(int)getpid();
		output=fopen("log/showResultLog.log","a");
		fprintf(output, "Pid\t\tResult1\t\tTImeElaps\n" );
		fclose(output);
			/*pid result1 resul2 icin icin fifo*/
		if (mkfifo(fifoClientShow, FIFO_PERM) == -1) { 
			if (errno != EEXIST) {
			fprintf(stderr, "[%ld]:failed to create named pipe %s: %s\n",
			(long)getpid(), fifoClientShow, strerror(errno));
			return 1;
			}
		}	
		if (mkfifo("res1", FIFO_PERM) == -1) { 
			if (errno != EEXIST) {
			fprintf(stderr, "[%ld]:failed to create named pipe %s: %s\n",
			(long)getpid(), "res1", strerror(errno));
			return 1;
			}
		}	
	
	 if (mkfifo("showserverpid", FIFO_PERM) == -1) 
	{ 
		if (errno != EEXIST) 
		{
		fprintf(stderr, "[%ld]:failed to create named pipe %s: %s\n",
		(long)getpid(), "showserverpid", strerror(errno));
		return 1;
		}
	 }
		
		/*surekli pid result1 result2 bekleyecek*/
		while(1)
		{
		printf("Clientdan server Pid sini bekliyor \n"  );	
		while (((fdClientShow = open("showserverpid", O_RDONLY)) == -1) && (errno == EINTR)) ;
						if (fdClientShow == -1) {
							fprintf(stderr, "FiFoMain failed to open named pipe for read:\n");
							return 1;
			}	
	 	read(fdClientShow, &serverpid,  sizeof(int));	
	 	printf("server Pid Name= %d*\n", serverpid);
	 	close(fdClientShow);
	 	/*********************end of getPid*****************************/

		printf("Cliena Pid sini gonderiyor \n"  );	
		while (((fdClientShow = open(fifoClientShow, O_WRONLY)) == -1) && (errno == EINTR)) ;
						if (fdClientShow == -1) {
							fprintf(stderr, "FiFoMain failed to open named pipe for read:\n");
							return 1;
			}	
	 	write(fdClientShow, &pidShow,  sizeof(int));	
	 	printf("Show Pid Name= %d*\n", pidShow);
	 	close(fdClientShow);
	 	/*********************end of getPid*****************************/	
		
		printf("Clientdan Pid  bekliyor \n"  );	
		while (((fdClientShow = open(fifoClientShow, O_RDONLY)) == -1) && (errno == EINTR)) ;
						if (fdClientShow == -1) {
							fprintf(stderr, "FiFoMain failed to open named pipe for read:\n");
							return 1;
			}	
	 	read(fdClientShow, bufPid,  sizeof(bufPid));
	 	printf("Pid Name= %s*\n", bufPid);
	 	close(fdClientShow);
	 	/*********************end of getPid*****************************/
	 	

	 	child=fork();
		 if(child==0)
		 {

		 	while(1){
		 	printf("ShowResult Clienttan  shifted result bekliyor \n"  );
			while (((fdres1 = open("res1", O_RDONLY)) == -1) && (errno == EINTR)) ;
					if (fdres1 == -1) {
						fprintf(stderr, "FifoShowResult1 failed to open named pipe for read:\n");
						return 1;
					}	
			
			read(fdres1,result,sizeof(result));
			printf("Result1 = %s\n",  result);
			output=fopen("log/showResultLog.log","a");
			fprintf(output, "%s\n", result);
			fclose(output);
			close(fdres1);
			//exit(0);
			sleep(1);
		 }

		}

		}
		while (r_wait(NULL) > 0);

	}
	else
	{
		fprintf(stderr,"/*Usage : ShowReslts ***/\n");
		return 1;
	}

}
