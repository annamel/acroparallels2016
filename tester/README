# Tester

 Tester is a script for __Mapped file__ c library. This script goes through all dirs in ../ and makes cross-people tests.

# Run

  ```bash
$ ./tester.sh
  ```

# Requirements
 Projects are expected to be built by
 __make__ at `$USERNAME/mapped_file/Makefile` or
 __cmake__ at `$USERNAME/mapped_file/CMakeLists.txt`.

 Tester expects to find __libmappedfile.a__ at `$USERNAME/mapped_file/out/`.

 Test files are build automatically and sources are expected to be at `$USERNAME/mapped_file/test/`.

 Library and test directories can be specified by setting the __ROOT_LIB_DIR__ and __ROOT_TEST_DIR__ variables. By default this script will test all libraries and use all tests.

 Example:
 ```bash
$ ROOT_LIB_DIR="../dkopyrin ../ivanov" ROOT_TEST_DIR="../sabramov ../osamara" ./tester.sh
 ```
 To use valgrind use `PREC="valgrind"`. CFLAGS are also supported.

# Credits
Made by [Denis Kopyrin](https://github.com/aglab2) and [Daniil Okhlopkov](https://github.com/ohld).
