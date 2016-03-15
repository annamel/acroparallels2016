#ifndef LOGGER
#define LOGGER

#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <execinfo.h> 

#define BUFFER_SIZE 1024
#define LOG_FILE "log.txt"
#define AMOUNT_OF_OPTIONS 5
#define SIZE 1000
using namespace std;

const string mes[AMOUNT_OF_OPTIONS] = 
{	"DEBUG: ", 
	"INFO: ", 
	"WARNING: ", 
	"ERROR: ", 
	"FATAL: "
};
 
typedef enum log_type 
{
	Debug, 
	Info, 
	Warning, 
	Error,
	Fatal
} log_type;

typedef struct circular_buffer 
{
	unsigned int position_of_last_empty;
	unsigned int the_end;
	string buffer[BUFFER_SIZE];
} circular_buffer;

/*
* This class descrides the structure of my logger.
*/

class Log 
{	static ofstream out;
	static circular_buffer c_buf;
public:
	static bool logger_init();
	static bool buf_is_full();
	static void clear_buf();
	static void buf_write(string message, log_type type_of_message);
	static void logger_deinit();
};

ofstream Log::out;
circular_buffer Log::c_buf;


#ifndef DEBUG

bool Log::logger_init() {
	return true;
}

void Log::logger_deinit() {
	return;
}

void Log::buf_write(string message, log_type type_of_message){
	return;
}

void clear_buf() {
	return;
}

bool buf_is_full(){
	return true;
}

#endif

#ifdef DEBUG

void Log::clear_buf() {
	for(int i = 0; i < BUFFER_SIZE; i++) {
		c_buf.buffer[i] = "";
		c_buf.position_of_last_empty = 0;
		c_buf.the_end = BUFFER_SIZE - 1;
	}
}

bool Log::buf_is_full() {
	return (c_buf.position_of_last_empty == c_buf.the_end);
}

/*
* This function initializes our class.
*/

bool Log::logger_init() {	
	try {
		for(int i = 0; i < BUFFER_SIZE; i++) {
			clear_buf();
		}
		out.open(LOG_FILE);
		return true;
	} catch(ofstream::failure e) {
		return false;
	}
}

void Log::logger_deinit() {
	try {
		out.close();
	} catch (ofstream::failure e) {
		cerr << "File cannot be closed!\n";
		exit(1);
	}
	return;
}

/*
*This function is used to write messages on buffer, there are 
*several conditions: 
* 1) if type of message is Fatal, this function copy the conten 
*	of buffer to the file and then it makes a stack trace, deinitialize
*	our class and makes an ABEND of the program.
* 2) For other cases this function write a message to the buffer,
*	but if buffer is full, this function copy the content of 
*	buffer and then write a message to the buffer.
*/
void *buffer_for_stack [SIZE];

void Log::buf_write(string message, log_type type_of_message) {
	if(type_of_message == Fatal) {
		for(int i = 0; i < (c_buf.position_of_last_empty - 1); i++) {
			out << c_buf.buffer[i] << endl;
		} 
		int nptrs = backtrace(buffer_for_stack, SIZE);
		char **stack_interior;
		stack_interior = backtrace_symbols(buffer_for_stack, SIZE);
		out << mes[type_of_message] << endl;
		for(int i = 0; i < nptrs; i++) {
			out << stack_interior[i] << endl;
		}
		logger_deinit();
		cerr << "Fatal error!\n";
		exit(1);
	} else {
	if(buf_is_full()) {
		for(int i = 0; i < BUFFER_SIZE; i++) {
		out << c_buf.buffer[i] << endl;
	}
	clear_buf();
	}
	c_buf.buffer[c_buf.position_of_last_empty] = mes[type_of_message] + message;
	c_buf.position_of_last_empty += 1;
}
}

#endif
#endif












