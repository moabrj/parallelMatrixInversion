#include<stdio.h>
#include<stdlib.h>
#include <time.h>
#include <mpi.h>

double **augmentedmatrix;
double r;
int i, j, k, dimension, temp; 
FILE *file;
FILE *fileOut;
int rank, nprocs;
clock_t inicio_fim;
MPI_Status status;

/*Aloca o espaço de memória para armazenar a matriz aumentada*/
void makeMatrix() {
	int i;
    augmentedmatrix = (double **)malloc(sizeof(double)*dimension);
    for(i = 0; i < dimension; i++) {         
        augmentedmatrix[i] = (double *)malloc(sizeof(double)*dimension*2); 
    }
}

/*Faz a leitura da mariz a partir do arquivo de entrada*/
void read(){
	for(i=0; i<dimension; i++)
		for(j=0; j<dimension; j++)
			fscanf(file,"%lf",&augmentedmatrix[i][j]);
}

/*Escreve a matriz inversa no arquivo de saida*/
void write(){
	for(i=0; i<dimension; i++){
		for(j=dimension; j<2*dimension; j++)
			fprintf(fileOut,"%.5lf ",augmentedmatrix[i][j]);
		fprintf(fileOut,"\n");
	}
	fprintf(fileOut,"inicio_fim:%d\n", inicio_fim);
}

/*Gera os valores da matriz aumentada adicionando a matriz identidade no fim da matriz de entrada*/
void augmentingmatrix(){
	for(i=0;i<dimension; i++)
		for(j=dimension; j<2*dimension; j++)
			if(i==j%dimension) augmentedmatrix[i][j]=1;
			else augmentedmatrix[i][j]=0;
}

/*Encontra o proximo elemento que vai ser utilizado como pivô*/
void findPivo(){
	temp=j;
	for(i=j+1; i<dimension; i++)
		if(augmentedmatrix[i][j]>augmentedmatrix[temp][j])
			temp=i;
}

/*Realiza a troca de linhas se o pivô não pertencer a linha atual*/
void swapLine(){
	if(temp!=j)
		for(k=0; k<2*dimension; k++){
			double temporary=augmentedmatrix[j][k];
			augmentedmatrix[j][k]=augmentedmatrix[temp][k];
			augmentedmatrix[temp][k]=temporary;
	}
}

/*Realiza o calculo dos novos valores para cada linha da matriz aumentada, gerando a matriz inversa*/
void calcInverse(){
    MPI_Bcast (augmentedmatrix,dimension*dimension*2,MPI_DOUBLE,0,MPI_COMM_WORLD);    	
    for(i=0; i<dimension; i++){
		if(i!=j){ //verifica se é a linha atual.
			r=augmentedmatrix[i][j];
			for(k=0; k<2*dimension; k++){
				augmentedmatrix[i][k]-=(augmentedmatrix[j][k]/augmentedmatrix[j][j])*r; //calcula o novo valor para as linhas diferentes da atual.
			}
		}else {
			r=augmentedmatrix[i][j];
			for(k=0; k<2*dimension; k++){
				augmentedmatrix[i][k]/=r; //divide os elementos da linha atual pelo pivô.
			}
		}
	}
}

/*Função main. Recebe como parametro a dimensão da matriz*/
int main(int argc, char *argv[]){
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);   /* get current process id */
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs); /* get number of processes */
	
	if(argc <2){
		return 1;
	}
	
	if (rank==0){
	    dimension = atoi(argv[1]);
	    file = fopen("matrix.txt", "r");
	    fileOut = fopen("inverse.txt", "w");
	    makeMatrix();
	    read();
	    augmentingmatrix();
        
        inicio_fim = clock();
    }
	MPI_Bcast (&dimension, 1, MPI_INT, 0, MPI_COMM_WORLD); 
	MPI_Bcast (&r, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);   
        MPI_Bcast (&temp, 1, MPI_INT, 0, MPI_COMM_WORLD);
	 MPI_Bcast (augmentedmatrix,dimension*dimension*2,MPI_DOUBLE,0,MPI_COMM_WORLD);
	if(rank != 0){
	    makeMatrix();
	}
	//MPI_Bcast (augmentedmatrix,dimension*dimension*2,MPI_DOUBLE,0,MPI_COMM_WORLD);
	if(rank == 0){
	    for(j=0; j<dimension; j++){   
	        findPivo();
	        swapLine();
	    }
	}
	for(j=0; j<dimension; j++){
	    calcInverse();
	}
	
	if(rank==0){
	    inicio_fim = clock() - inicio_fim;
	    write();
	    fclose( file );
	    fclose( fileOut );
	}
 MPI_Finalize();
 return 0;
}
