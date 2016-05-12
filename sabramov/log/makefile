logger: main.o logger.o
	gcc main.o logger.o -o logger
	
main.o: main.c
	gcc -c -DDEBUG_MODE -rdynamic main.c
	
logger.o: logger.c
	gcc -c -DDEBUG_MODE -rdynamic logger.c
