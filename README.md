png2c
=====

a small fast tool to convert a png file into a C file that can be embedeed.



Build instruction
==================

Requierements: libpng

make should build the tool. 

gcc png2c.c -lpng -o png2c



Usage 
=====
 ./png2c tests/tux.png deleteme.png

