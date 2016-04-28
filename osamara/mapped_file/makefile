
./out/libmappedfile.a: mapped_file.c mapped_file.h my_mapped_file.h
	mkdir -p out
	gcc -c mapped_file.c -o ./out/mf_api.o
	ar rc ./out/libmappedfile.a ./out/mf_api.o 
clean:
	rm ./out/*.o
	rm ./out/*.gch
	
