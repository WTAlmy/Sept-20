# compress.py
This program converts .png, .jpg images into .wta images.

It is important to note that the "compressed" .wta file will likely have a larger file size compared to the source image
files. This is because the source PNG and JPG images are already compressed. To compare them, the original file size is 
roughly equivalent to (width * height * 3 bytes).

# Notes
Compressed image size relative to uncompressed / TIFF format hovers around 40%-80%. This makes the compression ratio
of this program 1.25-2.5 : 1. This is compared to PNG's typical compression average of 2.7 : 1, and JPEG's of 10 : 1.

However, perhaps a better environment for this program would be C or C++. 80-85% of program run time is spent on bitwise
operations and array lookups, which in my experience tend to be much faster in those languages. This could be reduced by
writing full bytes to the disk rather than 18-bit and 9-13 bit values, however, it would diminish some of the possible
compression ability. Prior to bitwise reading/writing, program performed 8-10x faster.

The most effective possible improvement would be limiting the number of distinct colors. However, reducing this by too
much has the adverse effect of possibly eliminating small details, such as eye color. If someone's eye was the only
source of a given color in an image, for example light blue making up only 20 pixels of a 1mil pixel image, it could be
cut out and replaced by the nearest color.

# Usage
python3 compress.py file_name.png
python3 decompress.py file_name.wta
