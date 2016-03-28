#ifndef LOGGER_H   
#define LOGGER_H
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <cxxabi.h>
#include <mutex> 
#include <thread> 
#include <vector>
#include <queue>
#include <string>
#include <fstream>
#include <atomic>
using namespace std;

enum LOG_LEVEL{
	LOG_NONE, 
	LOG_DEBUG, 
	LOG_INFO, 
	LOG_WARNING, 
	LOG_ERROR,
	LOG_KILL, //special log level; when a process which writes from buffers into the file recieves it, it DIES
	LOG_LEVEL_NUMBER
}; 
extern string level_message[5];
extern LOG_LEVEL currentLogLevel;
extern mutex freeStringsOccupied, messagesOccupied;
extern queue< string > freeStrings, messages;
extern fstream logFile;
extern atomic<int> numOfMessages;
extern thread logThread;
void printToLog(string logString); //done; called from logging functions; places log strings into buffer to be printed into file by printing thread
int logInit(LOG_LEVEL level, string filename); //initialises logging thread - sets file, initialises buffers etc
void log(LOG_LEVEL level,string logString); //done; function called by the user; gets a logging level and a string; if current logging level is lower, does nothing
void setLogLevel(LOG_LEVEL level); //done; changes current logging level, apparently
void logBacktrace(unsigned int max_frames  = 63); //done; logs backtrace
void loggingThread (string filename); //code which is run by a thread which logs logs into logs
void logEnd();

#endif
