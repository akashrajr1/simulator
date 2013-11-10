#include "../uc-inc/stdlib.h"
#define LCG_MUL	214013
#define LCG_INC	2531011
#define LEN 10000
int array[LEN];

inline void swap(int arr[], int i, int j) {
    int temp = arr[j];
    arr[j] = arr[i];
    arr[i] = temp;
}

inline void quicksort(int arr[], int a, int b) {
    if (a >= b)
        return;

    int key = arr[a];
    int i = a + 1, j = b;
    while (i < j) {
        while (i < j && arr[j] >= key)
            --j;
        while (i < j && arr[i] <= key)
            ++i;
        if (i < j)
            swap(arr, i, j);
    }
    if (arr[a] > arr[i]) {
        swap(arr, a, i);
        quicksort(arr, a, i - 1);
        quicksort(arr, i + 1, b);
    } else { // there is no left-hand-side
        quicksort(arr, a + 1, b);
    }
}

int main(){
	int i;
	uint32_t num;
	for (i = 0, num = 0; i < LEN; i++){
		array[i] = num;
		sys_putint(num);
		num = num * LCG_MUL + LCG_INC;
	}
	quicksort(array, 0, LEN-1);
	for (i = 0; i < LEN; i++){
		sys_putint(array[i]);
	}
	return 0;
}