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


Build on windows
================

Install from the gnuwin32 project:
   - zlib (developer package)
   - libpng
   - popt

Set the global envionment variable GNUWIN32 to the root where you installed gnuwin32 packages.
This must be the directory that contains the "lib", "include", "bin" subdisectories.

Then you should be able to build the visual studio project provided.
