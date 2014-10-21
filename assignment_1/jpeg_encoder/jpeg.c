/* Copyright (c) 2006 Johan Ersfolk, Richard Westerbäck, Åbo Akademi
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
 
#include <stdio.h>
#include "jpeg.h"
#include "costable.h"
#include "jfifwriter.h"
#include "dcttable.h"

static void build_huffman_codes(unsigned char*, 
                                unsigned short*, 
                                const unsigned char*, 
                                const unsigned char*);

static void write_to_buffer(unsigned char byte);

static const float PI = 3.14159265359;

unsigned char  huff_size_dc_luminance[12];
unsigned short huff_code_dc_luminance[12];
unsigned char  huff_size_dc_chrominance[12];
unsigned short huff_code_dc_chrominance[12];

unsigned char  huff_size_ac_luminance[256];
unsigned short huff_code_ac_luminance[256];
unsigned char  huff_size_ac_chrominance[256];
unsigned short huff_code_ac_chrominance[256];

int prev_dc_component_y = 0;
int prev_dc_component_cb = 0;
int prev_dc_component_cr = 0;

static int buffer = 0;
static int buffer_index = 32;

/*
 * WARNING: Does not work very well sometimes ;)
 */
unsigned char data_buffer[1000000];
int data_index = 0;


/* This function simply subtracts every value in the 
 * block by 128. This transforms the values from the range
 * 0 -> 255 to -128 -> +127
 */
void adjustMatrix(int block[]) {
  int i;
  for (i = 0; i < 64; i++) {
    block[i] -= 128;
  }
}



/* Discrete cosine transform
 * For more information see A.3.3
 */

/* This one is slow
void dct(int input[], int output[]) {
  int u, v, x, y;
  float f,CU,CV;
  float c = 0.707106781187; // 1/sqrt(2)
  for(u = 0; u < 8; u++) {
    for(v = 0; v < 8; v++) {
      f = 0;
      for(x = 0; x < 8; x++) {
        for(y = 0; y < 8; y++) {
          f += input[x*8 + y] *
            my_cos( ((2*x+1)*u*PI)/16 ) *my_cos( ((2*y+1)*v*PI)/16 ); 
        }
      }
      CU = (u==0) ? (c): 1;
      CV = (v==0) ? (c): 1;
      output[u*8+v] = my_round( 0.25*CU*CV*f );
    }
  }
}
*/

#define dct_lookup(x,y) \
	dct_lookup_table[(x)*8+(y)]

#define inner_dct(s,x,y,u,v) \
	(s[(x)*8+(y)]) * dct_lookup(u,x) * dct_lookup(v,y)

void dct(int input[], int output[]) {
	int u, v, x;
	float f,CU,CV;
	float c = 0.707106781187; /* 1/sqrt(2) */
	for(u = 0; u < 8; u++) {
		for(v = 0; v < 8; v++) {
			CU = (u==0) ? (c): 1;
			CV = (v==0) ? (c): 1;
			f = 0;
			for(x = 0; x < 8; x++) {
				f += inner_dct( input, x, 0 , u, v);
				f += inner_dct( input, x, 1 , u, v);
				f += inner_dct( input, x, 2 , u, v);
				f += inner_dct( input, x, 3 , u, v);
				f += inner_dct( input, x, 4 , u, v);
				f += inner_dct( input, x, 5 , u, v);
				f += inner_dct( input, x, 6 , u, v);
				f += inner_dct( input, x, 7 , u, v);
			}
			output[u*8+v] = my_round( 0.25*CU*CV*f );
		}
	}
}


/* Quantization. This is a lossy compression. Divides the elements of the current block
 * by some numbers and rounds the value to the closest integer.
 */
void quantization(int input[], int output[], int type) {
  int i;
  if (type == LUMINANCE_BLOCK) {
    for (i = 0; i < 64; i++) {
      output[i] = my_round((float)input[i]/(float)std_luminance_quant_tbl[i]);
    }
  } else {
    for (i = 0; i < 64; i++) {
      output[i] = my_round((float)input[i]/(float)std_chrominance_quant_tbl[i]);
    }
  }
}


/* Arrange the data in zigzag order.
 * This will place most of the non-zero values in the begining of the array
 */
void zigzag(int input[], int output[]) {
  int i;
  for(i = 0; i < 64; i++)
    output[i] = input[zigzag_table[i]];
}


/* Run length coding and Variable length coding (huffman coding).
 * The zigzag pass must be run before this. Type is one of the 
 * block types: LUMINANCE_BLOCK or CHROMINANCE_BLOCK
 */
void rlc_vlc_encode(int block[], int type) {
	int run, value = 0, nbits, code, mant, i;

	int		prev_dc_component = -1;
	unsigned char	*huff_size_dc = NULL;
	unsigned short	*huff_code_dc = NULL;
	unsigned char	*huff_size_ac = NULL;
	unsigned short	*huff_code_ac = NULL;


	if ( type == LUMINANCE_BLOCK) {
		huff_size_dc = huff_size_dc_luminance;
		huff_code_dc = huff_code_dc_luminance;
		huff_size_ac = huff_size_ac_luminance;
		huff_code_ac = huff_code_ac_luminance;
		prev_dc_component = prev_dc_component_y;
		prev_dc_component_y = block[0];
	} else if ( type == CHROMINANCE_BLOCK_B) {
		huff_size_dc = huff_size_dc_chrominance;
		huff_code_dc = huff_code_dc_chrominance;
		huff_size_ac = huff_size_ac_chrominance;
		huff_code_ac = huff_code_ac_chrominance;
		prev_dc_component = prev_dc_component_cb;
		prev_dc_component_cb = block[0];
	} else if ( type == CHROMINANCE_BLOCK_R) {
		huff_size_dc = huff_size_dc_chrominance;
		huff_code_dc = huff_code_dc_chrominance;
		huff_size_ac = huff_size_ac_chrominance;
		huff_code_ac = huff_code_ac_chrominance;
		prev_dc_component = prev_dc_component_cr;
		prev_dc_component_cr = block[0];
	}


	/* DC component */
	value = block[0] - prev_dc_component;
	if (value == 0) {
		put_bits(huff_size_dc[0], huff_code_dc[0]);
	} else {
		mant = value;
		if (value < 0) {
			value = -value;
			mant--;
		}
		nbits = bits_needed(value);

		put_bits(huff_size_dc[nbits], huff_code_dc[nbits]);

		put_bits(nbits, mant & ((1 << nbits) - 1));
	}


	/* AC component */
	run = 0;
	for(i = 1; i < 64; i++) {
		value = block[i];
		if (value == 0)   {
			run++;
		} else {
			/* if the run is larger than 15 put in this special marker for every 15th zero */
			while (run >= 16) {
				put_bits(huff_size_ac[0xf0], huff_code_ac[0xf0]);
				run -= 16;
			}
			mant = value;
			if (value < 0) {
				value = -value;
				mant--;
			}

			nbits = bits_needed(value);
			code = (run << 4) | nbits;
			/* first add the huffman code for the runlength-size pair */
			put_bits(huff_size_ac[code], huff_code_ac[code]);
			/* then put the amplitude of the coefficient */
			put_bits(nbits, mant & ((1 << nbits) - 1));

			run = 0;
		}
	}

	if (run > 1) {put_bits(huff_size_ac[0], huff_code_ac[0]);}
}

/* This function builds the different huffman tables from the 
 * huffman specification tables.
 */

void init_huffman_codes() {
  build_huffman_codes(huff_size_ac_luminance,
                      huff_code_ac_luminance,
                      bits_ac_luminance,
                      val_ac_luminance);
  build_huffman_codes(huff_size_ac_chrominance,
                      huff_code_ac_chrominance,
                      bits_ac_chrominance,
                      val_ac_chrominance);
  build_huffman_codes(huff_size_dc_luminance,
                      huff_code_dc_luminance,
                      bits_dc_luminance,
                      val_dc_luminance);
  build_huffman_codes(huff_size_dc_chrominance,
                      huff_code_dc_chrominance,
                      bits_dc_chrominance,
                      val_dc_chrominance);
}

/* This function build the huffman tables from the specifications given in the 
 * huffman specification tables (bits_table and val_table). 
 * This function is based on the JPEG standard annex c
 * the implementation is a combination of figures C.1, C.2 and C.3
 * The three loops is the three pictures in the standard is combined to one
 * to avoid the use of unnessesary arrays of temporary values
 */
static void build_huffman_codes(unsigned char *huff_size, unsigned short *huff_code,
                                 unsigned const char *bits_table, unsigned const char *val_table)
{
  int i, j, k = 0;
  int code = 0;
  
  for (i = 1; i <= 16; i++) {
    for (j = 0; j < bits_table[i]; j++) {
      huff_size[ val_table[k] ] = i;
      huff_code[ val_table[k] ] = code;
      code++;
      k++;
    }
    code <<= 1;
  }
}

/* This funtion adds the n least significant bits of the value to a stream
 * In this case the values are just sent to the jfif writer
 */
void put_bits(int n, int value) {
	value = value << (buffer_index - n);
	buffer = buffer | value;
	buffer_index -= n;
	while (buffer_index < 24) {
		unsigned char byte;
		byte = (buffer >> 24) & 0xFF;
		write_to_buffer(byte);
		if (byte == 0xFF)	{
			write_to_buffer( 0x00 ); /* padding */
		}
		buffer = buffer << 8;
		buffer_index += 8;
	}
}

/* This function calculates how many bits are needed to represent 
 * the specific value. This number combined with the zero-run-lengt
 * is the key to the huffman table. It is also used when the value 
 * is written to the stream.
 */
int bits_needed(int value) {
  int i;
  if (value < 0) value *= -1;
  for (i = 1;;i++) {
    if (value < (1 << i)) return i;
  } 
}


static void write_to_buffer(unsigned char byte) {
  /* write the byte to a file */
  data_buffer[data_index++] = byte;
}

/* use this to flush the buffer
 * If the buffer contains data, pad it with ones 
 * If the buffer is empty: do nothing
 */
void padd_entropy_coded_data() {
  if (buffer_index < 32) { 
    int value = 0xFF << (buffer_index - 8);
    unsigned char byte;
    buffer = buffer | value;
    byte = (buffer >> 24) & 0xFF;
    write_to_buffer(byte);
    if (byte == 0xFF) write_to_buffer(0x00);
  }
  buffer_index=32;
  buffer = 0;
}

int get_entropy_coded_section_size() {
  return data_index;
}

unsigned char* get_entropy_coded_section() {
  return data_buffer;
}

void reset_entropy_coded_section() {
  data_index = 0;
}

/* A cosine implementation using a lookup table,
 * 
 */
float my_cos( float angle ) {
  int a = (my_round(angle * 100))%628;
  if (a > 471)      {return cosine_lookup_table[628 - a];}
  else if (a > 314) {return -cosine_lookup_table[a - 314];}
  else if (a > 157) {return -cosine_lookup_table[314 - a];}
  return cosine_lookup_table[a];
}

int my_round( float f ) {
  double factor = (f<0)?(-0.5):0.5;
  return (int)(f+factor);
}


