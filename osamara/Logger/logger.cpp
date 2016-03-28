#ifndef LOGGER_CPP 
#define LOGGER_CPP
#include "logger.h"
queue< string > freeStrings, Messages;
string level_message[5] = {"", "DEBUG", "INFO", "WARNING", "ERROR"};
mutex freeStringsOccupied, messagesOccupied;
LOG_LEVEL currentLogLevel;
fstream logFile;
atomic<int> numOfMessages, gottabekilled;
thread logThread;

void setLogLevel(LOG_LEVEL level){
	#ifdef DEBUG
	currentLogLevel = level;
	#endif
}
#ifdef DEBUG
void printToLog(string logString){
	string str;
	freeStringsOccupied.lock();
	if (!freeStrings.empty()){
		str = freeStrings.front();
		freeStrings.pop();
		freeStringsOccupied.unlock();
	}
	else {
		freeStringsOccupied.unlock();
		str="";
		
	}
	str = logString;
	messagesOccupied.lock();
	Messages.push(str);
	numOfMessages++;
	messagesOccupied.unlock();
}
#endif
void log(LOG_LEVEL level,string logString){
	#ifdef DEBUG
	if (level<=currentLogLevel){
		string intro = "Log from thread: ", outro = "End of log from thread: ";
		std::hash<std::thread::id> hasher;
		intro += to_string((int) hasher(this_thread::get_id()));
		intro += "; level: " + level_message[level];
		outro += to_string((int) hasher(this_thread::get_id()));
		printToLog(intro);
		printToLog(logString);
		
		if (level == LOG_ERROR)
			logBacktrace();
		printToLog(outro);	
	}
	#endif
}

void log(LOG_LEVEL level, int logNum){
	#ifdef DEBUG
	log(level, to_string(logNum));
	#endif
}

#ifdef DEBUG
void logBacktrace(unsigned int max_frames){
	int addrlen=0;
	void* addrlist[max_frames+1];
	string intro = "Backtrace called from thread: ";
	std::hash<std::thread::id> hasher;
	intro += to_string((int) hasher(this_thread::get_id()));
	printToLog(intro);
    addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));
    char** symbollist = backtrace_symbols(addrlist, addrlen);
    size_t funcnamesize = 256;
    char* funcname = (char*)malloc(funcnamesize);
    for (int i = 0; i < addrlen; i++)
    {
		printToLog(symbollist[i]);
		char *begin_name = 0, *begin_offset = 0, *end_offset = 0;
		for (char *p = symbollist[i]; *p; ++p)
		{
			if (*p == '(')
			begin_name = p;
			else if (*p == '+')
				begin_offset = p;
			else if (*p == ')' && begin_offset) {
				end_offset = p;
				break;
			}
		}
		if (begin_name && begin_offset && end_offset && begin_name < begin_offset) {
			*begin_name++ = '\0';
			*begin_offset++ = '\0';
			*end_offset = '\0';
			int status;
			char* ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
			if (status == 0) {
				funcname = ret;
				string tmp = ">demangled: ";
				tmp+=funcname;
				printToLog(tmp);
			}
		}
    }
    free(funcname);
    free(symbollist);
    printToLog("End of backtrace called from " + to_string((int) hasher(this_thread::get_id())));
}
#endif

#ifdef DEBUG
void loggingThread (string filename){
	std::streambuf *backup;
	backup = std::cout.rdbuf(); 
	if (filename!=""){
		logFile.open (filename, fstream::out );
		clog.rdbuf(logFile.rdbuf());
	}
	while (!gottabekilled){
		if (numOfMessages <= 0) continue;
		messagesOccupied.lock();
		string str = Messages.front();
		Messages.pop();
		numOfMessages--;
		messagesOccupied.unlock();
		clog<<str<<endl;
		str = "";
		freeStringsOccupied.lock();
		freeStrings.push(str);
		freeStringsOccupied.unlock();
	}
	clog<<"Logger session ended.\n";
	clog.rdbuf(backup);
	logFile.close();

}
#endif

int logInit(LOG_LEVEL level, string filename){
	#ifdef DEBUG
	setLogLevel(level);
	numOfMessages =0;
	gottabekilled=0;
	try{
		logThread=thread(loggingThread, filename);
	}
	catch 	(exception& e){
		cout<<e.what()<<endl;
	}
	#endif	
	return 0;
}


void logEnd(){
	#ifdef DEBUG
	while(numOfMessages>0) continue;
	gottabekilled=1;
	logThread.join();
	#endif
}

#endif
