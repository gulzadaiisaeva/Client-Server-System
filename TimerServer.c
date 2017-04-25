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

#define FIFONAME "log/serverLog.log"

int arrayOfClients[1000];
int sizeOfClients=0;
char fifoname[15];
int chilpid=0;



void sigHandler(int signalNo)
{
	FILE *output;
	char bufPid[15];
	int i;
	
	time_t t;
    time(&t);

    unlink(fifoname);
    //printf("\n CTRL + C sinayli geldi : %s\n",ctime(&t));
	output=fopen(FIFONAME,"a");
	fprintf(output, "CTRL + C sinayli geldi %s\n",ctime(&t) );
	fclose(output);
	for (i = 0; i < sizeOfClients; ++i)
	{
		sprintf(bufPid, "%d", arrayOfClients[sizeOfClients]);
		kill((pid_t)arrayOfClients[sizeOfClients],SIGINT);
		unlink(bufPid);
	}
	kill(chilpid,SIGINT);
	exit(signalNo);
	
}

/*sinyal belirleme*/
void initializeSignals(){

  signal(SIGINT,sigHandler);

}
void freeArray(double **a, int m) {
    int i;
    for (i = 0; i < m; ++i) {
        free(a[i]);
    }
    free(a);
}
/*kitaptan alindi*/
pid_t r_wait(int *stat_loc) {
   int retval;
   while (((retval = wait(stat_loc)) == -1) && (errno == EINTR)) ;
   return retval;
}

/*http://www.geeksforgeeks.org/adjoint-inverse-matrix/
  sitesinden alindi*/
void cofactor(double **A, double **temp, int p, int q, int n)
{
    int i = 0, j = 0,
    	row, col;
 
    // Looping for each element of the matrix
    for (row = 0; row < n; row++)
    {
        for (col = 0; col < n; col++)
        {
            //  Copying into temporary matrix only those element
            //  which are not in given row and column
            if (row != p && col != q)
            {
                temp[i][j++] = A[row][col];
 
                // Row is filled, so increase row index and
                // reset col index
                if (j == n - 1)
                {
                    j = 0;
                    i++;
                }
            }
        }
    }
}
/*http://www.geeksforgeeks.org/adjoint-inverse-matrix/
  sitesinden alindi*/
double Determinant(double **A, int n)
{
    double D = 0; // Initialize result
    double **temp; // To store cofactors
    int sign = 1;  // To store sign multiplier
    int i, j, f;
 
    //  Base case : if matrix contains single element
    if (n == 1)
        return A[0][0];
 
    
    temp = (double**)malloc(n*sizeof(double*));
    for (i = 0; i < n; ++i)
    {
        temp[i] = (double*)malloc(n*sizeof(double));
    }

 
     // Iterate for each element of first row
    for (f = 0; f < n; f++)
    {
        // Getting Cofactor of A[0][f]
        cofactor(A, temp, 0, f, n);
        D += sign * A[0][f] * Determinant(temp, n - 1);
 
        // terms are to be added with alternate sign
        sign = -sign;
    }


 
    return D;
}


/*tek boyutlu arrayi 2 boyutluya cevirir*/
void convert(double *matrix, double **dmatrix, int n)
{
	int i,j;
	for (i = 0; i < n; ++i)
	{
		for ( j = 0; j < n; ++j)
		{
			dmatrix[i][j] = matrix[i*n+j];
		}
	}
}
int main(int argc, char *argv[]) {

	FILE *serverLog;
	struct timeval tp;
	long int ms;
	double det=0;
	int size=0,i,j,n=0,serverPid=0;
	int fdfifoMain,fdFifoClient,fdsize,fdserver;
	double **detMatr;
	double *matris;
	char bufPid[8];
	pid_t pidchild;			/* forklama icin */

	initializeSignals();
	

	if(argc==4){
	
	size=atoi(argv[2]);
	size=size*2;
	strcpy(fifoname,argv[3]);
	serverPid=(int)getpid();

	/*request icin fifo*/
	if (mkfifo(argv[3], FIFO_PERM) == -1 ) { /* create a named pipe */
		if (errno != EEXIST) {
		fprintf(stderr, "[%ld]:failed to create named pipe %s: %s\n",
		(long)getpid(), argv[1], strerror(errno));
		return 1;
		}
	}

	/*pid icin fifo*/
	if (mkfifo("serverpid", FIFO_PERM) == -1 ) { /* create a named pipe */
		if (errno != EEXIST) {
		fprintf(stderr, "[%ld]:failed to create named pipe %s: %s\n",
		(long)getpid(), "pid", strerror(errno));
		return 1;
		}
	}
	remove(FIFONAME);
	serverLog=fopen(FIFONAME,"a");
	fprintf(serverLog, "Time\t     Pid\tDeterminant\n" );
	fclose(serverLog);

	
	
	while(1)
	{
	
	printf("Server pid sini  gonderiyor \n"  );	
    /*client olustumu request okuma*/
	while (((fdserver = open("serverpid", O_WRONLY)) == -1) && (errno == EINTR)) ;
					if (fdserver == -1) {
						fprintf(stderr, "server pid  failed to open named pipe for write:\n");
						return 1;
		}
	 
	 write(fdserver, &serverPid,  sizeof(int));
	 printf("ServerPid %d\n", serverPid );
	 close(fdserver);
	 /*********************end of request*****************************/
			
	printf("Server Pid istek bekliyor \n"  );	
    /*client olustumu request okuma*/
	while (((fdfifoMain = open(argv[3], O_RDONLY)) == -1) && (errno == EINTR)) ;
					if (fdfifoMain == -1) {
						fprintf(stderr, "client pid failed to open named pipe for read:\n");
						return 1;
		}	
	 read(fdfifoMain, bufPid,  sizeof(bufPid));	
	  sizeOfClients++;
	 arrayOfClients[sizeOfClients]=atoi(bufPid);

	 printf("Pid Name= %s*\n", bufPid);
	 close(fdfifoMain);
	 /*********************end of request*****************************/

	printf("Server n gonderiyor \n"  );	
    /*client olustumu request okuma*/
	while (((fdfifoMain = open(argv[3], O_WRONLY)) == -1) && (errno == EINTR)) ;
					if (fdfifoMain == -1) {
						fprintf(stderr, "Size failed to open named pipe for write:\n");
						return 1;
		}
	 
	 write(fdfifoMain, &size,  sizeof(int));
	 close(fdfifoMain);
	 /*********************end of request*****************************/



	 pidchild = fork();
	 if(pidchild==0)	
	{
			
			chilpid=getpid();
			while(1)
			{

			printf("Server matris gonderiyor \n"  );  	

			/*determiant sifir olmayana kadar random matris uret*/
			do{
					detMatr = (double **) malloc(size* sizeof(double *)) ;

	    			for (i = 0 ; i < size ; i++){
	       			detMatr[i] = (double *) malloc(size* sizeof(double)) ;}
	       			matris =(double*) malloc(size*size* sizeof(double)) ;

					srand(time(NULL));
					for(i=0;i<size*size;i++)
					{
							matris[i]=rand() % 10;
						
				 	}	
				 	gettimeofday(&tp, NULL);
				 	ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
				 
				 	convert(matris,detMatr,size);

					for(i=0;i<size;i++)
			    	{
					 	for(j=0;j<size;j++)
					 	{
					  		printf("%.1lf ",detMatr[i][j]);
					 	}
					  		printf("\n");
					}

					det=Determinant(detMatr,size);
					freeArray(detMatr,size);


			    }while(det==0);
				
				/*matris gonderme*/
				while (((fdFifoClient = open(bufPid, O_WRONLY)) == -1) && (errno == EINTR)) ;
						if (fdFifoClient == -1)
						{
							fprintf(stderr, "FifoClient matris gonderme failed to open named pipe for write:\n");
							return 1;
						}  
				write(fdFifoClient, matris, 2*size*size*sizeof(int) );	
				free(matris);
				close(fdFifoClient);
					
				/*log dosyasina yazma*/
				serverLog=fopen(FIFONAME,"a");
				 fprintf(serverLog, "%ld\t%s\t%lf\n",ms, bufPid,det);
				 fclose(serverLog);
				sleep(atoi(argv[1]));
			}	
			//exit(0);
	 }	


	  	
	 	sleep(atoi(argv[1]));

	 }

	while (r_wait(NULL) > 0);

	}
	else
	{
		fprintf(stderr,"/*Usage : TimerServer msec n fifoname  ***/\n");
		return 1;
	}	

	return 0;
}
