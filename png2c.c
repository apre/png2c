/* Original file from  http://zarb.org/~gc/html/libpng.html 
* Copyright 2002-2011 Guillaume Cottenceau and contributors.
*
* This software may be freely redistributed under the terms
* of the X11 license.
*
*/


/*

*/






//#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define PNG_DEBUG 3
#include <png.h>

//#include "png2c_raw_format.h"



typedef enum ColorTransform_e {
	UNKNOWN,
	RGB565
} ColorTransformation_t;








#define MAX_FNAME_LEN 2048
/** input png */
char  input_png[MAX_FNAME_LEN];

/** outout color mode for the conversion */
char  color_mode[MAX_FNAME_LEN];


/** outout png file */
char  output_png[MAX_FNAME_LEN];

/** C name of the image */
char  output_CNAME[MAX_FNAME_LEN];

/** outout C file */
char  output_C[MAX_FNAME_LEN];

/** outout h file */
char  output_h[MAX_FNAME_LEN];


/** Color Transformation to use. */
char  ColorTransform[MAX_FNAME_LEN];










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

/** the color transformation to use for the output C-file. */
ColorTransformation_t ColorTransformation;

/** number of bytes to describe the NbytePerNpixel*/
int NPixel;

/** number of bytes for npixel. */
int NbytePerNPixel;




void read_png_file(char* file_name)
{
	unsigned char header[8];    // 8 is the maximum size that can be checked
	png_color_16 my_background;
	png_color_16p image_background;

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
	printf("bit_depth: %d\n",bit_depth);
	printf("color_type: %d\n",color_type);


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



	/*

	my_background.red = 50;
	my_background.green=1;
	my_background.blue=100;



	if (png_get_bKGD(png_ptr, info_ptr, &image_background)) {
	printf("Input file has background: %x\n",image_background);
	png_set_background(png_ptr, image_background,
	PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
	}
	else{
	printf("Input file has no background\n");    
	png_set_background(png_ptr, &my_background,
	PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);


	}
	*/

	/* Expand any grayscale, RGB, or palette images to RGBA */
	//  png_set_expand(png_ptr);
	/* Reduce any 16-bits-per-sample images to 8-bits-per-sample */
	//  png_set_strip_16(png_ptr);


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
	int print_counter=0;

	/* Expand any grayscale, RGB, or palette images to RGBA */
	png_set_expand(png_ptr);
	/* Reduce any 16-bits-per-sample images to 8-bits-per-sample */
	png_set_strip_16(png_ptr);

	printf("Input image size: %dx%d = %x\n",width,height);



	for (y=0; y<height; y++) {


		png_byte* row = row_pointers[y];
		for (x=0; x<width; x++) {
			png_byte* ptr = &(row[x*4]);

			float ir, ig,ib,ia; // image color
			float or, og,ob,oa;   // output color
			float br,bg,bb,ba;  // background color.

				ir = ptr[0];
				ig = ptr[1];
				ib = ptr[2];
				ia = ptr[3]; // 1.0 is opaque, 0 is transparent



			switch (ColorTransformation) {

			case RGB565:

				or = ir / 255 * 31;
				og = ig / 255 * 63;
				ob = ib / 255 * 31;
				oa = ia;


				break;

			default:

				/* an attempt to alphacompose with background. */
				br = 0; 
				bg = 0;
				bb = 150;
				ba = 1; // background alpha

				ia/=255;

				oa = ia + ba * (1-ia);


				or = ((ir * ia )+ ba *  br * (1-ia))/(oa);
				og = ((ig * ia )+ ba *  bg * (1-ia))/(oa);
				ob = ((ib * ia )+ ba *  bb * (1-ia))/(oa);

				oa*=255;


				break;
			}






			if (print_counter<65 /*|| (ptr[3]!=255 &&  ptr[3]!=0)*/){   printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d - %d %.2f  %.2f  %.2f\n",
				x, y, ptr[0], ptr[1], ptr[2], ptr[3],or,og,ob);
			print_counter++;

			}

			ptr[0] = or; // modify output pixel.
			ptr[1] = og; // modify output pixel.
			ptr[2] = ob; // modify output pixel.

			ptr[3] = oa ; // modify output pixel.

		}
	}
}


void ConvertToC(char* file_name)
{
	FILE * pFile;
	int l_counter;

	

	png_color_16 my_background, *image_background;

	/* Expand any grayscale, RGB, or palette images to RGBA */
	//  png_set_expand(png_ptr);

	/* Reduce any 16-bits-per-sample images to 8-bits-per-sample */
	//   png_set_strip_16(png_ptr);

	// my_background.red = 0;
	//  my_background.green=255;
	//   my_background.blue=10;

	/*

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
	*/


	pFile = fopen (output_h,"w");
	if (pFile!=NULL) {

		fprintf(pFile,"/** generated by png2c from %s .\n *\n",input_png);
		fprintf(pFile,"  * @file  \n");
		fprintf(pFile,"  * %s array generated. \n",output_CNAME);
	    fprintf(pFile,"*/\n");

		fprintf(pFile,"\n\n\n");

		fprintf(pFile,"#include \"png2c_raw_format.h\"\n\n\n\n");


		fprintf(pFile,"extern const unsigned char %s_RAWDATA[]; /* forward declaration */\n",output_CNAME);
		
		fprintf(pFile,"extern const ImgData_t Img_%s;   /* Extern declaration  */\n",output_CNAME);



		
		fclose (pFile);
	}



	

	pFile = fopen (file_name,"w");
	if (pFile!=NULL) {
		fprintf(pFile,"/** generated by png2c from %s .\n *\n",input_png);
		fprintf(pFile,"  * @file  \n");
		fprintf(pFile,"  * %s array generated. \n",output_CNAME);
		
		fprintf(pFile,"*/\n");

		fprintf(pFile,"\n\n\n");

		fprintf(pFile,"#include \"png2c_raw_format.h\"\n\n\n\n");

		//fprintf(pFile,"const unsigned char %s_RAWDATA[]; /* forward declaration */\n",output_CNAME);
		fprintf(pFile,"const unsigned char   %s_RAWDATA[]={\n",output_CNAME);

		for (y=0; y<height; y++) {
			png_byte* row = row_pointers[y];
			l_counter = 0;
			for (x=0; x<width; x++) {

				int r,g,b;

				png_byte* ptr = &(row[x*4]);

				r = ptr[0];
				g = ptr[1];
				b = ptr[2];


				switch (ColorTransformation) {

				case RGB565:
					{
						unsigned long rg,gb;
						/* bitmask rgb components. */
						r &= 0x1f;
						g &= 0x3f;
						b &= 0x1f;

						rg = r << 3 | ( (g & 0x38) >> 3);
						gb = ((g & 0x07) << 5) | b& 0x1f;

								fprintf(pFile,"0x%x,0x%x,",rg, gb);
									l_counter+=1;

					}
					break;
				default:
					fprintf(pFile,"0x%x,0x%x,0x%x,",r, g, b);
					l_counter+=12;
					break;

				}





				if (l_counter>80) {
					l_counter = 0;
				
					fprintf(pFile,"\n\t",r, g, b);
				}





			}
			fprintf(pFile,"\n");
		}


		fprintf(pFile,"};");

		
		fprintf(pFile,"\n\n\n");

		fprintf(pFile,"const ImgData_t Img_%s= {\n",output_CNAME);
		fprintf(pFile,"%d, /* width */\n",width);
		fprintf(pFile,"%d, /* height */\n",height);
		//fprintf(pFile,"%d, /* height */\n",width*height);
		fprintf(pFile,"%d, /* size of data blob in byte */\n" ,NbytePerNPixel/NPixel*width*height);
		fprintf(pFile,"%s_RAWDATA /* RAW data of the image  */\n" ,output_CNAME);
		//fprintf(pFile," \n");
		//fprintf(pFile," \n");
		fprintf(pFile," };\n\n\n");





		fclose (pFile);
	} 



}


/** descsription of a command line parameter. */

typedef struct CommandLineParam_s {
	/* param string option*/
	char * param;
	/* target string where to store the param value.*/
	char * target;
}CommandLineParam_t ;


CommandLineParam_t AllowedParams[]= {
	{"--input=",input_png},
	{"--output=",output_png},
	{"--c_name=",output_CNAME},
	{"--color_depth:",output_C},


	{"",NULL},/* mandatory last entry. */

};

/** really brutal and simple command line and option parser.*/

void Analyse_commandline(int argc, char **argv) {

	int i;
	int pi;

	pi  = 0;

	while (AllowedParams[pi].target != NULL) {
		memset(AllowedParams[pi].target,0,MAX_FNAME_LEN);
		pi++;
	}

		memset(output_h,0,MAX_FNAME_LEN);
		memset(output_C,0,MAX_FNAME_LEN);


	for (i=0;i<argc;i++) {
		//printf("arg[%d]:%s\n",i,argv[i]);


		pi  = 0;

		while (AllowedParams[pi].target != NULL) {
			if (strstr(argv[i],AllowedParams[pi].param)!=NULL) {
				//	printf("param %s found\n",AllowedParams[pi].param);
				strcpy(AllowedParams[pi].target,argv[i]+strlen(AllowedParams[pi].param));
				printf("param %s has value %s\n",AllowedParams[pi].param,AllowedParams[pi].target);


			}

			pi++;

		} ;

	}


}

/** set default values for parameters or use the values from commandline.*/
void PrepareOptions() {

	/* */
	if (strlen(ColorTransform)==0 ) {
		ColorTransformation = RGB565;

	} else {
		ColorTransformation = RGB565;
	}


	/* set default config to 2 byte per 1 pixel */
	
 NbytePerNPixel=2;
 NPixel=1;


	strncat(output_C,output_CNAME,strlen(output_CNAME));
	strncat(output_C,".c",2);


	strncat(output_h,output_CNAME,strlen(output_CNAME));
	strncat(output_h,".h",2);

	

}


int main(int argc, char **argv)
{

	Analyse_commandline(argc,argv);

	if (strlen(input_png)==0 ||strlen(output_png)==0 ||strlen(output_CNAME)==0) {
		printf("ERROR: invalid parameters");
		printf("Usage: program_name --input=in_file.png --output=out.png --c_name=RAWimage --color_depth=RGB565 \n");
		//abort_("Usage: program_name <file_in> <file_out>");
		return 1;
	}

	PrepareOptions();

	printf("In:%s\n",input_png);
	printf("Out:%s\n",argv[2]);
	read_png_file(input_png);
	process_file();
	ConvertToC(output_C);
	write_png_file(output_png);

	return 0;
}

