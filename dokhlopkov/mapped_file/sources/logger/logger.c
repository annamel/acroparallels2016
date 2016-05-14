/*

 # logger source file
 #
 # Lang:     C
 # Author:   okhlopkov
 # Version:  0.3

 */

#include <stdint.h>
#include <unistd.h>  // for log file finder
#include <string.h> // for memset
#include <assert.h>
#include <stdio.h>

#include "logger.h"

/* PRIVATE FUNCTIONS */

// finds propriate file to write logs
static uint32_t _find_log_file(char *dst) {
  uint32_t current_file_number = 0;
  char current_file_already_exists = 1;

  // 10 is the length of the UINT32 max integer: 4294967295
  // 9 is for "logs/" + ".txt" + 7 for "logger/"
  int file_name_size = 10 + 9 + 7;

  char *current_file_name = (char *)calloc(file_name_size, sizeof(char));
  while (current_file_already_exists) {
    sprintf(current_file_name, "%s/logs/%u.txt", PATH_TO_LOGGER_FOLDER, current_file_number);
    if (access(current_file_name, F_OK) != -1) {
      // Log file exists, move to another number
      current_file_number += 1;
    } else {
      // File doesn't exists -> exit cycle
      FILE *output = fopen(current_file_name, "a");
      fclose(output);
      current_file_already_exists = 0;
    }
  }

  memcpy(dst, current_file_name, (file_name_size) * sizeof(char));
  free(current_file_name);
  return 0;
}

logger_t *logger_init() {
  logger_t *logger = (logger_t *)malloc(sizeof(logger_t));

  if (logger == NULL) {
    perror("Logger memory allocation error.\n");
    return NULL;
  }

  logger->buffer = (buffer_t *)malloc(sizeof(buffer_t));
  if (logger->buffer == NULL) {
    perror("Logger's buffer memory allocation error.\n");
    return NULL;
  }

  logger->buffer->data = (char *)calloc(BUFFERSIZE, sizeof(char));
  if (logger->buffer == NULL) {
    perror("Logger's buffer's data memory allocation error.\n");
    return NULL;
  }

  // 10 is the length of the UINT32 max integer: 4294967295
  // 9 is for "logs/" + ".txt" + 7 for "logger/"
  int file_name_size = 10 + 9 + 7;
  logger->log_file_name = (char *)calloc(file_name_size, sizeof(char));
  if (logger->log_file_name == NULL) {
    perror("Log file name data memory allocation error.\n");
    return NULL;
  }
  _find_log_file(logger->log_file_name);

  msg = (char *)calloc(MSGSIZE, sizeof(char));
  if (msg == NULL) {
    perror("Message memory allocation error.\n");
    return NULL;
  }

  logger->buffer->size = 0;
  return logger;
}

uint32_t logger_flush(logger_t *logger) {
  assert (logger != NULL);
  if (logger->buffer->size == 0) {
    return 0;
  }

  #ifdef ECHO
    printf("LOGGER:  Flushing into %s\n", logger->log_file_name);
  #endif

  FILE *output = fopen(logger->log_file_name, "a");
  if (output == 0) {
    printf("Can't open file %s\n", logger->log_file_name);
    return 1;
  }

  if (fputs(logger->buffer->data, output) < 0) {
    printf("Error with writing into %s\n", logger->log_file_name);
    return 1;
  }

  memset(logger->buffer->data, 0, logger->buffer->size * sizeof(char));
  logger->buffer->size = 0;

  fclose(output);
  return 0;
}

uint32_t logger_log(logger_t *logger, char *msg) {
  assert (logger != NULL);
  assert (msg != NULL);

  #ifdef ECHO
    printf("LOGMSG:  %s\n", msg);
  #endif

  uint32_t current_size = logger->buffer->size;

  if (current_size + (uint32_t)strlen(msg) >= BUFFERSIZE) {
    if (logger_flush(logger)) {
      printf("Can't flush.\n");
      return 1;
    }
  }

  int count = sprintf(&logger->buffer->data[current_size], "%s\n", msg);

  logger->buffer->size += count;
  return 0;
}

uint32_t logger_deinit(logger_t *logger) {
  assert (logger != NULL);
  if (logger_flush(logger)) printf("Can't flush while destruct.\n");
  free(logger->buffer->data);
  free(logger->buffer);
  free(logger->log_file_name);
  free(logger);
  if (msg != NULL) free(msg);
  return 0;
}

// destruct logger automatically
void cleanUp (void) __attribute__ ((destructor));

void cleanUp (void) {
  if (logger != NULL) logger_deinit(logger);
}
