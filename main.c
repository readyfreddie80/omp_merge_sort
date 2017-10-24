#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <assert.h>

void initialization(int *up, int N, int a, int b) {
    srand(time(NULL));
    for(int i = 0; i < N; i++) {
        up[i] = rand() % (b - a + 1) + a;
    }
}
int bin_search(int *arr, int left, int right, int val) {
    if(arr[right] < val)
        return right;
    if(arr[left] >= val)
        return left - 1;


    int mid;
    while(left != right) {
        mid = (left + right) / 2;
        if(arr[mid] == val)
            return mid;
        if(arr[mid] > val)
            right = mid;
        else
            left = mid + 1;
    }

    if(arr[left] > val)
        return left - 1;
    else
        return left;
}

int cmp(const void *a, const void *b) {
     return *(int*)a - *(int*)b;
 }


void merge(int *target, int index, int *l_buff, int l_f, int l_l, int *r_buff, int r_f, int r_l) {
  while(l_f <= l_l && r_f <= r_l) {
    if(l_buff[l_f] < r_buff[r_f]) {
      target[index] = l_buff[l_f];
      l_f++;
    }
    else {
      target[index] = r_buff[r_f];
      r_f++;
    }
    index++;
  }

  while(l_f <= l_l) {
    target[index] = l_buff[l_f];
    l_f++;
    index++;
  }

  while(r_f <= r_l) {
    target[index] = l_buff[r_f];
    r_f++;
    index++;
  }
}

int* merge_sort(int *up, int *down, int left, int right, int M) {

    int width = right - left + 1;
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
                r_buff = merge_sort(up, down, middle + 1, right, M);
            }

        }
    }


    // слияние
    int mid_1 = (middle + left) / 2;
    int mid_2 = bin_search(r_buff, middle + 1, right, l_buff[mid_1]);
    int mid_3 =  mid_1 + mid_2 - middle;

    int *target = l_buff == up ? down : up;
    target[mid_3] = l_buff[mid_1];
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task
            {
                merge(target, left, l_buff, left, mid_1 - 1, r_buff, middle + 1, mid_2);

            }
            #pragma omp task
            {
                merge(target, mid_3 + 1, l_buff, mid_1 + 1, middle, r_buff, mid_2 + 1, right);
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
    int *res = merge_sort(up, down, 0, N - 1, M);
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
