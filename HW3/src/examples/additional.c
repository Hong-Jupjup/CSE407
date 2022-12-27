#include <stdio.h>
#include <syscall.h>
#include <stdlib.h>

int
main (int argc, char **argv)
{
  int i;
  int arr[4];

  for(i=0; i<4; i++) {
	arr[i] = atoi(argv[i+1]);
  }

  printf("%d %d\n", fibonacci(arr[0]), max_of_four_int(arr[0], arr[1], arr[2], arr[3]));

  return EXIT_SUCCESS;
}
