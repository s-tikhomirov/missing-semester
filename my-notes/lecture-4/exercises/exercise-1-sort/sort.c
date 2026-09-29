#include <stdio.h>
#include <assert.h>

static int arr[] = {3, 1, 4, 1, 5, 9, 2, 6};
#define ARR_LEN (sizeof(arr) / sizeof(arr[0]))

// const means we don't modify
void print_array(const int *arr, int start, int end) {
  for (int i = start; i < end; i++) {
    printf("%d ", arr[i]);
  }
  printf("\n");
}

// start inclusive, end exclusive
void merge(
  int *arr,
  int left_start,
  int mid,
  int right_end) {
  #ifdef DEBUG
    printf("\nin merge()\n");
    printf("left idx:  [ %d %d ]\n", left_start, mid - 1);
    print_array(arr, left_start, mid);
    printf("right idx: [ %d %d ]\n", mid, right_end - 1);
    print_array(arr, mid, right_end);
  #endif
  int tmp[right_end - left_start];
  int i = left_start;
  int j = mid;
  int k = 0;
  while (i < mid && j < right_end) {
    if (arr[i] <= arr[j]) {
      tmp[k] = arr[i];
      i++;
    } else {
      tmp[k] = arr[j];
      j++;
    }
    k++;
  }
  // append remaining elements to tmp
  // exactly one half contains elements at this point
  while (i < mid) {
    tmp[k++] = arr[i++];
  }
  while (j < right_end) {
    tmp[k++] = arr[j++];
  }
  //copy tmp over to original arr
  for (int i = 0; i < (right_end - left_start); i++) {
    arr[left_start + i] = tmp[i];
  }
}

void merge_sort(int *arr, int start, int end) {
  #ifdef DEBUG
    printf("sorting arr [ %d %d ]\n", start, end - 1);
    printf("before sorting:\n");
    print_array(arr, start, end);
  #endif
  assert(start <= end);
  int len = end - start;
  if (len <= 1) {
    assert(start + 1 == end);
    return;
  }
  int mid = start + len / 2;
  merge_sort(arr, start, mid);
  merge_sort(arr, mid, end);
  merge(arr, start, mid, end);
  #ifdef DEBUG
    printf("after sorting:\n");
    print_array(arr, start, end);
  #endif
}

int main(void) {
  printf("Original array:\n");
  print_array(arr, 0, ARR_LEN);
  merge_sort(arr, 0, ARR_LEN);
  printf("Sorted array:\n");
  print_array(arr, 0, ARR_LEN);
  return 0;
}
