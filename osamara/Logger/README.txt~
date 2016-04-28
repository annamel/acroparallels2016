How to use logger:
	At the beginning of your program, call 
		logInit(level, logFilename);
	by level specifying level of your logging and by logFilename specifying name of file for log to be written into; if an empty string is passed, log will be written in clog. Acceptable log levels: LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR. If LOG_ERROR is passed, call trace will be printed.
	At the end of your program call 
		logEnd();
	If not called, unexpected behaviour is to be expected.
	To log a string, call
		log (level, message);
	To log an integer, call
		log (level, number);
	Keep in mind, that if current log level is less strict (detail), more detailed info won't be logged; i.e., if current log level is LOG_WARNING, call log(LOG_ERROR, 100); won't do anything.
	To change logging level, call
		setLogLevel(level);
	, by level specifying wanted log level among listed above.

How to build: 
	To build a release version from your code.cpp file, run command
		make release
	To specify file as <file>.cpp, run
		make release source=<file>
	To build a debug version from code.cpp, run
		make debug
	To build a debug from <file>.cpp, run
		make debug source=<file>
	To specify a name of your target file as <target>, add to any of commands listed above a target=<target> directive; i.e:
		make release source=SHM.cpp target=MAH_SHINEH_PROJECT
