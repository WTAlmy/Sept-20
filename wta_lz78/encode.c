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

// Keeps track of how many bytes are written/read from for statistics
uint64_t read_total = 0;
uint64_t write_total = 0;

//
// Returns how many bits are required to represent a given number
//
// code: the number to represent
// returns: the number of bits required to represent the code
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

  // If no user choice is provided, default files are STDIN/OUT
  int32_t infile = STDIN_FILENO;
  int32_t outfile = STDOUT_FILENO;

  if (in_file_name != NULL) {
    infile = open(in_file_name, O_RDONLY);
    if (infile == -1) {
      printf("Unable to open input file specified.\n");
      return -1;
    }
  }

  // Get Permissions of Input File
  struct stat sb;
  int32_t result = fstat(infile, &sb);
  if (result == -1) {
    printf("Unable to read input file permissions.\n");
    return -1;
  }

  if (out_file_name != NULL) {
    outfile = open(out_file_name, O_WRONLY | O_CREAT | O_TRUNC, sb.st_mode);
    if (outfile == -1) {
      printf("Unable to open output file specified.\n");
      return -1;
    }
  }

  // Write Header to Output File
  struct FileHeader fh;
  memset(&fh, 0, sizeof(fh));

  fh.magic = MAGIC;
  fh.protection = sb.st_mode;
  write_header(outfile, &fh);

  // Main Compression Logic
  TrieNode *root = trie_create();
  TrieNode *curr_node = root;
  TrieNode *prev_node = NULL;
  uint8_t curr_sym = 0;
  uint8_t prev_sym = 0;
  uint16_t next_code = START_CODE;

  while (read_sym(infile, &curr_sym)) {
    if (curr_node != NULL) {
      TrieNode *next_node = trie_step(curr_node, curr_sym);
      if (next_node != NULL) {
        prev_node = curr_node;
        curr_node = next_node;
      } else {
        buffer_pair(outfile, curr_node->code, curr_sym, bit_len(next_code));
        curr_node->children[curr_sym] = trie_node_create(next_code);
        curr_node = root;
        next_code++;
      }
      if (next_code >= MAX_CODE) {
        trie_reset(root);
        curr_node = root;
        next_code = START_CODE;
      }
      prev_sym = curr_sym;
    }
  }

  // Output Incomplete Pair
  if (curr_node != root) {
    buffer_pair(outfile, prev_node->code, prev_sym, bit_len(next_code));
    next_code = (next_code + 1) % MAX_CODE;
  }

  // Output STOP_CODE
  buffer_pair(outfile, STOP_CODE, 0, bit_len(next_code));
  flush_pairs(outfile);

  if (display_stats) {
    float ratio = (float)1 - (float)write_total / read_total;
    ratio = ratio * (float)100.0;
    if (out_file_name != NULL) {
      fprintf(stderr, "Compressed file size: ");
      fprintf(stderr, "%" PRIu64 " bytes\n", write_total);
      fprintf(stderr, "Uncompressed file size: ");
      fprintf(stderr, "%" PRIu64 " bytes\n", read_total);
      fprintf(stderr, "Compression ratio: %2.2f%%\n", ratio);
    } else {
      printf("Compressed file size: %" PRIu64 " bytes\n", write_total);
      printf("Uncompressed file size: %" PRIu64 " bytes\n", read_total);
      printf("Compression ratio: %2.2f%%\n", ratio);
    }
  }

  // Cleanup
  close(infile);
  close(outfile);
  trie_delete(root);
  return 0;
}
