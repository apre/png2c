/* Original file from  http://zarb.org/~gc/html/libpng.html 
 * Copyright 2002-2011 Guillaume Cottenceau and contributors.
 *
 * This software may be freely redistributed under the terms
 * of the X11 license.
 *
 */


/*

*/






#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define PNG_DEBUG 3
#include <png.h>

void abort_(const char * s, ...)
{
        va_list args;
        va_start(args, s);
        vfprintf(stderr, s, args);
        fprintf(stderr, "\n");
        va_end(args);
        abort();
}

int x, y;

int width, height, rowbytes;
png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;
png_bytep * row_pointers;

void read_png_file(char* file_name)
{
        unsigned char header[8];    // 8 is the maximum size that can be checked
          png_color_16 my_background, *image_background;


        /* open file and test for it being a png */
        FILE *fp = fopen(file_name, "rb");
        if (!fp)
                abort_("[read_png_file] File %s could not be opened for reading", file_name);
        fread(header, 1, 8, fp);
        if (png_sig_cmp(header, 0, 8))
                abort_("[read_png_file] File %s is not recognized as a PNG file", file_name);


        /* initialize stuff */
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

        if (!png_ptr)
                abort_("[read_png_file] png_create_read_struct failed");

        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
                abort_("[read_png_file] png_create_info_struct failed");

        if (setjmp(png_jmpbuf(png_ptr)))
                abort_("[read_png_file] Error during init_io");

        png_init_io(png_ptr, fp);
        png_set_sig_bytes(png_ptr, 8);

        png_read_info(png_ptr, info_ptr);

        width = png_get_image_width(png_ptr, info_ptr);
        height = png_get_image_height(png_ptr, info_ptr);
        color_type = png_get_color_type(png_ptr, info_ptr);
        bit_depth = png_get_bit_depth(png_ptr, info_ptr);

       // png_get_bKGD(png_ptr, info_ptr, &background); Get background color.

          my_background.red = 0;
       my_background.green=255;
        my_background.blue=10;



   if (png_get_bKGD(png_ptr, info_ptr, &image_background)) {
        printf("Input file has background: %x\n",image_background);
      png_set_background(png_ptr, image_background,
                         PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
}
   else{
       printf("Input file has no background: %x\n",my_background);    
      png_set_background(png_ptr, &my_background,
                         PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);

}

        number_of_passes = png_set_interlace_handling(png_ptr);
        png_read_update_info(png_ptr, info_ptr);


        /* read file */
        if (setjmp(png_jmpbuf(png_ptr)))
                abort_("[read_png_file] Error during read_image");

        row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);

        if (bit_depth == 16)
                rowbytes = width*8;
        else
                rowbytes = width*4;

        for (y=0; y<height; y++)
                row_pointers[y] = (png_byte*) malloc(rowbytes);

        png_read_image(png_ptr, row_pointers);

        fclose(fp);
}


void write_png_file(char* file_name)
{
        /* create file */
        FILE *fp = fopen(file_name, "wb");
        if (!fp)
                abort_("[write_png_file] File %s could not be opened for writing", file_name);


        /* initialize stuff */
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

        if (!png_ptr)
                abort_("[write_png_file] png_create_write_struct failed");

        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
                abort_("[write_png_file] png_create_info_struct failed");

        if (setjmp(png_jmpbuf(png_ptr)))
                abort_("[write_png_file] Error during init_io");

        png_init_io(png_ptr, fp);


        /* write header */
        if (setjmp(png_jmpbuf(png_ptr)))
                abort_("[write_png_file] Error during writing header");

        png_set_IHDR(png_ptr, info_ptr, width, height,
                     8, 6, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        png_write_info(png_ptr, info_ptr);


        /* write bytes */
        if (setjmp(png_jmpbuf(png_ptr)))
                abort_("[write_png_file] Error during writing bytes");

        png_write_image(png_ptr, row_pointers);


        /* end write */
        if (setjmp(png_jmpbuf(png_ptr)))
                abort_("[write_png_file] Error during end of write");

        png_write_end(png_ptr, NULL);

        /* cleanup heap allocation */
        for (y=0; y<height; y++)
                free(row_pointers[y]);
        free(row_pointers);

        fclose(fp);
}


void process_file(void)
{
        /* Expand any grayscale, RGB, or palette images to RGBA */
        png_set_expand(png_ptr);

	printf("Input image size: %dx%x = %x\n",width,height);

        /* Reduce any 16-bits-per-sample images to 8-bits-per-sample */
        png_set_strip_16(png_ptr);

        for (y=0; y<height; y++) {
                png_byte* row = row_pointers[y];
                for (x=0; x<width; x++) {
                        png_byte* ptr = &(row[x*4]);
                      /*  printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d - %d\n",
                               x, y, ptr[0], ptr[1], ptr[2], ptr[3]);*/

                        /* perform whatever modifications needed, for example to set red value to 0 and green value to the blue one:
                           ptr[0] = 0;
                           ptr[1] = ptr[2]; */
                }
        }
}


void ConvertToC(void)
{
  FILE * pFile;

  png_color_16 my_background, *image_background;
        
        /* Expand any grayscale, RGB, or palette images to RGBA */
        png_set_expand(png_ptr);

        /* Reduce any 16-bits-per-sample images to 8-bits-per-sample */
        png_set_strip_16(png_ptr);

       my_background.red = 0;
       my_background.green=255;
        my_background.blue=10;



   if (png_get_bKGD(png_ptr, info_ptr, &image_background)) {
        printf("Input file has background: %x\n",image_background);
      png_set_background(png_ptr, image_background,
                         PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
}
   else{
       printf("Input file has no background: %x\n",my_background);    
      png_set_background(png_ptr, &my_background,
                         PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);

}


        
  pFile = fopen ("myfile.c","w");
  if (pFile!=NULL) {
        fprintf(pFile,"/** generated by png2c\n*/\n");


          fprintf(pFile,"\n\n\n");
            fprintf(pFile,"const img[]={\n");

                    for (y=0; y<height; y++) {
                png_byte* row = row_pointers[y];
                for (x=0; x<width; x++) {

                        int r,g,b;

                        png_byte* ptr = &(row[x*4]);

                        r = ptr[0];
                        g= ptr[1];
                        b=ptr[2];



                        fprintf(pFile,"0x%x,0x%x,0x%x,",r, g, b);





                }
                fprintf(pFile,"\n");
        }


              fprintf(pFile,"};");




          fclose (pFile);
  } 


        for (y=0; y<height; y++) {
                png_byte* row = row_pointers[y];
                for (x=0; x<width; x++) {
                        png_byte* ptr = &(row[x*4]);
                      /*  printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d - %d\n",
                               x, y, ptr[0], ptr[1], ptr[2], ptr[3]);*/

                        /* perform whatever modifications needed, for example to set red value to 0 and green value to the blue one:
                           ptr[0] = 0;
                           ptr[1] = ptr[2]; */
                }
        }
}


int main(int argc, char **argv)
{
        if (argc != 3) {
                printf("Usage: program_name <file_in> <file_out> \n");
		//abort_("Usage: program_name <file_in> <file_out>");
		return 1;
}

        read_png_file(argv[1]);
        process_file();
        ConvertToC();
        write_png_file(argv[2]);

        return 0;
}
