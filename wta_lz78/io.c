//
// Contains implemention of various IO functions involving buffers
//

#include "io.h"
#include "word.h"

#define BITS_IN_BYTE 8

extern uint64_t read_total;
extern uint64_t write_total;

// Static variables for keeping track of buffers across multiple functions
static uint8_t buffer[FOUR_KB];
static uint32_t total_bits = 0;
static uint32_t total_bytes = 0;

// Buffer specifically for read bytes in decode.c, len is how full it is
static uint8_t encoded[FOUR_KB];
static uint32_t encoded_len = 0;

//
// Reads in sizeof (FileHeader) bytes from the input file.
// These bytes are read into the supplied FileHeader, header.
// Endianness is swapped if byte order isn’t little endian.
//
// infile: File descriptor of input file to read header from.
// header: Pointer to memory where the bytes of the read header should go.
// returns: Void.
//
void read_header(int infile, FileHeader *header) {
  read_total += sizeof(header);
  read(infile, header, sizeof(header));
  if (is_big()) {
    header->magic = swap32(header->magic);
    header->protection = swap16(header->protection);
  }
  return;
}

//
// Writes sizeof (FileHeader) bytes to the output file.
// These bytes are from the supplied FileHeader, header.
// Endianness is swapped if byte order isn’t little endian.
//
// outfile: File descriptor of output file to write header to.
// header: Pointer to the header to write out.
// returns: Void.
//
void write_header(int outfile, FileHeader *header) {
  write_total += sizeof(header);
  write(outfile, header, sizeof(header));
  if (is_big()) {
    header->magic = swap32(header->magic);
    header->protection = swap16(header->protection);
  }
  return;
}

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
bool read_sym(int infile, uint8_t *sym) {
  int16_t bytes_read = read(infile, sym, 1);
  if (bytes_read > 0) {
    read_total += 1;
    return true;
  }
  return false;
}

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
void buffer_pair(int outfile, uint16_t code, uint8_t sym, uint8_t bitlen) {
  // Writes the code in variable bit length
  for (uint8_t i = 0; i < bitlen; i++) {
    if (total_bits >= FOUR_KB * BITS_IN_BYTE) {
      flush_pairs(outfile);
    }
    uint8_t bit_val = (code >> i) & 0x01;
    uint32_t byte_to_write = total_bits / BITS_IN_BYTE;
    uint8_t bit_pos_write = total_bits % BITS_IN_BYTE;
    uint8_t new_byte = buffer[byte_to_write] | (bit_val << bit_pos_write);
    buffer[byte_to_write] = new_byte;
    total_bits++;
  }
  // Writes the symbol as 8 individual bits
  for (uint8_t i = 0; i < BITS_IN_BYTE; i++) {
    if (total_bits >= FOUR_KB * BITS_IN_BYTE) {
      flush_pairs(outfile);
    }
    uint8_t bit_val = (sym >> i) & 0x01;
    uint32_t byte_to_write = total_bits / BITS_IN_BYTE;
    uint8_t bit_pos_write = total_bits % BITS_IN_BYTE;
    uint8_t new_byte = buffer[byte_to_write] | (bit_val << bit_pos_write);
    buffer[byte_to_write] = new_byte;
    total_bits++;
  }
}

//
// Writes out any remaining pairs of symbols and codes to the output file.
//
// outfile: File descriptor of the output file to write to.
// returns: Void.
//
void flush_pairs(int outfile) {
  if (total_bits >= FOUR_KB * BITS_IN_BYTE) {
    write_total += FOUR_KB;
    write(outfile, buffer, FOUR_KB);
  } else {
    write_total += (total_bits / BITS_IN_BYTE) + 1;
    write(outfile, buffer, (total_bits / BITS_IN_BYTE) + 1);
  }
  for (uint32_t i = 0; i < FOUR_KB; i++) {
    buffer[i] = 0;
  }
  total_bits = 0;
  return;
}

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
bool read_pair(int infile, uint16_t *code, uint8_t *sym, uint8_t bitlen) {
  // Reads in a variable bit length code bit by bit
  uint16_t temp = 0;
  for (uint8_t i = 0; i < bitlen; i++) {
    if (total_bits >= FOUR_KB * BITS_IN_BYTE || encoded_len == 0) {
      encoded_len = read(infile, encoded, FOUR_KB);
      if (encoded_len < 1) {
        return false;
      }
      read_total += encoded_len;
      total_bits = 0;
    }
    uint32_t byte_to_read = total_bits / BITS_IN_BYTE;
    uint8_t bit_pos = total_bits % BITS_IN_BYTE;
    uint8_t bit_to_read = (encoded[byte_to_read] >> bit_pos) & 0x01;
    temp = temp | (bit_to_read << i);
    total_bits++;
  }
  *code = temp;
  // Reads in a symbol bit by bit with a given length of 8
  uint8_t temp_sym = 0;
  for (uint8_t i = 0; i < BITS_IN_BYTE; i++) {
    if (total_bits >= FOUR_KB * BITS_IN_BYTE || encoded_len == 0) {
      encoded_len = read(infile, encoded, FOUR_KB);
      if (encoded_len < 1) {
        return false;
      }
      read_total += encoded_len;
      total_bits = 0;
    }
    uint32_t byte_to_read = total_bits / BITS_IN_BYTE;
    uint8_t bit_pos = total_bits % BITS_IN_BYTE;
    uint8_t bit_to_read = (encoded[byte_to_read] >> bit_pos) & 0x01;
    temp_sym = temp_sym | (bit_to_read << i);
    total_bits++;
  }
  *sym = temp_sym;
  return true;
}

//
// Buffers a Word, or more specifically, the symbols of a Word.
// Each symbol of the Word is placed into a buffer.
// The buffer is written out when it is filled.
//
// outfile: File descriptor of the output file to write to.
// w: Word to buffer.
// returns: Void.
//
void buffer_word(int outfile, Word *w) {
  for (uint32_t i = 0; i < w->len; i++) {
    if (total_bytes >= FOUR_KB) {
      flush_words(outfile);
    }
    buffer[total_bytes] = w->syms[i];
    total_bytes++;
  }
}

//
// Writes out any remaining symbols in the buffer.
//
// outfile: File descriptor of the output file to write to.
// returns: Void.
//
void flush_words(int outfile) {
  if (total_bytes >= FOUR_KB) {
    write_total += FOUR_KB;
    write(outfile, buffer, FOUR_KB);
  } else {
    write_total += total_bytes;
    write(outfile, buffer, total_bytes);
  }
  for (uint32_t i = 0; i < FOUR_KB; i++) {
    buffer[i] = 0;
  }
  total_bytes = 0;
  return;
}
