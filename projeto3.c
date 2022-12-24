// Brendo Yuki Katagi - RA 120.682
// Luan Olimpio Claro da Costa - RA 120.855
// Matheus Anido Pereira Coimbra - RA 104.112

// Atividade 3 - Programacao Concorrente e Distribuida
// Jogo da Vida - MPI

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

# define GERACOES 2000
# define DIMENSOES 2048
# define POSITION(x) (x+DIMENSOES)%DIMENSOES
# define POS(x,size) (x+size)%size

const int cordX[] = {2,3,1,2,3,31,32,30,31,31};
const int cordY[] = {1,2,3,3,3,10,10,11,11,12};

int getSize(int pid, int num_proc){
    int size = DIMENSOES/num_proc;

    if(pid == num_proc-1){
        size = DIMENSOES - ((num_proc-1) * size);
    }

    return size;
}


void inicializa_tabuleiro1(int ** grid, int pid,int num_proc) {
  int i, y;
  int size = getSize(0, num_proc);
  // printf("numproc: %d\n", num_proc);
  // printf("pid: %d\n", pid);

  for(i = 0; i < 10; i++){
    if(pid == 0 && cordY[i] < size){ //processo pai
      grid[cordY[i]][cordX[i]] = 1;
    }else if(pid == num_proc-1 && cordY[i] >= (pid * size)){ //último processo
      y = POS(cordY[i], size);
      grid[y][cordX[i]] = 1;
    }else if(cordY[i] >= (pid * size) && cordY[i] < (pid+1 * size)){
      y = POS(cordY[i], size);
      printf("y: %d\n", y);
      grid[y][cordX[i]] = 1;
    };
  }
}

int varredura(int i, int j, int ** grid, int size, int rank, int num_proc){
  int k, l, count = 0, border = 0, src, dest, *send;
  for (k = i-1; k <= i+1; k++) {
    for(l = j-1; l <= j+1; l++) {
      if(l == -1 || l == size) {
        src = l == -1 ? (rank - 1 + num_proc) % num_proc : (rank + 1 + num_proc) % num_proc;
        dest = l == -1 ? (rank + 1 + num_proc) % num_proc : (rank - 1 + num_proc) % num_proc;
        send = l == -1 ? &grid[POSITION(k)][size-1] : &grid[POSITION(k)][0];
        if(rank % 2 == 0) {
          MPI_Send(send, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
          MPI_Recv(&border, 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        } else {
          MPI_Recv(&border, 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Send(send, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
        }
        count += border;
      } else if(k != i || l != j)
        count += grid[POSITION(k)][POSITION(l)];
    }
  }
  return count;
}

void jogo_da_vida(int ** grid, int ** newGrid, int size, int rank, int num_proc) {
  int i, j, count = 0;

  for(i=0; i<DIMENSOES; i++){
    for(j=0; j<size; j++) {
      count = varredura(i, j, grid, size, rank, num_proc);
      if((count == 2 || count == 3) && grid[i][j] == 1) {
        newGrid[i][j] = 1;
      } else if(count == 3 && grid[i][j] == 0) {
        newGrid[i][j] = 1;
      } else {
        newGrid[i][j] = 0;
      }
    }
  }

}

int ** inicia_grid(int size) {
  int i;

  int ** grid = (int **) calloc(sizeof(int *), DIMENSOES);
  for(i = 0; i < DIMENSOES; i++) {
    grid[i] = (int *) calloc(sizeof(int), size);
  }
  return grid;
}

void libera_grid(int ** grid) {
  int i;
  for(i = 0; i < DIMENSOES; i++)
    free(grid[i]);
  free(grid);
}

int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);

  int pid, num_proc;
  MPI_Comm_rank(MPI_COMM_WORLD, &pid);
  MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
  int i, j, count = 0, recvCount = 0;
  int ** grid, **newGrid, **aux;
  int size = getSize(pid, num_proc);
  double itime, ftime, exec_time;
  grid = inicia_grid(size);
  newGrid = inicia_grid(size);

  inicializa_tabuleiro1(grid, pid, num_proc);
  for(i=0; i < GERACOES; i++) {
    jogo_da_vida(grid, newGrid, size, pid, num_proc);
    aux = grid;
    grid = newGrid;
    newGrid = aux;
  }
  for(i = 0; i < DIMENSOES; i++) {
    for(j = 0; j < size; j++) {
      count += grid[i][j];
    }
  }
  if(pid == 0) {
    for(i = 1; i < num_proc; i++) {
      MPI_Recv(&recvCount, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      count += recvCount;
    }
    printf("%d\n", count);
  } else {
    MPI_Send(&count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }
  MPI_Finalize();
  return 0;
}