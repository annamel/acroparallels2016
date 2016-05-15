#include "mapped_file.h"
#include "../libmf/mfdef.h"

int main() {
	mf_handle_t mf = mf_open("test_file");
	if(mf == MF_OPEN_FAILED) {
		return 2;
	};

	mf_mapmem_handle_t mm_handle;

	for(int i = 0; i < 2; i++) {
		if(!mf_map(mf, i*256*1024*1024, 1024, &mm_handle)) {
			return 3;
		}
	}

	for(int j = 0; j < 1024 * 1024; j++) {
		for(int i = 0; i < 60; i++) {
			if(!mf_map(mf, i*1024*8, (512 * 1024 - 2*i*8)*1024, &mm_handle)) {
				return 3;	
			}
		}
	}

	if(mf_close(mf)) {
		return 4;
	}

	return 0;
}
