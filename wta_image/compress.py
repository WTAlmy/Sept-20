""" compress.py

Takes a standard compressed image file such as .png and .jpg, and converts it into a .wta compressed file type.

Uses a BitWriter from bit_io.py to write individual bits regardless of word boundaries.

The program begins by looping through each pixel in the image, converting it to single integer, and setting that integer
as a key in a color frequency dictionary with a value of 1. Each time that color is found in the image, it continues to
increment the same pixel value. After looping through the image, the number of distinct colors and their frequency is
known. The number of distinct colors is reduced by grouping Red, Green, and Blue values within (3/255) of each other.
The dictionary is then sorted by frequency and each color is given an associated frequency rank.

.wta File Format:
  <header: 16 bytes>
  <color_lookup_table: num_distinct_colors * 18 bits>
  <img_array: h * w * log2(num_distinct_colors) bits>

After writing the file header, the program begins by writing each color from the frequency table in order. This means
the most common colors in the image are written first, and the least common are written last. The file can be read later
and assume that the first color it reads has a ranking of 1.

The color-frequency table may look like this afterwards:  (RGB values converted into integer keys)
- [252, 148, 0] : 1
- [248, 144, 0] : 2
- ...
- [20, 252, 96] : 572

Following the frequency table, the image is again iterated over and it's RGB value is used as a key to the frequency
ranking table. EX: rankings[242, 140, 0] = 1. The number 1 is written to the file at a variable bit length, depending on
the number of distinct colors. If an image has 9,000 distinct colors, for example, 13 bits are written to distinguish.

The individual pixel data written to the file may look like this in the end:
- [1, 1, 2, 5, 1, 2, 572, 876, 473, 64, 5, 3, 2, 1, 1, 2, 1, 2, 96, 473, 1213, 789, 35, 4, 3, 7, 5, 2, 1, 1, 2]

Uses a BitWriter from bit_io.py to write individual bits regardless of word boundaries.

"""

import sys
import time
import math
from collections import defaultdict

import bit_io
import numpy as np
from PIL import Image

start_time = time.time()

########################################################################################################################

def log_time(label):
    global start_time
    print(label + ": " + str(time.time() - start_time))
    start_time = time.time()

def rgb_to_int(pixel):
    r, g, b = pixel[0] // 4, pixel[1] // 4, pixel[2] // 4
    return (r << 12) + (g << 6) + b

########################################################################################################################

if len(sys.argv) < 2:
    print("Format: python3 compress.py source_file.png")
    sys.exit(1)

try:
    img = Image.open(sys.argv[1])
except IOError:
    print("Invalid Image Path Provided: " + sys.argv[1])
    sys.exit(1)

########################################################################################################################

# Loop Through Pixels, Recording Color Totals
arr = np.asarray(img, dtype=np.int32)
pixels = defaultdict(int)
for row in arr:
    for pixel in row:
        pixels[rgb_to_int(pixel)] += 1

log_time("Created Color Frequency Table")

########################################################################################################################

pixels_sorted = sorted(pixels, key=pixels.get, reverse=True)

pos = 0
rankings = dict()

for rgbi in pixels_sorted:
    rankings[rgbi] = pos
    pos += 1

log_time("Sorted Frequency Table")

########################################################################################################################

file_name, file_ext = sys.argv[1].split('.', maxsplit=1)
bit_writer = bit_io.BitWriter(file_name)

num_rows = len(arr)
num_cols = len(arr[0])
num_colors = len(pixels_sorted)
bit_writer.write_header(num_rows, num_cols, num_colors)

log_time("File Prepared")

########################################################################################################################

# Write Lookup Table
for rgbi in pixels_sorted:
    bit_writer.buffer_bits(rgbi, 18)

log_time("Wrote Lookup Table")

########################################################################################################################

required_bits = math.ceil(math.log(num_colors, 2))
print("Required Bits Per Cell: " + str(required_bits))

i = 0
for row in arr:
    print("  " + str(100 * i / num_rows)[:5],end="%\r")
    for pixel in row:
        rank = rankings[rgb_to_int(pixel)]
        bit_writer.buffer_bits(rank, required_bits)
    i += 1

bit_writer.flush_buffer()
bit_writer.close()

log_time("Wrote Image Data")

########################################################################################################################
