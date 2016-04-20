#import <stdlib.h>
#import <string.h>

#import "../sources/mapped_file.h"

#define TESTFAILED 666

/* open / close */
void check(int err) {
  err ? printf("FAILED: %d\n", err) : printf("PASSED\n");
}


int test_1(const char* filename); // open / close
int test_2(const char* filename); // file size
int test_3(const char* filename); // read
int test_4(const char* filename); // map & unmap
int test_5(const char* filename); // write to a file & read & random access

/* Performance tests */
int ptest_1(const char* filename); // iterate over buffer with big file

int main(int argc, char **argv) {
  if (argc != 2) { printf("Input file?\n"); return 1; }
  char *filename = argv[1];
  printf("Input file: %s\n", filename);

  printf("Test 01: "); check(test_1(filename));
  printf("Test 02: "); check(test_2(filename));
  printf("Test 03: "); check(test_3(filename));
  printf("Test 04: "); check(test_4(filename));
  printf("Test 05: "); check(test_5(filename));

//  printf("Performance test 01: "); check(ptest_1(filename));

  return 0;
}

int test_1(const char* filename) {
  mf_handle_t handle = mf_open(filename, 0);
  if (handle == NULL) return TESTFAILED;
  return mf_close(handle);
}

int test_2(const char* filename) {
  mf_handle_t handle = mf_open(filename, 0);
  ssize_t size = mf_file_size(handle);
//  printf("%zd\n", size);
  if (size == -1) return TESTFAILED;
  return mf_close(handle);
}

int test_3(const char* filename) {
  size_t bytes_to_read = 10;
  mf_handle_t handle = mf_open(filename, 0);
  void *buf = malloc(bytes_to_read);
  ssize_t size = mf_file_size(handle);
  ssize_t read_bytes = mf_read(handle, mf_file_size(handle) / 2, bytes_to_read, buf);
  //  printf("\nsize: %zu\nshould read: %zu\nread: %zu\n", size, bytes_to_read, read_bytes);
  if (!((size_t)read_bytes == bytes_to_read || read_bytes == size))
    return (int)(read_bytes - bytes_to_read);
  return mf_close(handle);
}

int test_4(const char* filename) {
  mf_handle_t handle = mf_open(filename, 0);

  mf_mapmem_t *mm = mf_map(handle, 0, 10);
  if (mm == NULL || mm->ptr == NULL) return 1;

  int err = mf_unmap(mm);
  if (err) return err;

  return mf_close(handle);
}

int test_5(const char* filename) {
  mf_handle_t handle = mf_open("tests/temp", 0);
  mf_write(handle, 0, 10, "oTENCHARSo");
  char *buf = (char *)malloc(10);
  memset(buf, 0, 10);
  mf_read(handle, 1, 8, buf);
  if (strcmp("TENCHARS", buf)) return TESTFAILED;
  mf_write(handle, 4, 6, "ELEVEN");

  memset(buf, 0, 10);
  mf_read(handle, 6, 4, buf);
  if (strcmp("EVEN", buf)) return TESTFAILED + 1;

  memset(buf, 0, 10);
  mf_read(handle, 1, 6, buf);
  if (strcmp("TENELE", buf)) return TESTFAILED + 2;

  free(buf);
  int err = mf_close(handle);
  remove("tests/temp");
  return err;
}



int ptest_1(const char* filename) {
  if (!strcmp(filename, "big") && !strcmp(filename, "tests/big")) { printf("THIS TEST REQUIRES SPECIAL FILE - "); return 1; }
  mf_handle_t handle = mf_open(filename, 0);
  size_t size = (size_t)mf_file_size(handle);
  if (size != 40000001345) { printf("THIS TEST REQUIRES SPECIAL FILE - "); return 1; }
  //  printf("size = %zu\n", size);
  size_t bufsize = 1024;
  void *buf = malloc(bufsize);

  int counter = 1337 + 8; // number of ones (1) exists in file
  int counter1 = 1000;
  int counter2 = 300;
  int counter3 = 30;
  int counter4 = 7;
  size_t readbytes = 0;
  char current;
  for (size_t j = 0; readbytes < size; j += bufsize) {
    readbytes += (size_t)mf_read(handle, j, bufsize, buf);
    //  if (!(j % (bufsize * 1000000))) printf ("%zu\n", j);
    for (int i = 0; i < bufsize; ++i) {
      current = ((char *) buf)[i];
      if (current != '9') counter--;
      if (current == '1') counter1--;
      if (current == '2') counter2--;
      if (current == '3') counter3--;
      if (current == '4') counter4--;
    }
  }
  //  printf("\ntotal: %d\n1: %d\n2: %d\n3: %d\n4: %d\n", counter, counter1, counter2, counter3, counter4);
  if (counter || counter1 || counter2 || counter3 || counter4)
    return TESTFAILED;
  free(buf);
  return mf_close(handle);
}