""" Custom Bitwise File Loading / Saving Module

Reads or writes a variable-number of bits to a file regardless of word boundaries. Requires an input source or
an output file name. .PNG, .JPG, and other extensions are converted into .wta extensions. Reads and writes a
file header consisting of important compression information and a unique identification number to determine
whether a file has been compressed by this program.

It's fairly slow compared to reading full ints along word boundaries. Not meant for use outside of this project.

EX:                                 output.wta ->  00000000 | 00000000 | 00000000
bit_writer.buffer_bits(num = 0x3F, bitlen = 6) ->  11111100 | 00000000 | 00000000
bit_writer.buffer_bits(num = 0x00, bitlen = 4) ->  11111100 | 00000000 | 00000000
bit_writer.buffer_bits(num = 0xF0, bitlen = 8) ->  11111100 | 00000011 | 11000000
bit_writer.buffer_bits(num = 0xAA, bitlen = 6) ->  11111100 | 00000011 | 11101010
bit_writer.flush_bits(), bit_writer.close()

bit_reader = BitReader("output.wta")
x = bit.reader.read_bits(8)                    -> 0xFC
y = bit_reader.read_bits(16)                   -> 0x03EA

"""

import sys
import numpy as np

FOUR_KB = 0x1000
WORD_SIZE = 4
BITS_SIZE = 8

FILE_EXT = ".wta"
FILE_MAGIC = int("0xFFBEADFF", 16)


class BitWriter:
    """
    Creates a 4KB-buffered writer capable of writing with individual bits and variable bit length

    Usage:
    - bit_writer = BitWriter("unicorns")
    - bit_writer.write_header(100, 200, 4672)
    - bit_writer.write_bits(0xFFA, 12)
    - bit_writer.flush_bits()
    - bit_writer.close()
    """

    def __init__(self, filename):
        self.bits_in_buffer = 0
        self.buffer = np.zeros(FOUR_KB, dtype=np.uint8)
        try:
            self.outfile = open(filename + FILE_EXT, 'wb')
        except IOError:
            print("Error Opening File: " + filename + FILE_EXT)
            sys.exit(1)

    def write_int(self, num):
        # Writes a 32-bit number to file
        self.buffer_bits(num, WORD_SIZE * BITS_SIZE)

    def write_header(self, num_rows, num_cols, num_colors):
        # Writes 16-byte header to beginning of file
        self.write_int(FILE_MAGIC)
        self.write_int(num_rows)
        self.write_int(num_cols)
        self.write_int(num_colors)

    def buffer_bits(self, number, bitlen):
        # Writes a number with a given number of bits to the buffer; if buffer is full, flush it
        for i in range(bitlen):
            if self.bits_in_buffer >= FOUR_KB * BITS_SIZE:
                self.flush_buffer()
            current_buffer_bit = self.bits_in_buffer % BITS_SIZE
            current_buffer_byte = self.bits_in_buffer // BITS_SIZE
            extracted_bit = (number >> i) & 0x01
            extracted_byte = self.buffer[current_buffer_byte]
            self.buffer[current_buffer_byte] = (extracted_bit << current_buffer_bit) | extracted_byte
            self.bits_in_buffer += 1

    def flush_buffer(self):
        # Write the entire contents of the 4KB buffer to output file & clear for next use
        if self.bits_in_buffer < FOUR_KB * BITS_SIZE:
            self.buffer = self.buffer[:self.bits_in_buffer // BITS_SIZE + 1]
        self.buffer.tofile(self.outfile)
        self.buffer = np.zeros(FOUR_KB, dtype=np.uint8)
        self.bits_in_buffer = 0

    def close(self):
        self.outfile.close()


class BitReader:
    """
    Creates a BitReader capable of reading in a variable number of bits, regardless of word boundaries, and
    returning an integer representing the contents

    Usage:
    - bit_reader = BitReader("sample.wta")
    - rows, cols, colors = bit_reader.read_header()
    - rgbi = [bit_reader.read_bits(log2(colors)) for i in range(cols)]
    - bit_reader.close()
    """

    def __init__(self, filename):
        self.bits_read = 0
        try:
            self.infile = open(filename, 'rb')
            self.inbuffer = np.fromfile(self.infile, dtype=np.uint8, count=-1)
        except IOError:
            print("Error Opening File: " + filename)
            sys.exit(1)

    def read_int(self):
        # Returns next 32-bit integer from file
        return self.read_bits(WORD_SIZE * BITS_SIZE)

    def read_header(self):
        # Check File Compatibility
        magic = self.read_int()
        if magic != FILE_MAGIC:
            print("Magic: " + str(magic) + " != " + str(FILE_MAGIC))
            sys.exit(1)
        # Gather Compression Info
        num_rows = self.read_int()
        num_cols = self.read_int()
        num_colors = self.read_int()
        return num_rows, num_cols, num_colors

    def read_bits(self, bitlen):
        # Reads a variable number of bits through bitwise operations; stores result in returned value temp
        temp = 0
        for i in range(bitlen):
            current_buffer_bit = self.bits_read % BITS_SIZE
            current_buffer_byte = self.bits_read // BITS_SIZE
            extracted_byte = self.inbuffer[current_buffer_byte]
            extracted_bit = (extracted_byte >> current_buffer_bit) & 0x01
            temp = (extracted_bit << i) | temp
            self.bits_read += 1
        return temp

    def close(self):
        self.infile.close()
