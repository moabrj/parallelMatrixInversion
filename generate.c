#include <stdio.h>
#include<stdlib.h>

int dimension;
FILE *file;

double randomDouble() {
    double lowerRange = -1000.0;
    double upperRange = 1000.0;
    return ((double)rand() * (upperRange - lowerRange)) / (double)RAND_MAX + lowerRange;
}

int main(int argc, char *argv[]){
	
	file = fopen("matrix.txt", "w");
	int i, j;
 	
	if(argc <2){
		return 1;
	}
	dimension = atoi(argv[1]);

	for(i=0; i<dimension; i++){
		for(j=0; j<dimension; j++){
			double x=randomDouble();
			fprintf(file,"%.5lf ",x);
		}
		fprintf(file,"\n");
	}
	fclose( file );
 return 0;
 }