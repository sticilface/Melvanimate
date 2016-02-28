/*

Thanks to Adafruit for their GFX and Matrix libs.  Modified here by Sticilface, aka Andrew Melvin.
Provides a callback method for pixel location! 
Creates class to set params using json. 

This is the core graphics library for all our displays, providing a common
set of graphics primitives (points, lines, circles, etc.).  It needs to be
paired with a hardware-specific library for each display device we carry
(to handle the lower-level functions).

Adafruit invests time and resources providing this open source code, please
support Adafruit & open-source hardware by purchasing products from Adafruit!

Copyright (c) 2013 Adafruit Industries.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/



#pragma once
#include <functional>
#include <Adafruit_GFX.h>
#include <ArduinoJson.h>


#define NEO_MATRIX_TOP         0x00 // Pixel 0 is at top of matrix
#define NEO_MATRIX_BOTTOM      0x01 // Pixel 0 is at bottom of matrix
#define NEO_MATRIX_LEFT        0x00 // Pixel 0 is at left of matrix
#define NEO_MATRIX_RIGHT       0x02 // Pixel 0 is at right of matrix
#define NEO_MATRIX_CORNER      0x03 // Bitmask for pixel 0 matrix corner
#define NEO_MATRIX_ROWS        0x00 // Matrix is row major (horizontal)
#define NEO_MATRIX_COLUMNS     0x04 // Matrix is column major (vertical)
#define NEO_MATRIX_AXIS        0x04 // Bitmask for row/column layout
#define NEO_MATRIX_PROGRESSIVE 0x00 // Same pixel order across each line
#define NEO_MATRIX_ZIGZAG      0x08 // Pixel order reverses between lines
#define NEO_MATRIX_SEQUENCE    0x08 // Bitmask for pixel line order

// These apply only to tiled displays (multiple matrices):

#define NEO_TILE_TOP           0x00 // First tile is at top of matrix
#define NEO_TILE_BOTTOM        0x10 // First tile is at bottom of matrix
#define NEO_TILE_LEFT          0x00 // First tile is at left of matrix
#define NEO_TILE_RIGHT         0x20 // First tile is at right of matrix
#define NEO_TILE_CORNER        0x30 // Bitmask for first tile corner
#define NEO_TILE_ROWS          0x00 // Tiles ordered in rows
#define NEO_TILE_COLUMNS       0x40 // Tiles ordered in columns
#define NEO_TILE_AXIS          0x40 // Bitmask for tile H/V orientation
#define NEO_TILE_PROGRESSIVE   0x00 // Same tile order across each line
#define NEO_TILE_ZIGZAG        0x80 // Tile order reverses between lines
#define NEO_TILE_SEQUENCE      0x80 // Bitmask for tile line order


//#define swap(a, b) { int16_t t = a; a = b; b = t; }

typedef std::function<void(uint16_t, int16_t, int16_t)> ShapeUpdateCallback; // Callback for drawing the pixels..

class Melvtrix: public Adafruit_GFX
{

public:
  // Constructor for single matrix:
  Melvtrix(int w, int h,
           uint8_t matrixType = NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS);

  // Constructor for tiled matrices:
  Melvtrix(uint8_t matrixW, uint8_t matrixH, uint8_t tX,
           uint8_t tY,
           uint8_t matrixType = NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS +
                                NEO_TILE_TOP + NEO_TILE_LEFT + NEO_TILE_ROWS);

  void drawPixel(int16_t x, int16_t y, uint16_t color); // colour is ignored here.. handle it in the actual function...
  void drawPixel(int16_t x, int16_t y) { drawPixel(x, y, 0); }
  int getPixel(uint16_t x, uint16_t y);

  void setShapeFn(ShapeUpdateCallback Fn);
private:

  const uint8_t
  type;
  const uint8_t
  matrixWidth, matrixHeight, tilesX, tilesY;
  ShapeUpdateCallback ShapeFn;

};


//  **  NOT being used yet ** // 
//  This adds ability to Melvtix_json to use json object to set params. 
class MelvtrixJson: public Melvtrix
{
public:
   bool addJson(JsonObject & root);
   bool parseJson(JsonObject & root);
private:
  uint8_t _matrixconfig;
};







