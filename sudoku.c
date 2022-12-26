#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>

#define NTHREADS 16
typedef unsigned int uint;

int is_valid_move(int (*)[36], uint, uint, uint, int);
int is_valid_row(int *, uint, int);
int is_valid_col(int (*)[36], uint, uint, int);
int is_valid_box(int (*)[36], uint, uint, uint, int);
void *backtrack(void*);
int generate_threads(int (*)[36], int);
struct sudoku_params {
    int **grid;
    int i;
    int j;
    int size_sqrt;
    int idx;
};
int counter = 0;
int is_solved = 0;
int solved_idx = -1;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
struct sudoku_params thread_params[NTHREADS];

void read_grid_from_file(int size, char *ip_file, int grid[36][36]) {
	FILE *fp;
	int i, j;
	fp = fopen(ip_file, "r");
	for (i=0; i<size; i++) 
		    for (j=0; j<size; j++) {
			        fscanf(fp, "%d", &grid[i][j]);
	}
}

void print_grid(int size, int grid[36][36]) {
	int i, j;
	/* The segment below prints the grid in a standard format. Do not change */
	for (i=0; i<size; i++) {
		    for (j=0; j<size; j++)
			        printf("%d\t", grid[i][j]);
		    printf("\n");
	}
}

int main(int argc, char *argv[]) {
    int grid[36][36], size, i, j;
	
	if (argc != 3) {
            printf("Usage: ./sudoku.out grid_size inputfile");
            exit(-1);
	}
	
	size = atoi(argv[1]);
	read_grid_from_file(size, argv[2], grid);
	
    // backtrack fn
    //print_grid(size, grid);
    // backtrack(&params);
    generate_threads(grid, size);

    //memcpy(grid, params.grid, 36*36*sizeof(int));
    // print_grid(size, grid);
    printf("The grid is solved: %d\n", is_solved);
    return 0;
}

int is_valid_move(int grid[36][36], uint row_pos, uint col_pos, uint num, int size_sqrt) {
    uint row_valid = is_valid_row(grid[row_pos], num, size_sqrt * size_sqrt);
    uint col_valid = is_valid_col(grid, col_pos, num, size_sqrt * size_sqrt);
    uint box_valid = is_valid_box(grid, row_pos, col_pos, num, size_sqrt);

    // printf("\nrow valid: %d col valid: %d box valid: %d\n", row_valid, col_valid, box_valid);
    if (row_valid && col_valid && box_valid) {
        // printf("The grid is valid. %d\n", num);
        return 1;
    }
    return 0;
}

int is_valid_row(int *row, uint num, int size) {
    for (int i = 0 ; i < size; i++ ) {
        if (row[i] == num) {
            return 0;
        }
    }
    return 1;
}

int is_valid_col(int grid[36][36], uint col_pos, uint num, int size) {
    for (int i = 0 ; i < size; i++ ) {
        if (grid[i][col_pos] == num) {
            return 0;
        }
    }
    return 1;
}

int is_valid_box(int grid[36][36], uint row_pos, uint col_pos, uint num, int size_sqrt) {
    uint start_pos_row = row_pos - (row_pos % size_sqrt);
    uint start_pos_col = col_pos - (col_pos % size_sqrt);

    for (int i = start_pos_row; i < start_pos_row + size_sqrt; i++) {
        for (int j = start_pos_col; j < start_pos_col + size_sqrt; j++) {
            if (grid[i][j] == num) {
                return 0;
            }
        }
    }

    return 1;
}

int backtrack_bfs(int grid[36][36], int i, int j, int size_sqrt, int visited[36][36]) {

    if (i < 0 || j < 0 || i >= size_sqrt * size_sqrt || j >= size_sqrt * size_sqrt || visited[i][j]) {
        return 0;
    }

    visited[i][j] = 1;
    if (grid[i][j] != 0) {
        backtrack_bfs(grid, i, j + 1, size_sqrt, visited);
        backtrack_bfs(grid, i + 1, j, size_sqrt, visited);
        backtrack_bfs(grid, i, j - 1, size_sqrt, visited);
        backtrack_bfs(grid, i - 1, j, size_sqrt, visited);
    } else {
        uint valid_nums[9], p = 0, is_grid_valid = 1;

        for (int num = 1; num <= size_sqrt * size_sqrt; num++) {
            if (is_valid_move(grid, i, j, num, size_sqrt)) {
                valid_nums[p] = num;
                p += 1;
            }
        }
        for (int pos = 0; pos < p; pos++) {
            grid[i][j] = valid_nums[pos];

            backtrack_bfs(grid, i, j + 1, size_sqrt, visited);
            backtrack_bfs(grid, i + 1, j, size_sqrt, visited);
            backtrack_bfs(grid, i, j - 1, size_sqrt, visited);
            backtrack_bfs(grid, i - 1, j, size_sqrt, visited);

        if (is_grid_valid) {
            
            return 1;    
        }

        grid[i][j] = 0;
        }
    }
        print_grid(size_sqrt * size_sqrt, grid);
        is_solved = 1;
        return 1;
}

int backtrack_recurr(int grid[36][36], int start_i, int start_j, uint i, uint j, int size_sqrt) {
    if (is_solved) return 0;

    int n = size_sqrt * size_sqrt;
    
    if (grid[i][j] != 0) {
        int new_j = (j + 1) % n;
        int new_i = (i + 1) % n;
        if (new_j != start_j) {
            return backtrack_recurr(grid, start_i, start_j, i, new_j, size_sqrt);
        }
        else if (new_i != start_i) {
            return backtrack_recurr(grid, start_i, start_j, new_i, start_j, size_sqrt);
        }

        is_solved = 1;
        return 1;
    }

    uint valid_nums[9], p = 0, is_grid_valid = 1;

    for (int num = 1; num <= size_sqrt * size_sqrt; num++) {
        if (is_valid_move(grid, i, j, num, size_sqrt)) {
            valid_nums[p] = num;
            p += 1;
        }
    }

    for (int pos = 0; pos < p; pos++) {
        grid[i][j] = valid_nums[pos];

        int new_j = (j + 1) % n;
        int new_i = (i + 1) % n;
        if (new_j != start_j) {
            is_grid_valid = backtrack_recurr(grid, start_i, start_j, i, new_j, size_sqrt);
        }
        else if (new_i != start_i) {
            is_grid_valid = backtrack_recurr(grid, start_i, start_j, new_i, start_j, size_sqrt);
        } else {
            is_solved = 1;
            is_grid_valid = 1;
        }

        if (is_grid_valid) {
            return 1;    
        }

        grid[i][j] = 0;
    }

    return 0;
}

void *backtrack(void *args) {
    // printf("Thread number %ld\n", pthread_self());
    struct sudoku_params *params = (struct sudoku_params *) args;
    counter++;
    // printf("Counter value: %d params: %d %d \n", counter, params->i, params->j);
    int ans = backtrack_recurr(params->grid, params->i, params->j, params->i, params->j, params->size_sqrt);
    if (is_solved && ans == 1) {
        solved_idx = params->idx;
    }
}

int generate_threads(int grid[36][36], int size) {

    pthread_t thread_id[NTHREADS];
    int thread_cnt = 0;
    for (int j = 0; j < size; j++) {
        for (int k = 0; k < size; k++) {
            if (grid[j][k] == 0 && thread_cnt < NTHREADS) {
                thread_params[thread_cnt].grid = (int **) malloc(36*36*sizeof(int));
                memcpy(thread_params[thread_cnt].grid, grid, 36*36*sizeof(int));
                thread_params[thread_cnt].i = j;
                thread_params[thread_cnt].j = k;
                thread_params[thread_cnt].size_sqrt = sqrt(size);
                thread_params[thread_cnt].idx = thread_cnt;
                if (thread_cnt < NTHREADS) {
                    printf("Thread count: %d %d %d \n", thread_cnt, thread_params[thread_cnt].i, thread_params[thread_cnt].j);
                    pthread_create(&thread_id[thread_cnt], NULL, backtrack, &thread_params[thread_cnt]);
                    thread_cnt++;
                }
            }
        }
    }

    for(int l = 0; l < NTHREADS; l++) {
        pthread_join(thread_id[l], NULL); 
    }

    print_grid(thread_params[solved_idx].size_sqrt * thread_params[solved_idx].size_sqrt, thread_params[solved_idx].grid);
}