# README.md

## Author
William Almy  
June 2020  

## Contents
- README.md
- Makefile
- encode.c
- decode.c
... and associated ADTs

## Build Instructions

Build an executable by executing the command "make" in the directory.  

Multiple arguments may be supplied to the program, from the following:
- "-v" : Verbose. Show statistics such as size and compression ratio.
- "-i" : Input File Specifier. Provide file name as next argument.
- "-o" : Output File Specifier. Provide filename as next argument.

## Use Intructions

Run the encode program by typing "./encode" and providing a file to encode.
The default file input for encode is STDIN. The default output is STDOUT.
Decode an encoded file by typing "./decode" and providing the encoded file.
Use the argument "-v" to see compression statistics including size and ratio.
If the output files for either program do not exist, they will be created.

- EX: ./encode -i README.md -o compressed.txt
- EX: ./decode -i compressed.txt -o README.txt
