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


int pidShow=0;
int serverPid=0;
char result[256];

void sigHandler(int signalNo)
{
	FILE *output;
	char bufPid[15];
	
	time_t t;
    time(&t);

	unlink("res1");
	unlink("serverpid");
	unlink("ShowResult");
	output=fopen("log/clientLog.log","a");
	fprintf(output, "CTRL + C sinayli geldi %s\n",ctime(&t) );
	fclose(output);

	kill((pid_t)pidShow, SIGINT);
	kill((pid_t)serverPid, SIGINT);
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
void adjoint(double **A,double **adj, int n)
{
	double **temp; // temp is used to store cofactors of A[][]
	int sign = 1, i, j;

	temp = (double**)malloc(n*sizeof(double*));

    for (i = 0; i < n; ++i)
    {
        temp[i] = (double*)malloc(n*sizeof(double));
    }

    if (n == 1)
    {
        adj[0][0] = 1;
        return;
    }
 
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            // Get cofactor of A[i][j]
            cofactor(A, temp, i, j, n);
 
            // sign of adj[j][i] positive if sum of row
            // and column indexes is even.
            sign = ((i+j)%2==0)? 1: -1;
 
            // Interchanging rows and columns to get the
            // transpose of the cofactor matrix
            adj[j][i] = (sign)*(Determinant(temp, n-1));
        }
    }
     freeArray(temp,n);
}
/*kitaptan alindi*/
pid_t r_wait(int *stat_loc) {
   int retval;
   while (((retval = wait(stat_loc)) == -1) && (errno == EINTR)) ;
   return retval;
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

void invert(double **m, int s ,int e, int size)
{
	double **matrix,**inverse, **adj;
	double det;
	int i,j,n=size/2,tmp=e;


	
	matrix = (double**)malloc(n*sizeof(double*));
	inverse = (double**)malloc(n*sizeof(double*));
	adj = (double**)malloc(n*sizeof(double*));

    for (i = 0; i < n; ++i)
    {
        matrix[i] = (double*)malloc(n*sizeof(double));
        inverse[i] = (double*)malloc(n*sizeof(double));
        adj[i] = (double*)malloc(n*sizeof(double));
    }

    for (i = 0; i < n; ++i)
    {
    	 for (j = 0; j < n; ++j)
    	{
    	matrix[i][j]=m[s][e];
    	e++;
    	}
    	e=tmp;
    	s++;
    }

  
    det = Determinant(matrix,n);

    adjoint(matrix,adj,n);

    for (i = 0; i < n; ++i)
	{
		for (j = 0; j < n; ++j)
		{
			inverse[i][j] = adj[i][j]/det;
		}
	}
	for (i = 0; i < n; ++i)
    {
    	 for (j = 0; j < n; ++j)
    	{
    	m[s][e]=inverse[i][j];
    	e++;
    	}
    	e=tmp;
    	s++;
    }
    freeArray(matrix,n);
    freeArray(inverse,n);
    freeArray(adj,n);

}
int main(int argc, char *argv[]) {

	FILE *clientLog;
	double result1,result2,detOriginal=0,detshifted=0;
	int i,j,size=0,n=0;
	int fdfifoMain,fdFifoClient,fdClientShow,fdres1,fdres2,fdserver;
	char bufPid[8];
	double *matris;
	double **detMatr;
	char fifoClientShow[10]="ShowResult";
	char serverClientFifo[3]="pid";
	pid_t pidRes1, pidRes2;
	struct timeval start, stop;
	double secs = 0;

	
	if(argc==2){

	initializeSignals();
	
	sprintf(bufPid, "%d", (int)getpid());
	remove("log/clientLog.log");

			/*serverden matrsi okumak icin*/
	if (mkfifo(bufPid, FIFO_PERM) == -1) 
	{ 
		if (errno != EEXIST) 
		{
		fprintf(stderr, "[%ld]:failed to create named pipe %s: %s\n",
		(long)getpid(), bufPid, strerror(errno));
		return 1;
		}
	 }


	printf("Client serverpid sini  bekliyor \n"  );	
    /*client olustumu request okuma*/
	while (((fdserver = open("serverpid", O_RDONLY)) == -1) && (errno == EINTR)) ;
					if (fdserver == -1) {
						fprintf(stderr, "server pid FiFoMain failed to open named pipe for read:\n");
						return 1;
		}
	 
	 read(fdserver, &serverPid,  sizeof(int));
	 printf("ServerPid %d\n", serverPid );
	 close(fdserver);
	 /*********************end of request*****************************/

	 	printf("Client Showa  server Pid sini gonderiyor \n"  );	
		while (((fdClientShow = open("showserverpid", O_WRONLY)) == -1) && (errno == EINTR)) ;
						if (fdClientShow == -1) {
							fprintf(stderr, "FiFoMain failed to open named pipe for read:\n");
							return 1;
			}	
	 	write(fdClientShow, &serverPid,  sizeof(int));	
	 	close(fdClientShow);
	 	/*********************end of getPid*****************************/

	 printf("Client Showun Pid sini bekliyor \n"  );	
		while (((fdClientShow = open(fifoClientShow, O_RDONLY)) == -1) && (errno == EINTR)) ;
						if (fdClientShow == -1) {
							fprintf(stderr, "FiFoMain failed to open named pipe for read:\n");
							return 1;
			}	
	 	read(fdClientShow, &pidShow,  sizeof(int));	
	 	printf("Show Pid Name= %d*\n", pidShow);
	 	close(fdClientShow);
	 	/*********************end of getPid*****************************/

	printf("Client ShowResulta Pid gonderiyor \n"  );
	while (((fdClientShow = open(fifoClientShow, O_WRONLY)) == -1) && (errno == EINTR)) ;
					if (fdClientShow == -1) {
						fprintf(stderr, "FifoShowResult failed to open named pipe for write:\n");
						return 1;
					}	

	write(fdClientShow,bufPid,sizeof(bufPid));
	close(fdClientShow);
	//usleep(5);
	/*********************end of request*****************************/	
	
		

	printf("Client Servera Pid istek gonderiyor \n"  );
	/*servere request gonderme*/
	while (((fdfifoMain = open(argv[1], O_WRONLY)) == -1) && (errno == EINTR)) ;
					if (fdfifoMain == -1) {
						fprintf(stderr, "FifoMain failed to open named pipe for write:\n");
						return 1;
					}	

	printf("Pid Name= %s*\n", bufPid);
	write(fdfifoMain,bufPid,sizeof(bufPid));
	close(fdfifoMain);
	/*********************end of request*****************************/


	printf("Client n bekliyor \n"  );	
    /*client olustumu request okuma*/
	while (((fdfifoMain = open(argv[1], O_RDONLY)) == -1) && (errno == EINTR)) ;
					if (fdfifoMain == -1) {
						fprintf(stderr, "Size FiFoMain failed to open named pipe for read:\n");
						return 1;
		}	
	 read(fdfifoMain, &size,  sizeof(int));
	 printf("Gelen size %d\n", size );
	 close(fdfifoMain);
	 /*********************end of request*****************************/




		while(1)
	  {

	  	detMatr = (double **) malloc(size* sizeof(double *)) ;

	   for (i = 0 ; i < size ; i++){
	    detMatr[i] = (double *) malloc(size* sizeof(double)) ;}
	    matris =(double*) malloc(size*size* sizeof(double)) ;

		printf("Client matris bekliyor \n"  ); 		
		while (((fdFifoClient = open(bufPid, O_RDONLY)) == -1) && (errno == EINTR)) ;
						if (fdFifoClient == -1) {
							fprintf(stderr, "FifoClient failed to open named pipe for read:\n");
							return 1;
					}	
		gettimeofday(&start, NULL);		
		read(fdFifoClient, matris, 2*size*size*sizeof(int));
		
		close(fdFifoClient);
		/*********************end of matris okuma*****************************/

		convert(matris, detMatr ,size);
		
		clientLog=fopen("log/clientLog.log","a");
			 
		fprintf(clientLog, "Original A = [");		 
		for(i=0;i<size;i++)
		  	{
			 	for(j=0;j<size;j++)
			 	{
			  	printf("%.1lf ",detMatr[i][j]);
			  	fprintf(clientLog, "%.1lf , ",detMatr[i][j]);
			  	}
			  	fprintf(clientLog, ";");
			  	printf("\n");
				} 	
		fprintf(clientLog, " ]\n");		
		fclose(clientLog);
		 detOriginal=Determinant(detMatr,size);
		invert(detMatr, 0 , 0 ,size);
		 invert(detMatr, 0 , size/2, size);
		 //invert(detMatr, size/2 , 0, size);
		 //invert(detMatr, size/2 , size/2 , size);
		 detshifted=Determinant(detMatr,size);
		 result1=detOriginal-detshifted;	
		 sprintf(result,"%s\t%.4lf\t%lf",bufPid,result1,secs);
		 gettimeofday(&stop, NULL);
		secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
		clientLog=fopen("log/clientLog.log","a");
		fprintf(clientLog, "Shifted A = [");		 
		for(i=0;i<size;i++)
		  	{
			 	for(j=0;j<size;j++)
			 	{
			  	
			  	fprintf(clientLog, "%.1lf , ",detMatr[i][j]);
			  	}
			  	fprintf(clientLog, ";");
			  	
				} 	
		fprintf(clientLog, " ]\n");		
		fclose(clientLog);
		 freeArray(detMatr,size);
		 free(matris);


		pidRes1=fork();
		if(pidRes1==0)
		{
			printf("Client ShowResulta shifted result gonderiyor \n"  );
			while (((fdres1 = open("res1", O_WRONLY)) == -1) && (errno == EINTR)) ;
					if (fdres1 == -1) {
						fprintf(stderr, "FifoShowResult1 failed to open named pipe for write:\n");
						return 1;
					}	
					
			write(fdres1,result,sizeof(result));
			close(fdres1);
		
			
			exit(0);
		}

		while (r_wait(NULL) > 0);
		
	   }
	}
	else
	{
		fprintf(stderr,"/*Usage : SeeWhat fifoname  ***/\n");
		return 1;
	}
	return 0;
}

