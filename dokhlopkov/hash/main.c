#include <stdint.h>
#include <stdio.h>
#include "hash.h"


int main (int argc, char **argv ) {
  if (argc < 2) {
    printf("Give me integer in [0, 4294967295]\n");
    return 1;
  }
  uint32_t v = 0;
  if (sscanf(argv[1], "%u", &v) == 1) {
      printf("Input:  %u\n", v);
      printf("Output: %u\n", hash(v));
  } else {
      printf("Not an integer.\n");
  }
  return 0;
}
