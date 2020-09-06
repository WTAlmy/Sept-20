//
// Header file for varios IO file read/write related functions
//

#ifndef __IO_H__
#define __IO_H__

#include "endian.h"
#include "word.h"

#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define FOUR_KB 0x1000

// Program's magic number
#define MAGIC 0x8badbeef

//
// Struct definition of a FileHeader.
//
// magic: Magic number indicating a file compressed by this program.
// protection: Protection / permissions of the original, uncompressed file.
//
typedef struct FileHeader {
  uint32_t magic;
  uint16_t protection;
} FileHeader;

//
// Reads in sizeof (FileHeader) bytes from the input file.
// These bytes are read into the supplied FileHeader, header.
// Endianness is swapped if byte order isn’t little endian.
//
// infile: File descriptor of input file to read header from.
// header: Pointer to memory where the bytes of the read header should go.
// returns: Void.
//
void read_header(int infile, FileHeader *header);

//
// Writes sizeof (FileHeader) bytes to the output file.
// These bytes are from the supplied FileHeader, header.
// Endianness is swapped if byte order isn’t little endian.
//
// outfile: File descriptor of output file to write header to.
// header: Pointer to the header to write out.
// returns: Void.
//
void write_header(int outfile, FileHeader *header);

//
// "Reads" a symbol from the input file.
// The "read" symbol is placed into the pointer to sym. (e.g. * sym = val )
//
// Returns true if there are symbols to be read, false otherwise.
//
// infile: File descriptor of input file to read symbols from.
// sym: Pointer to memory which stores the read symbol.
// returns: True if there are symbols to be read, false otherwise.
//
bool read_sym(int infile, uint8_t *sym);

//
// Buffers a pair. A pair is comprised of a code and a symbol.
// The code buffered has a bit - length of bitlen.
// The buffer is written out whenever it is filled.
//
// outfile: File descriptor of the output file to write to.
// code Code of the pair to buffer.
// sym: Symbol of the pair to buffer.
// bitlen: Number of bits of the code to buffer.
// returns: Void.
//
void buffer_pair(int outfile, uint16_t code, uint8_t sym, uint8_t bitlen);

//
// Writes out any remaining pairs of symbols and codes to the output file.
//
// outfile: File descriptor of the output file to write to.
// returns: Void.
//
void flush_pairs(int outfile);

//
// "Reads" a pair (code and symbol) from the input file.
// The "read" code is placed in the pointer to code (e.g. * code = val)
// The "read" symbol is placed in the pointer to sym (e.g. * sym = val).
//
// Returns true if there are pairs left to read in the buffer, else false.
// There are pairs left to read if the read code is not STOP_CODE.
//
// infile: File descriptor of the input file to read from.
// code: Pointer to memory which stores the read code.
// sym: Pointer to memory which stores the read symbol.
// bitlen: Length in bits of the code to read.
// returns: True if there are pairs left to read, false otherwise.
//
bool read_pair(int infile, uint16_t *code, uint8_t *sym, uint8_t bitlen);

//
// Buffers a Word, or more specifically, the symbols of a Word.
// Each symbol of the Word is placed into a buffer.
// The buffer is written out when it is filled.
//
// outfile: File descriptor of the output file to write to.
// w: Word to buffer.
// returns: Void.
//
void buffer_word(int outfile, Word *w);

//
// Writes out any remaining symbols in the buffer.
//
// outfile: File descriptor of the output file to write to.
// returns: Void.
//
void flush_words(int outfile);

#endif
