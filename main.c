#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>
#include <assert.h>


void initialization(int *up, int N, int a, int b) {
    srand(time(NULL));
    for(int i = 0; i < N; i++) {
        up[i] = rand() % (b - a + 1) + a;
    }
}


int bin_search(int *arr, int left, int right, int val) {
    if(right - left <= 1) {
        if(arr[left] < val) {
            return right;
        }
        else return left;
    }
    int mid = (left + right) / 2;

    if(arr[mid] < val) {
         return bin_search(arr, mid, right, val);
    }
    else {
        return bin_search(arr, left, mid, val);
    }
}


int cmp(const void *a, const void *b) {
     return *(int*)a - *(int*)b;
 }


void merge(int *target, int ind, int *X, int lX, int rX, int *Y, int lY, int rY)
{
  while(lX < rX && lY < rY) {
    if(X[lX] < Y[lY]) {
      target[ind] = X[lX];
      lX++;
    } else {
      target[ind] = Y[lY];
      lY++;
    }
    ind++;
  }

  while(lX < rX) {
    target[ind] = X[lX];
    lX++;
    ind++;
  }

  while(lY < rY) {
    target[ind] = Y[lY];
    lY++;
    ind++;
  }
}

int* merge_sort(int *up, int *down, int left, int right, int M) {


    int width = right - left;
    if (width <= M) {
          qsort(up + left, width, sizeof(int), cmp);
          return up;
    }

    int middle = (int)((left + right) * 0.5);

    // сортировка
    int *l_buff;
    int *r_buff;

    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task
            {
                l_buff = merge_sort(up, down, left, middle, M);
            }
            #pragma omp task
            {
                r_buff = merge_sort(up, down, middle, right, M);
            }

        }
    }

    // слияние
    int mid_1 = (middle + left) / 2;
    int mid_2 = bin_search(r_buff, middle, right, l_buff[mid_1]);
    int mid_3 = left + (mid_1 - left) + (mid_2  - middle);

    int *target = l_buff == up ? down : up;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task
            {
                merge(target, left, l_buff, left, mid_1, r_buff, middle, mid_2);

            }
            #pragma omp task
            {
                merge(target, mid_3, l_buff, mid_1, middle, r_buff, mid_2, right);
            }
        }
    }

    return target;
}



int main(int argc, char** argv) {


    unsigned int N = atoi(argv[1]);
    unsigned int M = atoi(argv[2]);
    unsigned int P = atoi(argv[3]);

    int *up = (int*)malloc(sizeof(int) * N);

    initialization(up, N, 0, 200);
    omp_set_num_threads(P);

    FILE *data = fopen("data.txt", "w");

    for (int i = 0; i < N; i++) {
        fprintf(data, "%d ", up[i]);
    }

    struct timeval start, end;
    int *down = (int*)malloc(sizeof(int) * N);
    assert(gettimeofday(&start, NULL) == 0);
    int *res = merge_sort(up, down, 0, N, M);
    assert(gettimeofday(&end, NULL) == 0);

    double delta = ((end.tv_sec - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;

    FILE *stats = fopen("stats.txt", "w");
    fprintf(stats, "%.5fs %d %d %d\n", delta, N, M, P);
    fclose(stats);

    fprintf(data, "\n");
    for (int i = 0; i < N; i++) {
        fprintf(data, "%d ", res[i]);
    }

    fclose(data);
    free(up);
    free(down);
    return 0;
}
