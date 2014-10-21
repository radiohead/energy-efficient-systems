
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

#ifndef JFIF_WRITER
#define JFIF_WRITER


void init_jfif_writer(char* file_name, int size_x, int size_y);
void close_jfif_writer();

void jfif_start_of_image();
void jfif_frame();
void jfif_define_huff_tables();
void jfif_define_quant_tables();
void jfif_scan();
void jfif_entropy_coded_segment();
void jfif_put_entropy_coded_bits(int n, int value);
void jfif_end_of_image();

void jfif_write_buffer_to_output_buffer(unsigned char *byte, int num_of_bytes);


#endif
