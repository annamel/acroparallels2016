#include <fcntl.h>

#include "mapped_file.h"
#include "chunks.h"


int mf_open(const char* name, size_t max_memory, mf_handle_t* mf) {
  if (name == NULL || mf == NULL) return EINVAL;

  int fd = open(name, O_RDWR, S_IWRITE | S_IREAD);
  if (fd == -1) return errno;
  // *mf = (void *) ____;
  return 0;
}

int mf_close(mf_handle_t mf) {
  if (!mf) return EAGAIN;
  return 0; // TODO: fix
}

int mf_read(mf_handle_t mf, off_t offset, size_t size, void* buf) {
  return 0;
}

int mf_write(mf_handle_t mf, off_t offset, size_t size, void* buf) {
  return 0;
}
