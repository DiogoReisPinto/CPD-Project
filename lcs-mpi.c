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
	int *numLines = (int*) malloc(sizeof(int)*p);
    int *offsetLines = (int*) malloc(sizeof(int)*p);
    int *numCols = (int*) malloc(sizeof(int)*p);
    int *offsetCols = (int*) malloc(sizeof(int)*p);
	createPMatrix(stringB,lenB,c,pmat,id,p);
	divideLinesByProcesses(numLines,offsetLines,lenA,id,p);
	divideColumsnByProcesses(numCols,offsetCols,lenB,id,p);
	mat = (int **) malloc(sizeof(int *)* (numLines[id] + 1));
	createLocalMatrix(mat,pmat,numLines,lenB,id,p);
	fillMatrixWithValues(mat,pmat,numLines,numCols,offsetCols,offsetLines,stringA,stringB,lenA,lenB,id,p);
	MPI_Barrier(MPI_COMM_WORLD);
	computeLCSResult(mat,numLines,offsetLines,stringA,stringB,lenA,lenB,id,p);
	
	
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
void createLocalMatrix(int** mat, int **pmat,int* numLines,int lenB,int id,int p){
	int i,j;
	if(id == MASTER){
		for(i = 0; i <= numLines[id]; i++) {
			mat[i] = (int *) malloc(sizeof(int)* lenB+1);	
			mat[i][0] = 0;
		}
		for(j = 0; j <= lenB; j++) 
			mat[0][j] = 0;
	}
	else{
		for(i = 0; i <= numLines[id]; i++) {
			mat[i] = (int *) malloc(sizeof(int)* lenB+1);	
		}
		for(j = 0; j <= lenB; j++) 
			mat[0][j] = 0;
	}


}

//Computes the whole matrix, making each process store a part of it and colect the processed columns by each process
void fillMatrixWithValues(int** mat,int**pmat, int *numLines, int *numCols, int* offsetCols,int* offsetLines, char* stringA, char*stringB,int lenA, int lenB,int id,int p){
    int i,j,t,s;

    MPI_Status status;
    int* globalBuffer = (int *) calloc(lenB+1,sizeof(int));
	int* localBuffer = (int *) malloc(sizeof(int)* (numCols[id]));
	int* previousLine = (int *) malloc(sizeof(int)* (lenB+1));
	int lineReceiver = 0;
	
	for(i = 1; i <= lenA ; i++){
		if(id == lineReceiver){
			memcpy(previousLine, &mat[i-offsetLines[id]-1][0],  sizeof(int)* (lenB+1));
		}
		MPI_Bcast(previousLine,lenB+1,MPI_INT,lineReceiver,MPI_COMM_WORLD);
		for(j = offsetCols[id]+1; j <= offsetCols[id]+numCols[id]; j++) {
			t = (0 - pmat[getC(stringA[i - 1])][j]) < 0 ? 1 : 0;
			s = (0 - (previousLine[j]-t*previousLine[pmat[getC(stringA[i - 1])][j]-1])) < 0 ? 1 : 0;
			if(stringA[i-1]==stringB[j-1])
				cost(i);
			localBuffer[j-(offsetCols[id]+1)] = previousLine[j] + t*(s ^ 1);
		}	
		MPI_Gatherv(localBuffer,numCols[id],MPI_INT,&globalBuffer[1],numCols,offsetCols,MPI_INT,lineReceiver,MPI_COMM_WORLD);
		if(id == lineReceiver)
			memcpy(&mat[i-offsetLines[id]][0], globalBuffer,  sizeof(int)* (lenB+1));
		if(i == (offsetLines[lineReceiver] + numLines[lineReceiver]) && lineReceiver<p-1){
			if(id == lineReceiver){
				MPI_Send(&mat[i-offsetLines[id]][0], lenB+1, MPI_INT, lineReceiver+1, lineReceiver, MPI_COMM_WORLD);
			}else if(id == lineReceiver+1){
				MPI_Recv(&mat[0][0], lenB+1, MPI_INT, lineReceiver, lineReceiver, MPI_COMM_WORLD, &status);
			}
			lineReceiver++;
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
}


int main(int argc, char *argv[]){
	
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
	MPI_Finalize();
	return 0;
}
