#include <iostream>
#include "log.hpp"
#include <cstdlib>
using namespace std;

int main() {
	#ifdef DEBUG
	#endif
	if(Log::logger_init()) {
	for(int i = 0; i < 10; i++) {
		Log::buf_write("Test!", Debug);
		Log::buf_write("Test!", Info);
		Log::buf_write("Test!", Warning);
		Log::buf_write("Test!", Error);
		}
	} 	else {
		cerr << "File cannot be opened!\n";
		exit(1);
	}
	Log::buf_write("Test!", Fatal);
	Log::logger_deinit();
	return 0;
}