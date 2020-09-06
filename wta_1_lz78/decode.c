#include "code.h"
#include "io.h"
#include "math.h"
#include "trie.h"
#include "word.h"

#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define OPTIONS "vi:o:"

// Keep track of how many total bytes are written/read for statistics
uint64_t read_total = 0;
uint64_t write_total = 0;

//
// Calculates how many bits are required to represent a given code
//
// code: the code to represent
// returns: how many bits are required to represent the code
//
uint16_t bit_len(uint16_t code) {
  if (code == 0) {
    return 1;
  } else {
    return log2(code) + 1;
  }
}

//
// Default entry to program
//
int main(int argc, char **argv) {

  // Default values for program arguments
  bool display_stats = false;
  char *in_file_name = NULL;
  char *out_file_name = NULL;

  char c = 0;
  while ((c = getopt(argc, argv, OPTIONS)) != -1) {
    if (c == 'v') {
      display_stats = true;
    } else if (c == 'i') {
      in_file_name = optarg;
    } else if (c == 'o') {
      out_file_name = optarg;
    }
  }

  // Default values for input/output if a user does not provide them
  int32_t infile = STDIN_FILENO;
  int32_t outfile = STDOUT_FILENO;

  if (in_file_name != NULL) {
    infile = open(in_file_name, O_RDONLY);
    if (infile == -1) {
      printf("Unable to open input file specified.\n");
      return -1;
    }
  }

  // Read File Header from Input File
  struct FileHeader fh;
  memset(&fh, 0, sizeof(fh));
  read_header(infile, &fh);

  // Check if file has been compressed by this program
  if (fh.magic != MAGIC) {
    printf("Provided Magic: %" PRIu32 "\n", fh.magic);
    printf("Input file specified has an invalid magic number.\n");
    return -1;
  }

  // Create output file if it does not exist, using input file's protection
  if (out_file_name != NULL) {
    outfile = open(out_file_name, O_WRONLY | O_CREAT | O_TRUNC, fh.protection);
    if (outfile == -1) {
      printf("Unable to open output file specified.\n");
      return -1;
    }
  }

  // Main Decompression Logic
  WordTable *table = wt_create();
  uint8_t curr_sym = 0;
  uint16_t curr_code = 0;
  uint16_t next_code = START_CODE;
  while (read_pair(infile, &curr_code, &curr_sym, bit_len(next_code))) {
    if (curr_code == STOP_CODE) {
      break;
    }
    table[next_code] = word_append_sym(table[curr_code], curr_sym);
    buffer_word(outfile, table[next_code]);
    next_code = next_code + 1;
    if (next_code >= MAX_CODE) {
      wt_reset(table);
      next_code = START_CODE;
    }
  }
  flush_words(outfile);

  if (display_stats) {
    printf("Compressed file size: %" PRIu64 " bytes\n", read_total);
    printf("Uncompressed file size: %" PRIu64 " bytes\n", write_total);
    float ratio = 100.0 * ((float)1 - ((float)read_total / (float)write_total));
    printf("Compression ratio: %2.2f%%\n", ratio);
  }

  // Cleanup
  close(infile);
  close(outfile);
  wt_delete(table);
  return 0;
}
