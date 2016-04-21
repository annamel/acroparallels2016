target = release
source = main

release: release.o mf_api.o
	g++ -std=c++11 -lpthread -rdynamic -pthread $(source).o mf_api.o -o $(target)

release.o: $(source).cpp mapped_file_final_api.h
	g++ -std=c++11 -c -rdynamic -lpthread -pthread $(source).cpp mapped_file_final_api.h

mf_api.o: mapped_file.cpp mapped_file_final_api.h
	g++ -std=c++11 -c -rdynamic -lpthread -pthread mapped_file.cpp -o mf_api.o
	
debug: debug.o logger_d.o
	g++ -std=c++11 -lpthread -rdynamic -pthread -DDEBUG $(source).o logger_d.o -o debug

debug.o: $(source).cpp logger.h
	g++ -std=c++11 -c -rdynamic -lpthread -pthread -DDEBUG $(source).cpp logger.h

logger_d.o: logger.cpp logger.h
	g++ -std=c++11 -c -rdynamic -lpthread -pthread -DDEBUG logger.cpp -o logger_d.o

clean:
	rm *.o
	rm *.gch
