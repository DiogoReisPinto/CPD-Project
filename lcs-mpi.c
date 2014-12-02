/* Dynamic Programming implementation of LCS problem */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

#define MASTER 0


short cost(int x);
void createPMatrix(char * stringB,int lenB, char* c,int **pmat,int id,int p);
void createP(int **pmat, char *c, char* stringB, int n,int id,int p);
int getC(char c);
void divideColumsnByProcesses(int* numCols,int* offsetCols,int lenB,int id,int p);
void createLocalMatrix(int** mat, int **pmat,int* numCols,int lenB,int id,int p);
void fillMatrixWithValues(int** mat,int**pmat, int *numCols, int* offsetCols, char* stringA, char*stringB,int lenA, int lenB,int id,int p);
void computeLCSResult(int**mat,int* numCols,int* offsetCols,char* stringA,char* stringB, int lenA,int lenB,int id,int p);


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
	int **pmat = (int**) malloc(sizeof(int *)*27);
	char *c = (char *) malloc(sizeof(char)*27);
	int *numCols = (int*) malloc(sizeof(int)*p);
	int *offsetCols = (int*) malloc(sizeof(int)*p);
	createPMatrix(stringB,lenB,c,pmat,id,p);
	divideColumsnByProcesses(numCols,offsetCols,lenB,id,p);	
	mat = (int **) malloc(sizeof(int *)* (lenA + 1));
	createLocalMatrix(mat,pmat,numCols,lenA,id,p);
	fillMatrixWithValues(mat,pmat,numCols,offsetCols,stringA,stringB,lenA,lenB,id,p);
	MPI_Barrier(MPI_COMM_WORLD);
	computeLCSResult(mat,numCols,offsetCols,stringA,stringB,lenA,lenB,id,p);
	free(mat);
	free(pmat);
	free(c);
  free(numCols);
  free(offsetCols);
}


//Divides matrix by columns using numCols and offsetCols arrays to store information of columns assigned to each process
void divideColumsnByProcesses(int* numCols,int* offsetCols,int lenB,int id,int p){
	int remCols=lenB%p; 
	int sum = 0;
	int i;
	// calculate send counts and displacements
	for (i = 0; i < p; i++) {
		numCols[i] = lenB/p;
		if (remCols > 0) {
			numCols[i]++;
			remCols--;
		}
		offsetCols[i] = sum;
		sum += numCols[i];
	}
}

//Creates the matrix subset that each process will store
void createLocalMatrix(int** mat, int **pmat,int* numCols,int lenA,int id,int p){
	int i,j;
		for(i = 0; i <= lenA; i++) {
			mat[i] = (int *) malloc(sizeof(int)* (numCols[id]+1));	
			mat[i][0] = 0;
		
	}
		
	for(j = 0; j <= numCols[id]; j++) 
		mat[0][j] = 0;
		
}
 
//Computes the whole matrix, making each process store a part of it and colect the processed columns by each process
void fillMatrixWithValues(int** mat,int**pmat, int *numCols, int* offsetCols, char* stringA, char*stringB,int lenA, int lenB,int id,int p){
    int i,j,t,s,w;
    MPI_Status status;
   	int* localBuffer = (int *) malloc(sizeof(int)* (numCols[id]));
	  int* previousLine = (int *) calloc(numCols[id]+offsetCols[id]+1, sizeof(int) );

	for(i = 1; i <= lenA ; i++){
		if(id!=MASTER){
			MPI_Recv(&previousLine[1],  offsetCols[id], MPI_INT, id-1, i, MPI_COMM_WORLD, &status);
			mat[i-1][0] = previousLine[offsetCols[id]];
		}
		if(id != p-1)
			MPI_Ssend(&previousLine[1], offsetCols[id+1], MPI_INT, id+1, i, MPI_COMM_WORLD);
		
		
		for(j = offsetCols[id]+1; j <= offsetCols[id]+numCols[id]; j++) {
			t = (0 - pmat[getC(stringA[i - 1])][j]) < 0 ? 1 : 0;
			s = (0 - (previousLine[j]-t*previousLine[pmat[getC(stringA[i - 1])][j]-1])) < 0 ? 1 : 0;
			if(stringA[i-1]==stringB[j-1])
				cost(i);
			mat[i][j-(offsetCols[id]+1)+1] = previousLine[j] + t*(s ^ 1);
		}	
		
		memcpy(&previousLine[offsetCols[id]+1], &mat[i][1],  sizeof(int)* (numCols[id]));

		}
		mat[lenA][0] = previousLine[offsetCols[id]];	
	free(localBuffer);
	free(previousLine);
}

//Distributed backtracking algorithm for discover the lcs size and string
void computeLCSResult(int**mat,int* numCols,int* offsetCols,char* stringA,char* stringB, int lenA,int lenB,int id,int p){
	
	MPI_Status status;
	int lcsSize;
	if(id==p-1){
		lcsSize = mat[lenA][numCols[id]];
	}

	
	MPI_Bcast(&lcsSize,1,MPI_INT,p-1,MPI_COMM_WORLD);
	int data[2]={0,0};
	char* lcs =(char*)malloc(sizeof(char)* (lcsSize+1)) ;
	int index=lcsSize;
	int i = lenA; 
	int j = numCols[id];
	lcs[index] = '\0';
	if(id!=p-1){ 
		MPI_Recv(&data,2, MPI_INT, id+1, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&lcs[0],lcsSize, MPI_INT, id+1, 1, MPI_COMM_WORLD, &status);
		index = data[1];
		i=data[0];
	}
	while (i > 0 && j > 0)
	{
		if (stringA[i-1] == stringB[offsetCols[id]+j-1])
		{
			printf("%c\n",stringA[i-1]);
			lcs[index-1] = stringA[i-1]; 
			i--; j--; index--;    
			
		}
		else if (mat[i-1][j] > mat[i][j-1])
			i--;
		else
			j--;
	}
	data[0] = i;
	data[1] = index;
	
	if(id != MASTER){
		MPI_Send(&data,2, MPI_INT, id-1, 0, MPI_COMM_WORLD);
		MPI_Send(&lcs[0],lcsSize, MPI_CHAR, id-1, 1, MPI_COMM_WORLD);
	}else{
		printf("%d\n", lcsSize);fflush(stdout);
		for(i=0;i<lcsSize;i++){
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
