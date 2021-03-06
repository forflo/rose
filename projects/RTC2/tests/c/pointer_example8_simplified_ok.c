#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define PTR_SIZE 100
#define PTR2_SIZE 10

void print_array(unsigned int* array, unsigned int size) {}

// FIXME: The parameters are not being replaced with
// structed versions
unsigned int* fn2(unsigned int*, float*, double*, char*);

unsigned int* fn1(unsigned int* input)
{
  unsigned int* nixptr = fn2(input, (float*)input, (double*)input, (char*)input);

  printf("input: %u\n", *input);
  return (unsigned int*)malloc(PTR_SIZE*sizeof(unsigned int));
}

unsigned int* fn2(unsigned int* input, float* input2, double* input3, char* input4) {
  return NULL;
}

int main() {

  printf("main - begin\n");

  unsigned int *ptr = (unsigned int*)malloc(PTR_SIZE*sizeof(int));
  unsigned int *deptr1 = ptr;
  unsigned int *ptr2 = (unsigned int*)malloc(PTR2_SIZE*sizeof(int));
  unsigned int *deptr2 = ptr2;

  printf("ptr & ptr2 - done\n");

  unsigned int* start_ptr = ptr;
  unsigned int* start_ptr2 = ptr2;

  printf("start_ptr & start_ptr2 - done\n");

  unsigned int *start_ptr3 = (unsigned int*)malloc(PTR_SIZE*sizeof(unsigned int)), *start_ptr4 = start_ptr2;
  unsigned int *deptr3 = start_ptr3;

  printf("start_ptr3 - done\n");

  *start_ptr = 1;
  *start_ptr2 = 1;

  printf("start_ptr: %u\n", *start_ptr);
  printf("start_ptr2: %u\n", *start_ptr2);

  *ptr = 3;
  *ptr2 = 9;

  printf("start_ptr: %u\n", *start_ptr);
  printf("start_ptr2: %u\n", *start_ptr2);
  printf("start_ptr, start_ptr2, ptr, ptr2 - done\n");

  unsigned int* new_ptr;

  for(new_ptr = start_ptr; new_ptr < start_ptr + PTR_SIZE; new_ptr++) {
    *new_ptr = 5;
    printf("%u\n", *new_ptr);
  }

  for(new_ptr = start_ptr2; new_ptr < start_ptr2 + PTR2_SIZE; new_ptr++) {
    *new_ptr = 25;
    printf("%u\n", *new_ptr);
  }

  printf("for loop 1 - done\n");

  unsigned int* whileptr = NULL;

  do {
    unsigned int* doptr = start_ptr;
    unsigned int* forptr2 = (unsigned int*)malloc(PTR_SIZE*sizeof(unsigned int)), *doptr2 = forptr2;
    for(; doptr2 < (forptr2 + PTR_SIZE) ; doptr2++) {
    }

    free(forptr2);
  } while(whileptr != NULL);

  unsigned int* tempptr = start_ptr;

  if(whileptr == NULL) {
    start_ptr += PTR_SIZE - 1;
    *start_ptr = 10;
  }

  start_ptr = tempptr;

  printf("Final print\n");
  for(new_ptr = start_ptr; new_ptr < start_ptr + PTR_SIZE; new_ptr++) {
    printf("%u\n", *new_ptr);
  }
  printf("Final print -end\n");


  // CIRM Review Code
  unsigned int *p = (unsigned int*)malloc(PTR_SIZE*sizeof(unsigned int));
  unsigned int *deptr4 = p;
  unsigned int* index = p;
  for(; index < p + PTR_SIZE; index++) {
    *index = 15;
    printf("%u\n", *index);
  }

  unsigned int *q;
  unsigned int y = (q = p, *q = 5, *q);

  printf("q: %u\n", *q);
  printf("p: %u\n", *p);
  printf("y: %u\n", y);
  // ------



  // Passing pointers to function
  unsigned int* fnptr = fn1(start_ptr);
  unsigned int *deptr5 = fnptr;
  unsigned int* fnptr2;
  fnptr2 = fn1(fnptr);
  unsigned int *deptr6 = fnptr2;
  // ------

  // Complex expressions
  unsigned int* complexptr = start_ptr + 1;
  unsigned int *deptr7 = NULL;
  unsigned int complexexp = *ptr + *(deptr7 = fn1(start_ptr++)) - *p;
  printf("complexptr: %u\n", *complexptr);
  printf("complexexp: %u\n", complexexp);
  // Add more

  print_array(p, PTR_SIZE);
  print_array(ptr, PTR_SIZE);
  print_array(ptr2, PTR2_SIZE);

  free(deptr1);
  free(deptr2);
  free(deptr3);
  free(deptr4);
  free(deptr5);
  free(deptr6);
  free(deptr7);

  return 0;
}
