/* Copyright (c) 2006 Johan Ersfolk, Richard Westerb�ck, �bo Akademi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and 
 * associated documentation files (the "Software"), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial 
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/* This program takes as arguments 2 file names
 * the first is a name of an excisting bitmap-image-file
 * and the second is the name of output file. If the output
 * file exists it is overwritten.
 */

#include "jpeg.h"
#include "bmparser.h"
#include "colorspace.h"
#include "jfifwriter.h"
#include <stdio.h>
#include <malloc.h>


// these are created in bmparser
extern BITMAPFILEHEADER file_h;
extern BITMAPINFOHEADER info_h;


RGBPixel* rgb_pixel_array;


// Arrays to hold 8x8 blocks

int block_Y1[64];
int block_Cb1[64];
int block_Cr1[64];

int block_Y2[64];
int block_Cb2[64];
int block_Cr2[64];

int width, height;

/* A simple function which prints a 8x8 block 
 * to the screen, only used for testing 
 */
void print_block(int block[]) {
	int i;
	for (i = 0; i < 64; i++) {
		if ( (i % 8)==0 ) printf("\n");
		printf("%3d ", block[i]);
	}
	printf("\n");
}



/* Gets the block specified by index,
 * converts it to Y, Cb,Cr
 * and puts 8x8 blocks in the arrays
 */
void getBlock(int index, int Y[], int Cb[], int Cr[]) {
	unsigned int j = 0, i, x, y;
	unsigned int col = index % (width/8);
	unsigned int row = index / (width/8);

	for (y = 0; y < 8; y++)	{
		for (x = 0; x < 8; x++)	{
			i = 8*col + x + 8*row*width + y*width; /* source */
			RGB_to_Y(  bm_get_pixel(i, rgb_pixel_array), &Y[j]);
			RGB_to_Cb( bm_get_pixel(i, rgb_pixel_array), &Cb[j]);
			RGB_to_Cr( bm_get_pixel(i, rgb_pixel_array), &Cr[j]);
			j++;
		}
	}
}


/* This funtion simply reads the inputfile, runs the jpeg passes
 * and print the output file
 */
int main(int argc, char* argv[]){
	unsigned int i, num_of_pixels, num_of_blocks;

	if (argc < 3)	{
		printf("Usage: %s input-file output-file\n", argv[0]);
		return 1;
	}

	/* get a poniter to the first pixel in the array with rgb-values */
	bm_get_pixel_array(argv[1], &rgb_pixel_array); /* remember to free the array */

	/* the width and height must be a multiple of 8 */
	width = info_h.width;
	height = info_h.height;
	if (width %8) width  += 8-width%8;
	if (height%8) height += 8-height%8;
	num_of_pixels =  width * height;
	num_of_blocks = num_of_pixels/64;




	init_huffman_codes();

	init_jfif_writer(argv[2], info_h.width, info_h.height);

	// write headers to file
	jfif_start_of_image();
	jfif_define_quant_tables();
	jfif_frame();
	jfif_define_huff_tables();

	// write scan header - a scan is where the data goes
	jfif_scan();

	// a array for temp values
	int block[64];

	for(i = 0; i < num_of_blocks; i+=2) {

		// Get 2 vertically neighbouring blocks
		getBlock(i, block_Y1, block_Cb1, block_Cr1);
		getBlock(i+1, block_Y2, block_Cb2, block_Cr2);

		// Downsample the color components (4:2:2)
		downsample_vertical(block_Cb1, block_Cb2);
		downsample_vertical(block_Cr1, block_Cr2);

		// Y
		adjustMatrix(block_Y1);
		dct(block_Y1, block);
		quantization(block, block_Y1, LUMINANCE_BLOCK);
		zigzag(block_Y1, block);
		rlc_vlc_encode(block, LUMINANCE_BLOCK);

		// Y
		adjustMatrix(block_Y2);
		dct(block_Y2, block);
		quantization(block, block_Y2, LUMINANCE_BLOCK);
		zigzag(block_Y2, block);
		rlc_vlc_encode(block, LUMINANCE_BLOCK);

		//Cb
		adjustMatrix(block_Cb1);
		dct(block_Cb1, block);
		quantization(block, block_Cb1, CHROMINANCE_BLOCK_B);
		zigzag(block_Cb1, block);
		rlc_vlc_encode(block, CHROMINANCE_BLOCK_B);

		//Cr
		adjustMatrix(block_Cr1);
		dct(block_Cr1, block);
		quantization(block, block_Cr1, CHROMINANCE_BLOCK_R);
		zigzag(block_Cr1, block);
		rlc_vlc_encode(block, CHROMINANCE_BLOCK_R);
	}

	padd_entropy_coded_data();

	jfif_write_buffer_to_output_buffer(
		get_entropy_coded_section(),
		get_entropy_coded_section_size()
	);
	reset_entropy_coded_section();


	jfif_end_of_image();

	close_jfif_writer();

	free(rgb_pixel_array);

	return 0;
}


