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

#include "colorspace.h"
#include <stdio.h>



/* JPEG-YCbCr (601) from "digital 8-bit R'G'B'  "
 * R, G, B    in {0, 1, 2, ..., 255}
 * Y, Cb, Cr  in {0, 1, 2, ..., 255}
 */
void RGB_to_YCbCr(RGBPixel* rgb, YCbCrPixel* ycc) {
  ycc->Y  = (int)(      + 0.299    * rgb->red + 0.587    * rgb->green + 0.114    * rgb->blue);
  ycc->Cb = (int)(128   - 0.168736 * rgb->red - 0.331264 * rgb->green + 0.5      * rgb->blue);
  ycc->Cr = (int)(128   + 0.5      * rgb->red - 0.418688 * rgb->green - 0.081312 * rgb->blue);
}


void RGB_to_Y(RGBPixel *rgb, int *Y) {
  *Y =   (int) (        0.299    * rgb->red + 0.587    * rgb->green + 0.114    * rgb->blue);
}

void RGB_to_Cb(RGBPixel *rgb, int *Cb) {
  *Cb =  (int) (128   - 0.168736 * rgb->red - 0.331264 * rgb->green + 0.5      * rgb->blue);
}

void RGB_to_Cr(RGBPixel *rgb, int *Cr) {
  *Cr =  (int) (128   + 0.5      * rgb->red - 0.418688 * rgb->green - 0.081312 * rgb->blue);
}


void downsample_vertical(int b1[], int b2[]) {
  int row;
  for (row = 0; row < 8; row++) {
    b1[row*8 + 0] = (b1[row*8 + 0] + b1[row*8 + 1])/2;
    b1[row*8 + 1] = (b1[row*8 + 2] + b1[row*8 + 3])/2;
    b1[row*8 + 2] = (b1[row*8 + 4] + b1[row*8 + 5])/2;
    b1[row*8 + 3] = (b1[row*8 + 6] + b1[row*8 + 7])/2;
    b1[row*8 + 4] = (b2[row*8 + 0] + b2[row*8 + 1])/2;
    b1[row*8 + 5] = (b2[row*8 + 2] + b2[row*8 + 3])/2;
    b1[row*8 + 6] = (b2[row*8 + 4] + b2[row*8 + 5])/2;
    b1[row*8 + 7] = (b2[row*8 + 6] + b2[row*8 + 7])/2;
  }
}

/* TODO: this filter is too simple */
void downsample_component(int *component, int width, int height) {
  int i;
  for (i = 0; i < width*height/2; i++) {
    component[i] =
      (component[(i*2)]+
         component[(i*2+1)]
       )/2;
  }
    
  /*for (y = 0; y < height/2; y++) {
    for (x = 0; x < width/2; x++) {
      component[x + y*width] =
        (component[(x*2) + (y*2)*width]+
         component[(x*2+1) + (y*2)*width]+
         component[(x*2) + (y*2+1)*width]+
         component[(x*2+1) + (y*2+1)*width]     
        )/4;
    }
  }*/
}
