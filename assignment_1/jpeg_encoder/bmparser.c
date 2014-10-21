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

/* A simple bit-map parser
 * The picture must be a 24-bit precision bitmap picture
 */
 
#include <stdio.h>
#include <malloc.h>
#include "bmparser.h"
#include "colorspace.h"

BITMAPFILEHEADER file_h;
BITMAPINFOHEADER info_h;

RGBPixel blackPixel; // used for padding the image size to a multiple of 8

/* Extracts an array of pixel structs from a file
 */
void bm_get_pixel_array(char* file_name, RGBPixel** pixel_array){
   blackPixel.red   = 0;
   blackPixel.green = 0;
   blackPixel.blue  = 0;
   
   FILE* input;
   unsigned char buffer[54];
   int num_pixels,i,j;

   input = fopen( file_name, "rb" );

   if (input == NULL)
     return;

   fread( buffer, 54, 1, input );

   file_h.size         =  (unsigned long)buffer[2];
   file_h.offsetbits   =  (int)buffer[10];
   info_h.width        =  (int)buffer[18] | ((int)buffer[19])<<8;
   info_h.height       =  (int)buffer[22] | ((int)buffer[23])<<8;

   /* Move the file pointer to the start of the pixel area */
   fseek( input, file_h.offsetbits, SEEK_SET );

   num_pixels = info_h.width * info_h.height;
   *pixel_array = ( RGBPixel* ) malloc ( sizeof( RGBPixel ) * num_pixels );
   
   int padding = 0;
   if ((info_h.width * 3) % 4) {
     padding = 4 - (info_h.width * 3) % 4; // must be multiple of 4
   }
   
   for(i=0; i< num_pixels ;i++){
      (*pixel_array)[i].blue  = (unsigned char)fgetc(input);
      (*pixel_array)[i].green = (unsigned char)fgetc(input);
      (*pixel_array)[i].red   = (unsigned char)fgetc(input);
      if ( (i % info_h.width) == info_h.width-1 ) { // remove padding bytes
        for (j=0; j<padding; j++) { fgetc(input); }
      }
   }
   
   fclose(input);
}


/*
 * Returns a pixel given the indices x= [0..m-1] y = [0..n-1] 
 * m,n = size in dim x ,dim y
 *
 */
RGBPixel* bm_get_pixel_at(unsigned int x, unsigned int y, RGBPixel* pixel_array){
   if (x >= info_h.width || y >= info_h.height) {return &blackPixel;} // instead of padding
   int width_with_padding = info_h.width;
   int index = ((info_h.height-1-(y)) * (width_with_padding) + x);
   return (pixel_array + index );
}

/* Returns the pixel in a natural order (with index 0 being the top/left pixel),
 * this function assumes that every row is a multiple of 8
 */
RGBPixel* bm_get_pixel(int index, RGBPixel* pixel_array){
  int width_with_padding = info_h.width;
  if (width_with_padding %8) width_with_padding  += 8-width_with_padding%8;
  return bm_get_pixel_at(index % width_with_padding, index / width_with_padding, pixel_array);
}




