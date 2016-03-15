#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <cxxabi.h>
#include "logger.h"

int func2(int a){
	log(LOG_INFO, "tis a log from func 2");
	return a;
}

int func1(int calls){
	return calls>0?func1 (calls - 1):func2(5);
}

int func3(int b){
	log(LOG_WARNING, "tis a log from func 3");
}

int main (int argc, char *argv[]){
	string logFilename ="";
	if (argc >= 2){
		logFilename = argv[1];
	}
	logInit(LOG_WARNING, logFilename);
	int b = 10;
	
	func1(b);
	func3(b);
	thread Thr=thread(func3, b);
	Thr.join();
	setLogLevel(LOG_INFO);
	
	func1(b);
	func3(b);
	logEnd();
	
}
