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
    MPI_Bcast (&j, 1, MPI_INT, 0, MPI_COMM_WORLD);
       
    for(i=0; i<dimension; i++){
		if(rank==0 && (i%nprocs)!=0){
			/*Envia cada linha da matrix para um nó de computo*/
      MPI_Send (&augmentedmatrix[i][0],dimension*2,MPI_DOUBLE,i%nprocs,0,MPI_COMM_WORLD);		
		}
		if(i%nprocs==rank){
		    if((i%nprocs)!=0)  //cada processo recebe uma linha da matriz para ser calculada
		        MPI_Recv (&augmentedmatrix[i][0],dimension*2,MPI_DOUBLE,0,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
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
			if((i%nprocs)!=0)
			    /*Cada processo envia o resultado para o master*/
          MPI_Send (&augmentedmatrix[i][0],dimension*2,MPI_DOUBLE,0,0,MPI_COMM_WORLD);	
		}
		if(rank==0 && (i%nprocs)!=0){
      /*O nó master recebe o resultado calculado pelos processos*/
			MPI_Recv (&augmentedmatrix[i][0],dimension*2,MPI_DOUBLE,i%nprocs,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);		
		}
	}
  /*SIncroniza a interação atual*/
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
	MPI_Bcast (&augmentedmatrix[0][0],dimension*dimension*2,MPI_DOUBLE,0,MPI_COMM_WORLD);
	for(j=0; j<dimension; j++){   
	    findPivo();
	    swapLine();
	    calcInverse();
	}
	
	if(rank==0){
	    fim = clock();
		total = (double) (fim - inicio)/CLOCKS_PER_SEC;
	    printf("Matrix size %d x %d nprocs: %d time: %ds\n", dimension, dimension, nprocs, total);
	    write();
	    fclose( file );
	    fclose( fileOut );
	}
 MPI_Finalize();
 return 0;
}
