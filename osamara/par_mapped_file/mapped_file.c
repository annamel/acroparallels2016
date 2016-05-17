
#include "mapped_file.h"
#include "my_mapped_file.h"
#include <stdio.h>
#include <sys/times.h>
#ifdef __cplusplus
extern "C"
{
#endif 

#define MF_MAP_FAILED NULL
uint64_t min (uint64_t a, uint64_t b){
	return a>=b?b:a;
}
uint64_t max (uint64_t a, uint64_t b){
	return a<=b?b:a;
}
mf_handle_t mf_open(const char* pathname){
	handle_t * mf = ( handle_t*) malloc(sizeof( handle_t));
	if (!mf){
		//printf("Alloc fail\n");
		//fflush(stdout);
		errno = ENOMEM;
		return NULL;
	}
	if ((mf->fd = open(pathname, O_RDWR, 0)) == -1){
		//printf("Open fail\n");	
		free (mf);
		return NULL;
	}
	pthread_rwlock_init(&mf->rwlock_lock, NULL);
	pthread_rwlock_init(&mf->rwlock_file, NULL);
	//printf("Open success\n");
	//fflush(stdout);
	//printf("---A\n");fflush(stdout);
	mf->mapped = 0;
	//printf("--A\n");fflush(stdout);
	mf->r_map = ( mapmem_handle_t *) malloc(sizeof( mapmem_handle_t));
	//printf("-A\n");fflush(stdout);
	mf->r_map->offset = 0;
	mf->r_map->size = 0;
	mf->r_map->ptr = NULL;
	mf->r_map->true_ptr = NULL;
	mf->r_map->refCounter = 0;
	mf->r_map->isRecent = 1;
	//printf("A\n");fflush(stdout);
	struct stat st;
	fstat(mf->fd, &st);
	mf->fsize = st.st_size;
	mf->r_map->true_ptr = mmap(NULL, st.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, mf->fd, 0);
	//printf("B\n");fflush(stdout);
	if (mf->r_map->true_ptr != MAP_FAILED){
		mf->r_map->size = st.st_size;
		mf->r_map->ptr = mf->r_map->true_ptr;
		mf->mapped = 1;
	}
	//printf("C\n");fflush(stdout);
	//file->file_size = st.st_size;
	//file->page_size = (size_t) sysconf(_SC_PAGE_SIZE);
	
	//printf("open took %d\n",time_ms()-time_start);
	return (mf_handle_t)mf;
}
	
int mf_close(mf_handle_t mf){
	if (!mf) return 0;
	handle_t * my_mf = ( handle_t *) mf;
	if (my_mf->fd == -1) return 0;
	close (my_mf->fd);
	free(my_mf->r_map);
	free(my_mf);
	pthread_rwlock_destroy(&((handle_t*)mf)->rwlock_lock);
	pthread_rwlock_destroy(&((handle_t*)mf)->rwlock_file);
	return 0;
}
// mapmem_handle_t *  mf->r_map = NULL;
ssize_t mf_read(mf_handle_t mf, void* buf, size_t size, off_t offset){
	
	if (!mf) return 0;
	if (((handle_t*)mf)->fd == -1) return 0;
	mf_mapmem_handle_t mapped_mem = NULL;
	handle_t * my_mf = (handle_t*) mf;
	void* buff = mf_map(mf, offset, size, &mapped_mem);
	
	mapmem_handle_t* mm = (mapmem_handle_t*)mapped_mem;
	
	//printf("or: size:%d\n offset: %d fsize: %d mapsize: %d", size,offset ,my_mf->fsize);
	//printf("mm: %d\n", mm);fflush(stdout);
	size = min(my_mf->fsize-offset, min(mm->size, size));
	pthread_rwlock_wrlock(&my_mf->rwlock_file);
	//printf("%d\n", size);
	memcpy(buf, buff, size);
	pthread_rwlock_unlock(&my_mf->rwlock_file);
	//printf("CC\n");fflush(stdout);
	mf_unmap(mf, mapped_mem);
	// if (offset + size > filesize) return max(0, filesize - offset); else return size        ---------offset---filesize---offset+size
	return (ssize_t) size; // min(size, max(0, filesize - (offset + size)))
}

ssize_t mf_write(mf_handle_t mf, const void* buf, size_t size, off_t offset){
	if (!mf) return 0;
	if (((handle_t*)mf)->fd == -1) return 0;
	mf_mapmem_handle_t mapped_mem;
	void* buff = mf_map(mf, offset, size, &mapped_mem);
	pthread_rwlock_wrlock(&((handle_t*)mf)->rwlock_file);
	memcpy(buff, buf, size);
	pthread_rwlock_unlock(&((handle_t*)mf)->rwlock_file);
	//mf_unmap(mf, mf_mapmem_hamdle_t);
	return (ssize_t) size;
}
/*off_t offset;
	size_t size;
	uint64_t refCounter;
	void* ptr, *true_ptr;*/
void* mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t* mapmem_handle){
	//printf("AA\n");fflush(stdout);
	if (!mf) return 0;
	if (((handle_t*)mf)->fd == -1) return 0;
	handle_t* my_mf = (handle_t*) mf;
	//printf("Mappin %d %d; mapped %d %d\n", (int)offset, (int)size, (int)my_mf->r_map->offset, (int)my_mf->r_map->size);
	*mapmem_handle = (void*) malloc (sizeof(mapmem_handle_t));
	mapmem_handle_t* my_mm = (mapmem_handle_t*) *mapmem_handle;
	pthread_rwlock_wrlock(&my_mf->rwlock_lock);
	my_mm->size = my_mf->r_map->size;
	my_mm->ptr = my_mf->r_map->ptr;
	my_mm->true_ptr = my_mf->r_map->true_ptr;
	my_mm->offset = my_mf->r_map->offset;
	//printf("BB\n");fflush(stdout);
	//*mapmem_handle = (void*)  my_mf->r_map;
	
	if (my_mf->mapped){
		my_mf->r_map->ptr = (void*)((char*)my_mf->r_map->true_ptr + offset);
		
		my_mm->ptr = my_mf->r_map->ptr;
		pthread_rwlock_unlock(&my_mf->rwlock_lock);
		return (void*) ( my_mf->r_map->ptr);
	}
	long long int pg_size=sysconf(_SC_PAGE_SIZE);
	
	long long int moffset = (offset/pg_size)*pg_size;
	long long int msize = min(max(size+offset%pg_size,pg_size*64*1024), mf_file_size(mf)-moffset);
	
	if (my_mf->r_map->offset <= moffset && my_mf->r_map->size + my_mf->r_map->offset >= offset + size){
		//printf("yay\n");
		my_mf->r_map->ptr = (void*)((char*) my_mf->r_map->ptr + (moffset - my_mf->r_map->offset));
		my_mm->ptr = my_mf->r_map->ptr;
		my_mm->r_map = my_mf->r_map;
		((mapmem_handle_t*)my_mf->r_map)->refCounter++;
		pthread_rwlock_unlock(&my_mf->rwlock_lock);
		return (void*) ( my_mf->r_map->ptr);
	}
	
	if(my_mf->r_map->offset != moffset || my_mf->r_map->size != msize){
		//HERE
		//printf("miss\n");
		my_mf->r_map->isRecent = 0;
		if(my_mf->r_map->refCounter <= 0){
			munmap((void*)my_mf->r_map->true_ptr, (size_t)my_mf->r_map->size);
		}
		my_mf->r_map = malloc(sizeof(mapmem_handle_t));
		my_mf->r_map->true_ptr = mmap(NULL, msize, PROT_READ|PROT_WRITE, 
			MAP_SHARED, my_mf->fd, moffset);
		my_mm->true_ptr = my_mf->r_map->true_ptr;
		if ( my_mf->r_map->true_ptr == MAP_FAILED){
			pthread_rwlock_unlock(&my_mf->rwlock_lock);
			return NULL;
		}
		my_mf->r_map->offset =  moffset;
		my_mf->r_map->isRecent = 1;
		my_mf->r_map->refCounter = 1;
		my_mf->r_map->size =  msize;
		my_mf->r_map->ptr = (char*) my_mf->r_map->true_ptr + offset%pg_size;
		my_mm->size = my_mf->r_map->size;
		my_mm->ptr = my_mf->r_map->ptr;
		my_mm->offset = my_mf->r_map->offset;
		my_mm->r_map = my_mf->r_map;
		//*mapmem_handle =  my_mf->r_map;
	}
	pthread_rwlock_unlock(&my_mf->rwlock_lock);
	return (void*) (my_mm->ptr);
}

int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle){
	if (!mf) return 0;
	if (((handle_t*)mf)->fd == -1) return 0;
	if (((handle_t*)mf)->mapped) return 0;
	if (!mapmem_handle)
		return -1;
	mapmem_handle_t* map_mem_handle = ( mapmem_handle_t*) mapmem_handle;
	if (!map_mem_handle)
		return -1;
	pthread_rwlock_wrlock(&((handle_t*)mf)->rwlock_lock);
	if (!map_mem_handle->r_map){free(mapmem_handle); return 0;}
	((mapmem_handle_t*)map_mem_handle->r_map)->refCounter--;
	if (((mapmem_handle_t*)map_mem_handle->r_map)->isRecent == 0 && ((mapmem_handle_t*)map_mem_handle->r_map)->refCounter <= 0){
		munmap((void*)map_mem_handle->true_ptr, (size_t)map_mem_handle->size);
		free(map_mem_handle->r_map);
	}
	free(mapmem_handle);
	//munmap((void*)map_mem_handle->true_ptr, (size_t)map_mem_handle->size);
	//if(map_mem_handle) {
	//	free(map_mem_handle);
	//}
	pthread_rwlock_unlock(&((handle_t*)mf)->rwlock_lock);
	return 0;
}

ssize_t mf_file_size(mf_handle_t mf){
	if (!mf) return 0;
	//printf("HOY");fflush(stdout);
	handle_t * my_mf = ( handle_t *) mf;
	//printf("AHOY");fflush(stdout);
	//printf("file %d\n", my_mf->fd);fflush(stdout);
	if (my_mf->fd == -1) return 0;
	struct stat buf;
	 
	fstat(my_mf->fd, &buf);
	return buf.st_size;
}
#ifdef __cplusplus
}
#endif // __cplusplus
