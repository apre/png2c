#if !defined(PNG2C_RAWFORMAT_HEADER_INCLUDED)
#define PNG2C_RAWFORMAT_HEADER_INCLUDED



typedef struct ImgData_s {
	unsigned long sx;
	unsigned long sy;
	unsigned long size_raw;
	const unsigned char * data;
} ImgData_t;

#endif
