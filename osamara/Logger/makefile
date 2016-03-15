target = release
source = code

release: release.o logger_r.o
	g++ -std=c++11 -lpthread -rdynamic -pthread $(source).o logger_r.o -o $(target)

release.o: $(source).cpp logger.h
	g++ -std=c++11 -c -rdynamic -lpthread -pthread $(source).cpp logger.h

logger_r.o: logger.cpp logger.h
	g++ -std=c++11 -c -rdynamic -lpthread -pthread logger.cpp -o logger_r.o
	
debug: debug.o logger_d.o
	g++ -std=c++11 -lpthread -rdynamic -pthread -DDEBUG $(source).o logger_d.o -o debug

debug.o: $(source).cpp logger.h
	g++ -std=c++11 -c -rdynamic -lpthread -pthread -DDEBUG $(source).cpp logger.h

logger_d.o: logger.cpp logger.h
	g++ -std=c++11 -c -rdynamic -lpthread -pthread -DDEBUG logger.cpp -o logger_d.o

clean:
	rm *.o
	rm *.gch
