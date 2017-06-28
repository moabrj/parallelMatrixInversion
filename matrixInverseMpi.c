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
clock_t inicio, fim, total;
MPI_Status status;

/*Aloca o espaço de memória para armazenar a matriz aumentada*/
void makeMatrix() {
	//printf("MAKE MATRIX\n");
	int i;
    augmentedmatrix = (double **)malloc(sizeof(double)*dimension);
    for(i = 0; i < dimension; i++) {         
        augmentedmatrix[i] = (double *)malloc(sizeof(double)*dimension*2); 
    }
}

/*Faz a leitura da mariz a partir do arquivo de entrada*/
void read(){
	//printf("READ\n");
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
	//printf("AUGMENTINGMATRIX\n");
	for(i=0;i<dimension; i++)
		for(j=dimension; j<2*dimension; j++)
			if(i==j%dimension) augmentedmatrix[i][j]=1;
			else augmentedmatrix[i][j]=0;
}

/*Encontra o proximo elemento que vai ser utilizado como pivô*/
void findPivo(){
	//printf("FIND PIVO\n");
	if(rank==0){
		temp=j;
		for(i=j+1; i<dimension; i++){
			if(augmentedmatrix[i][j]>augmentedmatrix[temp][j]){
				temp=i;
			}	
		}		
	}
}

/*Realiza a troca de linhas se o pivô não pertencer a linha atual*/
void swapLine(){
	//printf("SWAP LINE\n");
	if(rank==0){
		if(temp!=j){
			for(k=0; k<2*dimension; k++){
				double temporary=augmentedmatrix[j][k];
				augmentedmatrix[j][k]=augmentedmatrix[temp][k];
				augmentedmatrix[temp][k]=temporary;
			}
		}
	}
}

/*Realiza o calculo dos novos valores para cada linha da matriz aumentada, gerando a matriz inversa*/
void calcInverse(){
    MPI_Bcast (&augmentedmatrix[0][0],dimension*dimension*2,MPI_DOUBLE,0,MPI_COMM_WORLD); 	   
    MPI_Bcast (&j, 1, MPI_INT, 0, MPI_COMM_WORLD);
       
    //printf("CALC INVERSE\n");
    for(i=0; i<dimension; i++){
		if(i%nprocs==rank){
			//printf("My rank is: %d and I am calculating i: %d \n", rank, i);
			//printf("i: %d j:%d k: %d r: %lf dimension: %d augmented: %lf \n", i, j, k, r, dimension, augmentedmatrix[i][j]);
			if(i!=j){ //verifica se é a linha atual.
				//printf("r: %lf augmented: %lf\n", r, augmentedmatrix[i][j]);
				r=augmentedmatrix[i][j];
				//printf("r: %lf augmented: %lf\n", r, augmentedmatrix[i][j]);
				for(k=0; k<2*dimension; k++){
					augmentedmatrix[i][k]-=(augmentedmatrix[j][k]/augmentedmatrix[j][j])*r; //calcula o novo valor para as linhas diferentes da atual.
				}
			}else {
				//printf("r: %lf augmented: %lf\n", r, augmentedmatrix[i][j]);
				r=augmentedmatrix[i][j];
				//printf("r: %lf augmented: %lf\n", r, augmentedmatrix[i][j]);
				for(k=0; k<2*dimension; k++){
					augmentedmatrix[i][k]/=r; //divide os elementos da linha atual pelo pivô.
				}
			}
			//printf("fim for\n");
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
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
	}
	
	MPI_Bcast (&dimension, 1, MPI_INT, 0, MPI_COMM_WORLD); 
	makeMatrix();
	if(rank==0){
		read();
		augmentingmatrix();
    	inicio = clock();
    }
    
	MPI_Bcast (&r, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);   
    MPI_Bcast (&temp, 1, MPI_INT, 0, MPI_COMM_WORLD);
	for(j=0; j<dimension; j++){   
	    findPivo();
	    swapLine();
	    calcInverse();
	}
	
	if(rank==0){
	    fim = clock();
		total = (double) (fim - inicio) /  CLOCKS_PER_SEC;
	    printf("Matrix size %d x %d nprocs: %d time: %.2f s\n", dimension, dimension, nprocs, total);
	    write();
	    fclose( file );
	    fclose( fileOut );
	}
 MPI_Finalize();
 return 0;
}
