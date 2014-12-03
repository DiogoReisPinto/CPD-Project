/* Dynamic Programming implementation of LCS problem */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>
#include <omp.h>

#define MASTER 0

int numBlocks;


short cost(int x);
void createPMatrix(char * stringB,int lenB, char* c,int **pmat,int id,int p);
void createP(int **pmat, char *c, char* stringB, int n,int id,int p);
int getC(char c);
void divideLinesByProcesses(int* numLines, int* offsetLines, int lenA,int id,int p);
void divideColumsnByProcesses(int* numCols,int* offsetCols,int lenB,int id,int p);
void createLocalMatrix(int** mat, int **pmat,int* numLines,int lenB,int id,int p);
void fillMatrixWithValues(int** mat,int**pmat, int *numLines, int *numCols, int* offsetCols,int* offsetLines, char* stringA, char*stringB,int lenA, int lenB,int id,int p);
void computeLCSResult(int**mat,int* numLines,int* offsetLines,char* stringA,char* stringB, int lenA,int lenB,int id,int p);



short cost(int x) {
	int i, n_iter = 20;
	double dcost = 0;
	for(i = 0; i < n_iter; i++)
		dcost += pow(sin((double) x),2) + pow(cos((double) x),2);
	return (short) (dcost / n_iter + 0.1);
}


void createPMatrix(char * stringB,int lenB, char* c,int **pmat,int id,int p){
	int i;
	c[0] = ' ';	
	for(i=1;i<=26;i++){
		c[i]=(char)'A'+i-1;
	}
	for (i = 0; i <= 26; i++) 
		pmat[i] = (int *) malloc(sizeof(int)*(lenB + 1));
	createP(pmat,c,stringB,lenB,id,p);
}


//computes the P[i,j] matrix
void createP(int **pmat, char *c, char* stringB, int n,int id,int p) {
	int i, j;
	
	#pragma omp parallel for private(j) 
	for(i = 0; i <= 26; i++) {

		for(j = 0; j <= n; j++) {
			//eq. 6
			if(j == 0)
				pmat[i][j] = 0;
			else if(stringB[j - 1] == c[i])
				pmat[i][j] = j;
			else pmat[i][j] = pmat[i][j - 1];
			//
		}
	}
}

// gets the number of a char c on the array containing the finite alphabet
int getC(char c) {
	return c-'A'+1;
}

//Master will broadcast the strings and its length and the other processes will receive and store them
void sendStringsToProcesses(char** stringA, char** stringB, int* lenA, int* lenB,int id,int p){

	int strSizes[2] = {*lenA,*lenB};

	MPI_Bcast(strSizes,2,MPI_INT,MASTER, MPI_COMM_WORLD);

	if(id != MASTER){
		*lenA = strSizes[0];
		*lenB = strSizes[1];
		*stringA = (char *) malloc(sizeof(char)*(*lenA+1));
		*stringB = (char *) malloc(sizeof(char)*(*lenB+1));
	}
	
	MPI_Bcast(*stringA,*lenA+1,MPI_CHAR,MASTER, MPI_COMM_WORLD);

	MPI_Bcast(*stringB,*lenB+1,MPI_CHAR,MASTER, MPI_COMM_WORLD);
}

//Main point of project 
void lcs(char* stringA,char* stringB,int lenA,int lenB,int id,int p){
	int **mat;
	int i;
	int **pmat = (int**) malloc(sizeof(int *)*27);
	char *c = (char *) malloc(sizeof(char)*27);
	int *numCols = (int*) malloc(sizeof(int)*numBlocks);
	int *offsetCols = (int*) malloc(sizeof(int)*numBlocks);
	int *numLines = (int*) malloc(sizeof(int)*p);
  int *offsetLines = (int*) malloc(sizeof(int)*p);
	createPMatrix(stringB,lenB,c,pmat,id,p);
	divideLinesByProcesses(numLines,offsetLines,lenA,id,p);
	divideColumsnByProcesses(numCols,offsetCols,lenB,id,p);	
	mat = (int **) malloc(sizeof(int *)* (numLines[id] + 1));
	createLocalMatrix(mat,pmat,numLines,lenB,id,p);
	fillMatrixWithValues(mat,pmat,numLines,numCols,offsetLines,offsetCols,stringA,stringB,lenA,lenB,id,p);
	MPI_Barrier(MPI_COMM_WORLD);
	computeLCSResult(mat,numLines,offsetLines,stringA,stringB,lenA,lenB,id,p);
	
	MPI_Barrier(MPI_COMM_WORLD);
	
	#pragma omp parallel
  {
   		#pragma omp for nowait
			for(i=0;i<numLines[id];i++)
				free(mat[i]);
			
			#pragma omp for
			for(i=0;i<27;i++)
				free(pmat[i]);
	}
	free(mat);
	free(pmat);
	free(c);
  free(numCols);
  free(offsetCols);
	free(numLines);
  free(offsetLines);
}

//Divides matrix by lines using numLines and offsetLines arrays to store information of lines assigned to each process
void divideLinesByProcesses(int* numLines, int* offsetLines,int lenA,int id,int p){
    int remLines=lenA%p; 
    int sum=0;
    int i;
    // calculate send counts and displacements
	
		for (i = 0; i < p; i++) {
	    numLines[i] = lenA/p;
	    if (remLines > 0) {
	        numLines[i]++;
	        remLines--;
	    }
	    offsetLines[i] = sum;
	    sum += numLines[i];
	}
}


//Divides matrix by columns using numCols and offsetCols arrays to store information of columns assigned to each process
void divideColumsnByProcesses(int* numCols,int* offsetCols,int lenB,int id,int p){
	int remCols=lenB%numBlocks; 
	int sum = 0;
	int i;
	#pragma omp parallel for
	for(i = 0; i < numBlocks; i++) {
		numCols[i] = lenB/numBlocks;
		if (remCols > 0) {
			numCols[i]++;
			remCols--;
		}
		offsetCols[i] = sum;
		sum += numCols[i];
	}
}

//Creates the matrix subset that each process will store
void createLocalMatrix(int** mat, int **pmat,int* numLines,int lenB,int id,int p){
	int i,j;
	#pragma omp parallel
	{
		if(id == MASTER){
			#pragma omp for
			for(i = 0; i <= numLines[id]; i++) {
				mat[i] = (int *) malloc(sizeof(int)* lenB+1);	
				mat[i][0] = 0;
			}
			#pragma omp for
			for(j = 0; j <= lenB; j++) 
				mat[0][j] = 0;
			}
		else{
			#pragma omp for
			for(i = 0; i <= numLines[id]; i++) {
				mat[i] = (int *) malloc(sizeof(int)* lenB+1);	
			}
			#pragma omp for
			for(j = 0; j <= lenB; j++) 
				mat[0][j] = 0;
		}
	}
}
 
//Computes the whole matrix, making each process store a part of it and colect the processed columns by each process
void fillMatrixWithValues(int** mat,int**pmat, int *numLines, int *numCols, int* offsetLines,int* offsetCols, char* stringA, char*stringB,int lenA, int lenB,int id,int p){
	int i,n,j,t,s;
	MPI_Status status;
	int* previousLine = (int *) calloc(lenB+1, sizeof(int) );
	
	for(n = 0; n < numBlocks ; n++){
		if(id!=MASTER){
			MPI_Recv(&mat[0][offsetCols[n]+1], numCols[n], MPI_INT, id-1, n, MPI_COMM_WORLD, &status);
		}
		
		
		for(i=1;i<numLines[id]+1;i++){
			#pragma omp parallel for private(s,t)
			for(j=offsetCols[n]+1;j<=offsetCols[n]+numCols[n];j++){
				t = (0 - pmat[getC(stringA[offsetLines[id]+i - 1])][j]) < 0 ? 1 : 0;
				s = (0 - (mat[i-1][j]-t*mat[i-1][pmat[getC(stringA[offsetLines[id]+i - 1])][j]-1])) < 0 ? 1 : 0;
				if(stringA[offsetLines[id]+i-1]==stringB[j-1])
					cost(i);
				mat[i][j] = mat[i-1][j] + t*(s ^ 1);
			}
		}
		if(id != p-1){
			MPI_Ssend(&mat[numLines[id]][offsetCols[n]+1], numCols[n], MPI_INT, id+1, n, MPI_COMM_WORLD);
			
		}
	}
}

//Distributed backtracking algorithm for discover the lcs size and string
void computeLCSResult(int**mat,int* numLines,int* offsetLines,char* stringA,char* stringB, int lenA,int lenB,int id,int p){
	
	MPI_Status status;
	int lcsSize;
	if(id==p-1){
		lcsSize = mat[numLines[id]][lenB]+1;
	}
	MPI_Bcast(&lcsSize,1,MPI_INT,p-1,MPI_COMM_WORLD);
	int data[2]={0,0};
	char* lcs =(char*)malloc(sizeof(char)* (lcsSize+1)) ;
	int index=lcsSize;
	int i = numLines[id]; 
	int j = lenB;
	lcs[index] = '\0';
	if(id!=p-1){ 
		MPI_Recv(&data,2, MPI_INT, id+1, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&lcs[0],lcsSize, MPI_INT, id+1, 1, MPI_COMM_WORLD, &status);
		index = data[1];
		j=data[0];
	}
	
	while (i > 0 && j > 0)
	{
		if (stringA[offsetLines[id]+(i-1)] == stringB[j-1])
		{
			
			lcs[index-1] = stringA[offsetLines[id]+(i-1)]; 
			i--; j--; index--;    
			
		}
		else if (mat[i-1][j] > mat[i][j-1])
			i--;
		else
			j--;
	}
	data[0] = j;
	data[1] = index;
	if(id != MASTER){
		MPI_Send(&data,2, MPI_INT, id-1, 0, MPI_COMM_WORLD);
		MPI_Send(&lcs[0],lcsSize, MPI_CHAR, id-1, 1, MPI_COMM_WORLD);
	}else{
		printf("%d\n", lcsSize-1);fflush(stdout);
		for(i=1;i<lcsSize;i++){
			printf("%c",lcs[i]);fflush(stdout);
		}
		printf("\n");fflush(stdout);
	}
	
	free(lcs);
	
}


int main(int argc, char *argv[]){
	double start, end;
	int lenA=0;
	int lenB=0;
	int id, p;
 	char *stringA;
	char *stringB;
	if(argc!=2)
    	return 0;
    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &p);
    MPI_Barrier (MPI_COMM_WORLD);
    start = MPI_Wtime();
	if(id == MASTER){
		FILE *file = fopen(argv[1], "r" );
		if ( file == 0 ){
		    printf( "Could not open file\n" );
		    return 0;
		}else{
		 	fscanf(file,"%d %d\n",&lenA,&lenB);
		 	stringA = (char *) malloc(sizeof(char)*(lenA+1));
			stringB = (char *) malloc(sizeof(char)*(lenB+1));
		 	fscanf(file,"%s\n",stringA);
		 	fscanf(file,"%s\n",stringB);
		 	fclose( file );
		}	
	}
	sendStringsToProcesses(&stringA,&stringB,&lenA,&lenB,id,p);
	if(0.01*lenB<10)
			numBlocks=4;
		else
			numBlocks=((((0.01*lenB)/4)*p)/4);
	lcs(stringA,stringB,lenA,lenB,id,p);
	end = MPI_Wtime();
	MPI_Finalize();
	if (id == MASTER) { 
    printf("Runtime = %f\n", end-start);
}
	free(stringA);
	free(stringB);
	return 0;
}
