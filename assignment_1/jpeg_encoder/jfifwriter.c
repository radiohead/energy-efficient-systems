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

/* File for writing a simple jfif file
 * based on ISO/IEC 10918-1 Annex B
 */

#include <stdio.h>
#include "jfifwriter.h"
#include "jpeg.h"

static void flush_output_buffer();
static void write_to_file(unsigned char byte);


static char output_buffer[4096];
static int output_index = 0;

char img_width[2];
char img_height[2];

FILE *fp;


void init_jfif_writer(char* file_name, int size_x, int size_y) {
  fp = fopen(file_name, "wb");
  img_width[0] = size_x/0x100;
  img_width[1] = size_x%0x100;
  img_height[0]= size_y/0x100;
  img_height[1]= size_y%0x100;
}

void jfif_start_of_image() {
  write_to_file(0xFF); write_to_file(0xD8); // start of image
  write_to_file(0xFF); write_to_file(0xE0); // JFIF APP0 marker
  write_to_file(0x00); write_to_file(0x06); // APP0 segment length - might not be needed
  write_to_file(0x4A); // J
  write_to_file(0x46); // F
  write_to_file(0x49); // I
  write_to_file(0x46); // F
}

void jfif_define_quant_tables() {
  int i;
  // first we write the luminance table
  write_to_file(0xFF); write_to_file(0xDB); // marker
  write_to_file(0x00); write_to_file(0x43); // length of this segment
  write_to_file(0x00);      // Q-table precision(8 bit) and Q-table destination id 
  for(i = 0; i < 64; i++) { // write the table
    write_to_file(std_luminance_quant_tbl[i]);
  }
  
  // write the chrominance table 
  write_to_file(0xFF); write_to_file(0xDB); // marker
  write_to_file(0x00); write_to_file(0x43); // length of this segment
  write_to_file(0x01);      // Q-table precision(8 bit) and Q-table destination id 
  for(i = 0; i < 64; i++) { // write the table
    write_to_file(std_chrominance_quant_tbl[i]);
  }
}

void jfif_frame() {
  write_to_file(0xFF); write_to_file(0xC0); // start of frame for baseline
  write_to_file(0x00); write_to_file(0x11); // frame header length
  write_to_file(0x08);                      // Sample precision, 8 bit
  write_to_file(img_height[0]); write_to_file(img_height[1]); // Y: number of lines
  write_to_file(img_width[0]);  write_to_file(img_width[1]);  // X: number of samples per line
  write_to_file(0x03);                      // Nf: number of image components Y, Cb, Cr
  // Y component
  write_to_file(0x01);                      // C: Component id
  write_to_file(0x21);                      // H & V: Horizontal and Vertical sampling factor
  write_to_file(0x00);                      // Tq: Quantization table destination selector
  // Cb component
  write_to_file(0x02);                      // C: Component id
  write_to_file(0x11);                      // H & V: Horizontal and Vertical sampling factor
  write_to_file(0x01);                      // Tq: Quantization table destination selector
  // Cr component
  write_to_file(0x03);                      // C: Component id
  write_to_file(0x11);                      // H & V: Horizontal and Vertical sampling factor
  write_to_file(0x01);                      // Tq: Quantization table destination selector
   
}

void jfif_define_huff_tables() {
  int i;

  //  Dc luminance
  write_to_file(0xFF); write_to_file(0xC4); // Define Huffman table marker
  write_to_file(0x00); write_to_file(0x1F); // Lh - length of this segment 
  write_to_file(0x00);                      // Tc and Th - Table class(Dc=0, Ac=1) and Table Id
  for(i = 1; i < 17; i++) { // the BITS list
    write_to_file(bits_dc_luminance[i]);
  }
  for(i = 0; i < 12; i++) { // the HUFFVAL list
    write_to_file(val_dc_luminance[i]);
  }

  //  Ac luminance
  write_to_file(0xFF); write_to_file(0xC4); // Define Huffman table marker
  write_to_file(0x00); write_to_file(0xB5); // Lh - length of this segment 
  write_to_file(0x10);                      // Tc and Th - Table class(Dc=0, Ac=1) and Table Id
  for(i = 1; i < 17; i++) { // the BITS list
    write_to_file(bits_ac_luminance[i]);
  }
  for(i = 0; i < 162; i++) { // the HUFFVAL list
    write_to_file(val_ac_luminance[i]);
  }
  
  //  Dc chrominance
  write_to_file(0xFF); write_to_file(0xC4); // Define Huffman table marker
  write_to_file(0x00); write_to_file(0x1F); // Lh - length of this segment 
  write_to_file(0x01);                      // Tc and Th - Table class(Dc=0, Ac=1) and Table Id
  for(i = 1; i < 17; i++) { // the BITS list
    write_to_file(bits_dc_chrominance[i]);
  }
  for(i = 0; i < 12; i++) { // the HUFFVAL list
    write_to_file(val_dc_chrominance[i]);
  }
  
  //  Ac chrominance
  write_to_file(0xFF); write_to_file(0xC4); // Define Huffman table marker
  write_to_file(0x00); write_to_file(0xB5); // Lh - length of this segment
  write_to_file(0x11);                      // Tc and Th - Table class(Dc=0, Ac=1) and Table Id
  for(i = 1; i < 17; i++) { // the BITS list
    write_to_file(bits_ac_chrominance[i]);
  }
  for(i = 0; i < 162; i++) { // the HUFFVAL list
    write_to_file(val_ac_chrominance[i]);
  }
}


void jfif_scan() {
  write_to_file(0xFF); write_to_file(0xDA); // start of scan marker
  write_to_file(0x00); write_to_file(0x0C); // Ls - length of scan header c tidigare
  write_to_file(0x03);                      // Ns - number of image components   3 tidigare
  // Y
  write_to_file(0x01);                      // Cs - scan component selector
  write_to_file(0x00);                      // Td, Ta - DC & AC entropy coding table selector
  // Cb
  write_to_file(0x02);                      // Cs - scan component selector
  write_to_file(0x11);                      // Td, Ta - DC & AC entropy coding table selector
  // Cr
  write_to_file(0x03);                      // Cs - scan component selector
  write_to_file(0x11);                      // Td, Ta - DC & AC entropy coding table selector

  write_to_file(0x00);                      // Ss - start of spectral selection, 0 for baseline
  write_to_file(0x3f);                      // Se - end of spectral selection, 63 for baseline
  write_to_file(0x00);                      // Ah, Al - successive approximation.., not use here
}



void jfif_end_of_image() {
  write_to_file(0xFF); write_to_file(0xD9); // end of image
}


static void write_to_file(unsigned char byte) {
  // write the byte to a file
  output_buffer[output_index++] = byte;
  if(output_index >=4096) flush_output_buffer();
}

void jfif_write_buffer_to_output_buffer(unsigned char *byte, int num_of_bytes) {
  // write the byte to a file
  int i;
  for (i=0; i<num_of_bytes; i++) {
    output_buffer[output_index++] = byte[i];
    if(output_index >=4096) flush_output_buffer();
  }
}

static void flush_output_buffer() {
  fwrite( output_buffer, sizeof(char), output_index, fp);
  output_index = 0;
}

void close_jfif_writer() {
  flush_output_buffer();
  fclose(fp);
}
