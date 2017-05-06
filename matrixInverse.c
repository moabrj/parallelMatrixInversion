#include<stdio.h>
#include<stdlib.h>

double **augmentedmatrix;
double temporary, r;
int i, j, k, dimension, temp; 
FILE *file;
FILE *fileOut;

void makeMatrix() {
	int i;
    augmentedmatrix = (double **)malloc(sizeof(double)*dimension);
    for(i = 0; i < dimension; i++) {         
        augmentedmatrix[i] = (double *)malloc(sizeof(double)*dimension*2);     
    }
}

void read(){
	for(i=0; i<dimension; i++)
		for(j=0; j<dimension; j++)
			fscanf(file,"%lf",&augmentedmatrix[i][j]);
}

void write(){
	for(i=0; i<dimension; i++){
		for(j=dimension; j<2*dimension; j++)
			fprintf(fileOut,"%.5lf ",augmentedmatrix[i][j]);
		fprintf(fileOut,"\n");
	}
}

void augmentingmatrix(){
	for(i=0;i<dimension; i++)
		for(j=dimension; j<2*dimension; j++)
			if(i==j%dimension) augmentedmatrix[i][j]=1;
			else augmentedmatrix[i][j]=0;
}

void findPivo(){
	temp=j;
	for(i=j+1; i<dimension; i++)
		if(augmentedmatrix[i][j]>augmentedmatrix[temp][j])
			temp=i;
}

void swapLine(){
	if(temp!=j)
		for(k=0; k<2*dimension; k++){
			temporary=augmentedmatrix[j][k];
			augmentedmatrix[j][k]=augmentedmatrix[temp][k];
			augmentedmatrix[temp][k]=temporary;
	}
}

void calcInverse(){
	for(i=0; i<dimension; i++)
		if(i!=j){
			r=augmentedmatrix[i][j];
			for(k=0; k<2*dimension; k++)
				augmentedmatrix[i][k]-=(augmentedmatrix[j][k]/augmentedmatrix[j][j])*r;
		}else {
			r=augmentedmatrix[i][j];
			for(k=0; k<2*dimension; k++)
				augmentedmatrix[i][k]/=r;
		}
}

int main(int argc, char *argv[]){
	
	file = fopen("matrix.txt", "r");
	fileOut = fopen("inverse.txt", "w");
 	
	if(argc <2){
		return 1;
	}
	
	dimension = atoi(argv[1]);
	makeMatrix();
	read();
	augmentingmatrix();

	for(j=0; j<dimension; j++){
		findPivo();
		swapLine();
		calcInverse();
	}
	
	write();
	fclose( file );
	fclose( fileOut );
 return 0;
 }
