/* Dynamic Programming implementation of LCS problem */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

#define MASTER 0

int max(int a, int b);
short cost(int x);
void createP(int **p, char *c, char* stringB, int n);
int getC(char c);
void fillMatrixWithValues(int** mat,int **p,char* stringA,char* stringB,int m,int n);
void computeLCSResult(int** mat,char* stringA,char* stringB,int m,int n);




/* Computes de LCS and prints its length and value to the output 
void lcs( char *stringA, char *stringB, int m, int n ){
	int **mat = (int **) malloc(sizeof(int *)* (m + 1));
	int **p = (int**) malloc(sizeof(int *)*27);
	int i=0;
	int j, s, t;
	char *c = (char *) malloc(sizeof(char)*27);
	c[0] = ' ';	
	for(i=1;i<=26;i++){
		c[i]=(char)'A'+i-1;
	}
	for(i = 0; i <= m; i++) {
		mat[i] = (int *) malloc(sizeof(int)* (n + 1));		
		mat[i][0] = 0;
	}

	for(i = 0; i <= n; i++) 
		mat[0][i] = 0;

	for (i = 0; i <= 26; i++) 
		p[i] = (int *) malloc(sizeof(int)*(n + 1));
	
	createP(p,c,stringB,n);
	fillMatrixWithValues(mat,p,stringA,stringB,m,n);
   	computeLCSResult(mat,stringA,stringB,m,n);

	for(i = 0; i <= m; i++)
		free(mat[i]);
	
	free(mat);

	for(i = 0; i <= 4; i++)	
		free(p[i]);

	free(p);
	free(c);	
}
*/
//computes the P[i,j] matrix
void createP(int **p, char *c, char* stringB, int n) {
	int i, j;

	for(i = 0; i <= 26; i++) {

		for(j = 0; j <= n; j++) {
			//eq. 6
			if(j == 0)
				p[i][j] = 0;
			else if(stringB[j - 1] == c[i])
				p[i][j] = j;
			else p[i][j] = p[i][j - 1];
			//
		}
	}
}

// gets the number of a char c on the array containing the finite alphabet
int getC(char c) {
	return c-'A'+1;
}

/*Fills the matrix with the matching pattern then used to compute the LCS*/
void fillMatrixWithValues(int** mat, int **p,char* stringA,char* stringB, int lenA, int lenB){
	int i, j, s, t;

	for(i = 1; i <= lenA; i++) {
		for(j = 1; j <= lenB; j++) {
			//eq. 7
			t = (0 - p[getC(stringA[i - 1])][j]) < 0 ? 1 : 0;
			s = (0 - (mat[i - 1][j]-t*mat[i - 1][p[getC(stringA[i - 1])][j]-1])) < 0 ? 1 : 0;
			if(stringA[i-1]==stringB[j-1])
				cost(i);
			mat[i][j] = mat[i - 1][j] + t*(s ^ 1);
			//
		}
	}
}





short cost(int x) {
	int i, n_iter = 20;
	double dcost = 0;
	for(i = 0; i < n_iter; i++)
		dcost += pow(sin((double) x),2) + pow(cos((double) x),2);
	return (short) (dcost / n_iter + 0.1);
}



int max(int a, int b)
{
    return (a > b)? a : b;
}
  


int main(int argc, char *argv[])
{
	MPI_Status status;
	int id, p, lineReceiver;
	int remLines;
	int lenA=0;
	int lenB=0;
	int strSizes[2] = {0};
 	char *stringA;
	char *stringB;
	int sum = 0;
    int *numLines;   
    int *offsetLines; 
    int colimit;
    int **mat;
    int t,s;
    int remCols;
    int *offsetCols;
    int *numCols;
    int *globalBuffer;
    int *localBuffer;
    int *previousLine;
    int lcsSize;

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
		}else{
		 	fscanf(file,"%d %d\n",&lenA,&lenB);
		 	stringA = (char *) malloc(sizeof(char)*(lenA+1));
			stringB = (char *) malloc(sizeof(char)*(lenB+1));
		 	fscanf(file,"%s\n",stringA);
		 	fscanf(file,"%s\n",stringB);
		 	fclose( file );
		 	strSizes[0] = lenA;
		 	strSizes[1] = lenB;

		}

	}

	MPI_Bcast(&strSizes,2,MPI_INT,MASTER, MPI_COMM_WORLD);
	if(id != MASTER){
		lenA = strSizes[0];
		lenB = strSizes[1];
		stringA = (char *) malloc(sizeof(char)*(lenA+1));
		stringB = (char *) malloc(sizeof(char)*(lenB+1));
	}

	MPI_Bcast(stringA,lenA+1,MPI_CHAR,MASTER, MPI_COMM_WORLD);
	MPI_Bcast(stringB,lenB+1,MPI_CHAR,MASTER, MPI_COMM_WORLD);
	
	//##############################################################
	int i=0;
	int j =0;
	int **pmat = (int**) malloc(sizeof(int *)*27);
	char *c = (char *) malloc(sizeof(char)*27);
	c[0] = ' ';	

	for(i=1;i<=26;i++){
		c[i]=(char)'A'+i-1;
	}
	for (i = 0; i <= 26; i++) 
		pmat[i] = (int *) malloc(sizeof(int)*(lenB + 1));
	
	createP(pmat,c,stringB,lenB);

	//################################PARA DIVIDIR QUE PROCESSOS FICAM COM QUE LINHAS################################
    remLines=lenA%p; 

    numLines = malloc(sizeof(int)*p);
    offsetLines = malloc(sizeof(int)*p);

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
    //########################### PARA DIVIDIR AS CELULAS EM COLUNAS PARA CADA PROCESSO #####################################
 
    remCols=lenB%p; 

    numCols = malloc(sizeof(int)*p);
    offsetCols = malloc(sizeof(int)*p);
    sum = 0;
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

    //############################ MATRIX PARA GUARADA DADOS EM CADA PC ####################################

	mat = (int **) malloc(sizeof(int *)* (numLines[id] + 1));

	if(id == MASTER){
		for(i = 0; i <= numLines[id]; i++) {
			mat[i] = (int *) malloc(sizeof(int)* lenB+1);	
			mat[i][0] = 0;
		}
		for(j = 0; j <= lenB; j++) 
			mat[0][j] = 0;
	}else{
		for(i = 0; i <= numLines[id]; i++) {
			mat[i] = (int *) malloc(sizeof(int)* lenB+1);	
		}
		for(j = 0; j <= lenB; j++) 
			mat[0][j] = 0;
	}

    //################################################################
    globalBuffer = (int *) calloc(lenB+1,sizeof(int));
	localBuffer = (int *) malloc(sizeof(int)* (numCols[id]));
	previousLine = (int *) malloc(sizeof(int)* (lenB+1));
	lineReceiver = 0;
	for(i = 1; i <= lenA ; i++){

		if(id == lineReceiver)
			memcpy(previousLine, &mat[i-offsetLines[id]-1][0],  sizeof(int)* (lenB+1));
		
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

    MPI_Barrier(MPI_COMM_WORLD);

    if(id==p-1){
		lcsSize = mat[numLines[id]][lenB]+1;
	}

	MPI_Bcast(&lcsSize,1,MPI_INT,p-1,MPI_COMM_WORLD);
	int data[2]={0,0};
	char* lcs =(char*)malloc(sizeof(char)* (lcsSize+1)) ;
	int index=lcsSize;
    i = numLines[id]; 
    j = lenB;
	
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
		printf("%d\n", lcsSize-1);
		for(i=0;i<lcsSize;i++){
			printf("%c",lcs[i]);
		}
		printf("\n");
   	}

   	MPI_Finalize();
  	return 0;
}
