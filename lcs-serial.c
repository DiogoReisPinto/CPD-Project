/* Dynamic Programming implementation of LCS problem */
#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <math.h>
  

  
  
int max(int a, int b);
short cost(int x);
void fillMatrixWithValues(int** mat,char* stringA,char* stringB,int m,int n);
void computeLCSResult(int** mat,char* stringA,char* stringB,int m,int n);




/* Computes de LCS and prints its length and value to the output */
void lcs( char *stringA, char *stringB, int m, int n )
{
   int **mat = (int **) malloc(sizeof(int *)* (m + 1));
   int i=0;

   for(i = 0; i <= m; i++)
      mat[i] = (int *) malloc(sizeof(int)* (n + 1));

   fillMatrixWithValues(mat,stringA,stringB,m,n);
   computeLCSResult(mat,stringA,stringB,m,n); 

   for(i = 0; i <= m; i++)
      free(mat[i]);
   free(mat);
}



/*Fills the matrix with the matching pattern then used to compute the LCS*/
void fillMatrixWithValues(int** mat,char* stringA,char* stringB, int lenA, int lenB){
   int i=0;
   int j=0;
   for (i=0; i <= lenA; i++)
   {
     for (j=0; j <= lenB; j++)
     {
       if (i == 0 || j == 0)
         mat[i][j] = 0;
       else if (stringA[i-1] == stringB[j-1]){
          mat[i][j] = mat[i-1][j-1] + 1;
          //cost(i); 
        }
       else
         mat[i][j] = max(mat[i-1][j], mat[i][j-1]);
     }
   }
}



/*Computes the LCS with the given matrix filled with the matching pattern*/
void computeLCSResult(int** mat,char* stringA,char* stringB, int lenA, int lenB){
   int index = mat[lenA][lenB];

   char* lcs =(char*)malloc(sizeof(char)*(index+1)) ;
   lcs[index] = '\0'; 
   int i = lenA; 
   int j = lenB;
   while (i > 0 && j > 0)
   {
      if (stringA[i-1] == stringB[j-1])
      {
          lcs[index-1] = stringA[i-1]; 
          i--; j--; index--;    
      }
      else if (mat[i-1][j] > mat[i][j-1])
         i--;
      else
         j--;
   }
 
   int lcsSize = strlen(lcs);
   // Print the size
   printf("%d\n",lcsSize);

   // Print the lcs
   printf("%s\n",lcs);
   free(lcs);
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
  if(argc!=2)
    return 0;

  FILE *file = fopen(argv[1], "r" );

  if ( file == 0 )
  {
      printf( "Could not open file\n" );
  }
  else 
  {
    int lenA=0;
    int lenB=0;
	 fscanf(file,"%d %d\n",&lenA,&lenB);
	 char *stringA = (char *) malloc(sizeof(char)*(lenA+1));
    char *stringB = (char *) malloc(sizeof(char)*(lenB+1));
	 fscanf(file,"%s\n",stringA);
	 fscanf(file,"%s\n",stringB);
	 fclose( file );
    lcs( stringA, stringB, lenA, lenB );
	 free(stringA);
    free(stringB);
  }
  return 0;
}
