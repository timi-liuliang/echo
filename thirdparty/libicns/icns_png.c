/*
File:       icns_png.c
Copyright (C) 2001-2013 Mathew Eis <mathew@eisbox.net>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
Boston, MA 02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "icns.h"
#include "icns_internals.h"

#include <png.h>

typedef struct icns_png_io_ref {
	void*	data;
	size_t	size;
	size_t	offset;
} icns_png_io_ref;

static void icns_png_read_memory(png_structp png_ptr, png_bytep data, png_size_t length) {
	icns_png_io_ref* _ref = (icns_png_io_ref*) png_get_io_ptr( png_ptr );
	memcpy( data, (char*)_ref->data + _ref->offset, length );
	_ref->offset += length;
}

static void icns_png_write_memory(png_structp png_ptr, png_bytep data, png_size_t length) {
  icns_png_io_ref* _ref = (icns_png_io_ref*) png_get_io_ptr( png_ptr );
  size_t nsize = _ref->offset + length;

  /* allocate or grow buffer */
  if(_ref->data)
    _ref->data = realloc(_ref->data, nsize);
  else
    _ref->data = malloc(nsize);

  if(_ref->data == NULL)
    png_error(png_ptr, "Unable to allocate memory!");

  /* copy new bytes to end of buffer */
  //memcpy(_ref->data + _ref->offset, data, length);
  _ref->offset += length;
}

int icns_png_to_image(icns_size_t dataSize, icns_byte_t *dataPtr, icns_image_t *imageOut)
{
	int error = ICNS_STATUS_OK;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_uint_32 w;
	png_uint_32 h;
	png_bytep *rows;
	int bit_depth;
	int32_t color_type;
	int row;
	int rowsize;
	
	
	if(dataPtr == NULL)
	{
		icns_print_err("icns_png_to_image: JP2 data is NULL!\n");
		return ICNS_STATUS_NULL_PARAM;
	}
	
	if(imageOut == NULL)
	{
		icns_print_err("icns_png_to_image: Image out is NULL!\n");
		return ICNS_STATUS_NULL_PARAM;
	}
	
	if(dataSize == 0)
	{
		icns_print_err("icns_png_to_image: Invalid data size! (%d)\n",dataSize);
		return ICNS_STATUS_INVALID_DATA;
	}
	
	#ifdef ICNS_DEBUG
	printf("Decoding PNG image...\n");
	#endif

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if(png_ptr == NULL) {
		return ICNS_STATUS_NO_MEMORY;
	}

	info_ptr = png_create_info_struct(png_ptr);

	if(info_ptr == NULL) {
	   png_destroy_read_struct(&png_ptr, NULL, NULL);
	   return ICNS_STATUS_NO_MEMORY;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return ICNS_STATUS_INVALID_DATA;
	}

	// set libpng to read from memory
	icns_png_io_ref io_data = { dataPtr, dataSize, 0 };
	png_set_read_fn(png_ptr, (void *)&io_data, &icns_png_read_memory); 
	
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &w, &h, &bit_depth, &color_type, NULL, NULL, NULL);

	switch (color_type)
	{
		case PNG_COLOR_TYPE_RGB:
			if (bit_depth == 16) {
				png_set_strip_16(png_ptr);
				bit_depth = 8;
			}

			png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
			break;

		case PNG_COLOR_TYPE_RGB_ALPHA:
			if (bit_depth == 16) {
				png_set_strip_16(png_ptr);
				bit_depth = 8;
			}

			break;
	}
	
	png_read_update_info(png_ptr, info_ptr);
	
	rowsize = png_get_rowbytes(png_ptr, info_ptr);
	rows = malloc (sizeof(png_bytep) * h);

	imageOut->imageWidth = w;
	imageOut->imageHeight = h;
	imageOut->imageChannels = 4;
	imageOut->imagePixelDepth = 8;
	imageOut->imageDataSize = w * h * 4;
	imageOut->imageData = malloc( rowsize * h + 8 );

	if(imageOut->imageData == NULL) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		free(rows);
		return ICNS_STATUS_NO_MEMORY;
	}
	
	rows[0] = imageOut->imageData;
	for (row = 1; row < h; row++) {
		rows[row] = rows[row-1] + rowsize;
	}

	png_read_image(png_ptr, rows);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	free(rows);

	#ifdef ICNS_DEBUG
	if(error == ICNS_STATUS_OK) {
		printf("  decode result:\n");
		printf("  width is: %d\n",imageOut->imageWidth);
		printf("  height is: %d\n",imageOut->imageHeight);
		printf("  channels are: %d\n",imageOut->imageChannels);
		printf("  pixel depth is: %d\n",imageOut->imagePixelDepth);
		printf("  data size is: %d\n",(int)imageOut->imageDataSize);
	} else {
		printf("  decode failed.\n");
	}
	#endif
	
	return error;
}

int icns_image_to_png(icns_image_t *image, icns_size_t *dataSizeOut, icns_byte_t **dataPtrOut)
{
	int 			width = 0;
	int 			height = 0;
	int 			image_channels = 0;
	int			  image_pixel_depth = 0;
	png_structp 		png_ptr;
	png_infop 		info_ptr;
	png_bytep 		*row_pointers;
	int			i, j;
	
	if(image == NULL)
	{
		icns_print_err("icns_image_to_png: Image is NULL!\n");
		return ICNS_STATUS_NULL_PARAM;
	}
	
	if(dataSizeOut == NULL)
	{
		icns_print_err("icns_image_to_png: Data size NULL!\n");
		return ICNS_STATUS_NULL_PARAM;
	}
	
	if(dataPtrOut == NULL)
	{
		icns_print_err("icns_image_to_png: Data ref is NULL!\n");
		return ICNS_STATUS_NULL_PARAM;
	}

	if (image->pngFilename) {
		FILE *fp = fopen(image->pngFilename, "rb");
		if (fp) {
			fseek(fp, 0, SEEK_END);
			*dataSizeOut = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			*dataPtrOut = malloc(*dataSizeOut);
			if (fread(*dataPtrOut, 1, *dataSizeOut, fp)) {
				return ICNS_STATUS_OK;
			}
		}
	}

	#ifdef ICNS_DEBUG
	printf("Encoding PNG image...\n");
	#endif
	
	width = image->imageWidth;
	height = image->imageHeight;
	image_channels = image->imageChannels;
	image_pixel_depth = image->imagePixelDepth;
	
	png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	
	if (png_ptr == NULL)
	{
		fprintf (stderr, "PNG error: cannot allocate libpng main struct\n");
		return ICNS_STATUS_NO_MEMORY;
	}

	info_ptr = png_create_info_struct (png_ptr);

	if (info_ptr == NULL)
	{
		fprintf (stderr, "PNG error: cannot allocate libpng info struct\n");
		png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
		return ICNS_STATUS_NO_MEMORY;
	}

  icns_png_io_ref io_data = { NULL, 0, 0 };
	png_set_write_fn(png_ptr, (void *)&io_data, &icns_png_write_memory, NULL);
	
	png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
	
	png_set_IHDR (png_ptr, info_ptr, width, height, image_pixel_depth, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
	png_write_info (png_ptr, info_ptr);
	
	if(image_pixel_depth < 8)
		png_set_packing (png_ptr);
	
	row_pointers = (png_bytep*)malloc(sizeof(png_bytep)*height);
	
	if (row_pointers == NULL)
	{
		fprintf (stderr, "PNG error: unable to allocate row_pointers\n");
	}
	else
	{
		for (i = 0; i < height; i++)
		{
			if ((row_pointers[i] = (png_bytep)malloc(width*image_channels)) == NULL)
			{
				fprintf (stderr, "PNG error: unable to allocate rows\n");
				for (j = 0; j < i; j++)
					free(row_pointers[j]);
				free(row_pointers);
				return ICNS_STATUS_NO_MEMORY;
			}
			
			for(j = 0; j < width; j++)
			{
				icns_uint32_t *src_pixel = (icns_uint32_t*)&(image->imageData[i*width*image_channels+j*image_channels]);
				icns_uint32_t *dst_pixel = (icns_uint32_t*)&(row_pointers[i][j*image_channels]);
				*dst_pixel = *src_pixel;
			}
		}
	}
	
	png_write_image (png_ptr,row_pointers);
	
	png_write_end (png_ptr, info_ptr);
	
	*dataSizeOut = io_data.offset;
	*dataPtrOut = io_data.data;
	
	png_destroy_write_struct (&png_ptr, &info_ptr);
	
	for (j = 0; j < height; j++)
		free(row_pointers[j]);
	free(row_pointers);

	return ICNS_STATUS_OK;
}

