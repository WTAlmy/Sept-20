""" decompress.py

Reads in a given compressed image file, converts it to a standard bitmap, and opens before/after for viewing.

Uses a BitReader from bit_io.py to read individual bits regardless of word boundaries.

Does NOT convert the decompressed image to another format. Decompresses for viewing only.

.wta File Format:
  <header: 16 bytes>
  <color_lookup_table: num_distinct_colors * 18 bits>
  <img_array: h * w * log2(num_distinct_colors) bits>

Each color in the lookup table usually represents a minimum of 6-8 pixels, with the average being 300-700 pixels, and
the maximum depending greatly on the image, typically ranging in the tens of thousands of pixels.

The program first reads in the file header to know how many rows, columns, and distinct colors the image has. Then,
it reads in 18 bits * the number of distinct colors, storing the colors in a dictionary in order they are read. Finally,
the program reads in individual pixel data at a variable bit length, calculated by how many bits would be needed to
individually represent each color in the color lookup table. For example, 9000 colors requires 13 bits. The 13 bits are
used as a key to lookup the color the pixel should be in the dictionary.

The color lookup dictionary may look something like this after being read:
- 1: [252, 148, 0]
- 2: [248, 144, 0]
- ...
- 572: [20, 252, 96]

The individual pixel data as read from the file may look like this after being read:
- [1, 1, 2, 5, 1, 2, 572, 876, 473, 64, 5, 3, 2, 1, 1, 2, 1, 2, 96, 473, 1213, 789, 35, 4, 3, 7, 5, 2, 1, 1, 2]

"""

import sys
import time
import math

import bit_io
import numpy as np
from PIL import Image
from threading import Thread

start_time = time.time()


########################################################################################################################

def log_time(label):
    # For Performance Profiling: outputs time in seconds between calls to log_time()
    global start_time
    print(label + ": " + str(time.time() - start_time))
    start_time = time.time()


def int_to_rgb(rgbi):
    # Converts a 18-bit integer, EX: [0x3FFFF] into an RGB value, EX: [0xFF, 0xFF, 0xFF]
    b = 2 + 4 * (rgbi & 0xFF)
    g = 2 + 4 * ((rgbi >> 6) & 0xFF)
    r = 2 + 4 * ((rgbi >> 12) & 0xFF)
    return [r, g, b]


########################################################################################################################

if len(sys.argv) == 1:
    print("Required: Input File Name")
    sys.exit(1)

bit_reader = bit_io.BitReader(sys.argv[1])
num_rows, num_cols, num_colors = bit_reader.read_header()

print("Retrieved Data: " + str(num_colors) + " colors, " + str(num_rows) + "x" + str(num_cols))
log_time("Decompress Initialized")

########################################################################################################################

rank_color = dict()
for rank in range(num_colors):
    rank_color[rank] = bit_reader.read_bits(18)

log_time("Lookup Table Generated")

########################################################################################################################

required_bits = math.ceil(math.log(num_colors, 2))
print("Stored Bits Per Cell: " + str(required_bits))

i = 0
new_image = []
for y in range(num_rows):
    new_row = []
    print("  " + str(100 * i / num_rows)[:5], end="%\r")
    for x in range(num_cols):
        rgb = int_to_rgb(rank_color[bit_reader.read_bits(required_bits)])
        new_row.append(rgb)
    new_image.append(new_row)
    i += 1

log_time("Image Data Recreated")

########################################################################################################################
"""
Why multithreaded? 
Because according to a Google search, Pillow won't open two images on the same thread.
"""


def async_show(img):
    img.show()


reconstructed_array = np.array(new_image)
reconstructed_image = Image.fromarray(reconstructed_array.astype('uint8'), 'RGB')

t1 = Thread(target=async_show, args=(reconstructed_image,))
t1.start()

try:
    # Guessing source file is PNG
    original_name, file_ext = sys.argv[1].split('.', maxsplit=1)
    original_image = Image.open(original_name + ".png")
    # Open on new thread
    t2 = Thread(target=async_show, args=(original_image,))
    t2.start()
except IOError:
    print("Failed to open original file for comparison")
    sys.exit(1)

########################################################################################################################
