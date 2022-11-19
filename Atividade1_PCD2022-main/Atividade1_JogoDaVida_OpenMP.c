// Brendo Yuki Katagi - RA 120.682
// Luan Olimpio Claro da Costa - RA 120.855
// Matheus Anido Pereira Coimbra - RA 104.112

// Atividade 1 - Programacao Concorrente e Distribuida
// Jogo da Vida - OpenMP

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

# define GERACOES 2000
# define DIMENSOES 2048
# define POSITION(x) (x+DIMENSOES)%DIMENSOES

void inicializa_tabuleiro1(int ** grid) {
    //GLIDER
  grid[1][2]=1;
  grid[2][3]=1;
  grid[3][1]=1;
  grid[3][2]=1;
  grid[3][3]=1;

  grid[10][31]=1;
  grid[10][32]=1;
  grid[11][30]=1;
  grid[11][31]=1;
  grid[12][31]=1;
}

int varredura(int i, int j, int ** grid) {
  int k, l, count = 0;
  for (k = i-1; k <= i+1; k++) {
    for(l = j-1; l <= j+1; l++) {
      if(k != i || l != j)
        count += grid[POSITION(k)][POSITION(l)];
    }
  }
  return count;
}

void jogo_da_vida(int ** grid, int ** newGrid, int g) {
  int i, j, count = 0;

  #pragma omp parallel shared(grid, newGrid) private(i, j, count) 
  {
    #pragma omp for
    for(i=0; i<DIMENSOES; i++){
      for(j=0; j<DIMENSOES; j++) {
        count = varredura(i, j, grid);
        if((count == 2 || count == 3) && grid[i][j] == 1) {
          newGrid[i][j] = 1;
        } else if(count == 3 && grid[i][j] == 0) {
          newGrid[i][j] = 1;
        } else {
          newGrid[i][j] = 0;
        }
      }
    }
    if(g == 0) 
      printf("N Threads: %d\n", omp_get_num_threads());
  }
}

int ** inicia_grid() {
  int i;
  int ** grid = (int **) calloc(sizeof(int *), DIMENSOES);
  for(i = 0; i < DIMENSOES; i++) {
    grid[i] = (int *) calloc(sizeof(int), DIMENSOES);
  }
  return grid;
}

void libera_grid(int ** grid) {
  int i;
  for(i = 0; i < DIMENSOES; i++)
    free(grid[i]);
  free(grid);
}

int main() {
  int i, j, count = 0;
  int ** grid, **newGrid, **aux;
  double itime, ftime, exec_time;

  grid = inicia_grid();
  newGrid = inicia_grid();

  inicializa_tabuleiro1(grid);

  itime = omp_get_wtime();
  for(i=0; i < GERACOES; i++) {
    jogo_da_vida(grid, newGrid, i);
    aux = grid;
    grid = newGrid;
    newGrid = aux;
  }
  ftime = omp_get_wtime();
  exec_time = ftime - itime;

  for(i = 0; i < DIMENSOES; i++) {
    for(j = 0; j < DIMENSOES; j++) {
        count += grid[i][j];
    }
  }
  printf("%d\n", count);
  printf("Tempo de execucao: %f\n", exec_time);
  libera_grid(grid);
  libera_grid(newGrid);
  aux = NULL;
  return 0;
}
