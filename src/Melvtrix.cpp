/*

Thanks to Adafruit for their GFX and Matrix libs.  Modified here by Sticilface, aka Andrew Melvin.

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

#include "melvtrix.h"

// Constructor for single matrix:
Melvtrix::Melvtrix(int w, int h,
                   uint8_t matrixType) : Adafruit_GFX(w, h),
  type(matrixType), matrixWidth(w),
  matrixHeight(h), tilesX(0), tilesY(0), ShapeFn(nullptr) { }

// Constructor for tiled matrices:
Melvtrix::Melvtrix(uint8_t mW, uint8_t mH, uint8_t tX,
                   uint8_t tY, uint8_t matrixType) :
  Adafruit_GFX(mW * tX, mH * tY), type(matrixType), matrixWidth(mW), matrixHeight(mH), tilesX(tX),
  tilesY(tY), ShapeFn(nullptr) { }

void Melvtrix::setShapeFn( ShapeUpdateCallback Fn)
{
  ShapeFn = Fn;
}

void Melvtrix::drawPixel(int16_t x, int16_t y, uint16_t color)
{

  uint16_t pixel = getPixel(x, y);

  if (ShapeFn) ShapeFn(pixel, x, y );

}

int Melvtrix::getPixel(uint16_t x, uint16_t y)
{

  if ((x < 0) || (y < 0) || (x >= _width) || (y >= _height)) return - 1;

  int16_t t;
  switch (rotation) {
  case 1:
    t = x;
    x = WIDTH  - 1 - y;
    y = t;
    break;
  case 2:
    x = WIDTH  - 1 - x;
    y = HEIGHT - 1 - y;
    break;
  case 3:
    t = x;
    x = y;
    y = HEIGHT - 1 - t;
    break;
  }

  int tileOffset = 0, pixelOffset;

  // if(remapFn) { // Custom X/Y remapping function
  //   pixelOffset = (*remapFn)(x, y);
  // } else {      // Standard single matrix or tiled matrices

  uint8_t  corner = type & NEO_MATRIX_CORNER;
  uint16_t minor, major, majorScale;

  if (tilesX) { // Tiled display, multiple matrices
    uint16_t tile;

    minor = x / matrixWidth;            // Tile # X/Y; presume row major to
    major = y / matrixHeight,           // start (will swap later if needed)
    x     = x - (minor * matrixWidth);  // Pixel X/Y within tile
    y     = y - (major * matrixHeight); // (-* is less math than modulo)

    // Determine corner of entry, flip axes if needed
    if (type & NEO_TILE_RIGHT)  minor = tilesX - 1 - minor;
    if (type & NEO_TILE_BOTTOM) major = tilesY - 1 - major;

    // Determine actual major axis of tiling
    if ((type & NEO_TILE_AXIS) == NEO_TILE_ROWS) {
      majorScale = tilesX;
    } else {
      adagfxswap(major, minor);
      majorScale = tilesY;
    }

    // Determine tile number
    if ((type & NEO_TILE_SEQUENCE) == NEO_TILE_PROGRESSIVE) {
      // All tiles in same order
      tile = major * majorScale + minor;
    } else {
      // Zigzag; alternate rows change direction.  On these rows,
      // this also flips the starting corner of the matrix for the
      // pixel math later.
      if (major & 1) {
        corner ^= NEO_MATRIX_CORNER;
        tile = (major + 1) * majorScale - 1 - minor;
      } else {
        tile =  major      * majorScale     + minor;
      }
    }

    // Index of first pixel in tile
    tileOffset = tile * matrixWidth * matrixHeight;

  } // else no tiling (handle as single tile)

  // Find pixel number within tile
  minor = x; // Presume row major to start (will swap later if needed)
  major = y;

  // Determine corner of entry, flip axes if needed
  if (corner & NEO_MATRIX_RIGHT)  minor = matrixWidth  - 1 - minor;
  if (corner & NEO_MATRIX_BOTTOM) major = matrixHeight - 1 - major;

  // Determine actual major axis of matrix
  if ((type & NEO_MATRIX_AXIS) == NEO_MATRIX_ROWS) {
    majorScale = matrixWidth;
  } else {
    adagfxswap(major, minor);
    majorScale = matrixHeight;
  }

  // Determine pixel number within tile/matrix
  if ((type & NEO_MATRIX_SEQUENCE) == NEO_MATRIX_PROGRESSIVE) {
    // All lines in same order
    pixelOffset = major * majorScale + minor;
  } else {
    // Zigzag; alternate rows change direction.
    if (major & 1) pixelOffset = (major + 1) * majorScale - 1 - minor;
    else          pixelOffset =  major      * majorScale     + minor;
  }

  return tileOffset + pixelOffset;

}



bool MelvtrixJson::addJson(JsonObject & root)
{
    JsonObject& color = root.createNestedObject("Matrix");

}

bool MelvtrixJson::parseJson(JsonObject & root)
{

  if (!root.containsKey("Matrix") ) 
  {
    return false; 
  }


  

}





