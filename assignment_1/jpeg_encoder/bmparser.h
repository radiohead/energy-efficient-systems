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


/*
 * Bitmap header
 * Bytes #0-1 store the data that will be used to identify the bitmap file.
 * Typical values for these 2 bytes are BM.
 * Bytes #2-5 store the size of the bitmap file using a dword.
 * Bytes #6-9 are reserved. Actual values depend on the application that creates the image.
 * Bytes #10-13 store the offset, i.e. starting address, of the byte where the bitmap data can be found.
 *
 * [edit] Bitmap information
 *
 * This block of bytes tells the application detailed information about the image, 
 * which will be used to display the image on the screen. It starts at byte #14 of the file.

 * Bytes #14-17 specify the header size. Values are: 40 - Windows V3, 12 - OS/2 V1, 64 - OS/2 V2, 
 * 108 - Windows V4, 124 - Windows V5
 * Bytes #18-21 store the bitmap width in pixels.
 * Bytes #22-25 store the bitmap height in pixels.
 * Bytes #26-27 store the number of color planes being used. Not often used.
 * Bytes #28-29 store the number of bits per pixel, which is the color depth of the image.
 * Typical values are 1, 4, 8, 24 and 32.
 * Bytes #30-33 define the compression method being used. Possible values are 0, 1, 2, 3, 4 and 5:
 * Bytes #34-37 store the image size. This is the size of the raw bitmap data (see below),
 * and should not be confused with the file size.
 * Bytes #38-41 store the horizontal resolution of the image.
 * Bytes #42-45 store the vertical resolution of the image.
 * Bytes #46-49 store the number of colors used.
 * Bytes #50-53 store the number of important colors used.
 * This number will be equal to the number of colors when every color is important.
 *
 */
#include "colorspace.h"

#ifndef BM_PARSER
#define BM_PARSER

typedef struct {
   unsigned short type;
   unsigned long size;
   unsigned short reserved1;
   unsigned short reserved2;
   unsigned long offsetbits;
} BITMAPFILEHEADER;

typedef struct {
   unsigned long size;
   unsigned long width;
   unsigned long height;
   unsigned short planes;
   unsigned short bitcount;
   unsigned long compression;
   unsigned long sizeimage;
   long xpelspermeter;
   long ypelspermeter;
   unsigned long colorsused;
   unsigned long colorsimportant;
} BITMAPINFOHEADER;


void bm_get_pixel_array(char* file_name, RGBPixel** pixel_array);
RGBPixel* bm_get_pixel_at(unsigned int x, unsigned int y, RGBPixel* pixel_array);
RGBPixel* bm_get_pixel(int index, RGBPixel* pixel_array);

#endif
