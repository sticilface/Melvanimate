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

  if (ShapeFn) { ShapeFn(pixel, x, y ); }

}

int Melvtrix::getPixel(uint16_t x, uint16_t y)
{

  if ((x < 0) || (y < 0) || (x >= _width) || (y >= _height)) { return - 1; }

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
    if (type & NEO_TILE_RIGHT) { minor = tilesX - 1 - minor; }
    if (type & NEO_TILE_BOTTOM) { major = tilesY - 1 - major; }

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
  if (corner & NEO_MATRIX_RIGHT) { minor = matrixWidth  - 1 - minor; }
  if (corner & NEO_MATRIX_BOTTOM) { major = matrixHeight - 1 - major; }

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
    if (major & 1) { pixelOffset = (major + 1) * majorScale - 1 - minor; }
    else { pixelOffset =  major      * majorScale     + minor; }
  }

  return tileOffset + pixelOffset;

}


/*

                      Matrix Manager...  Manages Matrix object, and json addittion and retreival.


*/

MelvtrixMan::MelvtrixMan()
{
  Serial.printf("[MelvtrixMan()] called\n");
  createMatrix();
}

MelvtrixMan::MelvtrixMan(uint16_t x, uint16_t y, uint8_t config): _grid_x(x), _grid_y(y), _matrixconfig(config)
{
  Serial.printf("[MelvtrixMan(x,y,config)] called\n");
  createMatrix();
}

MelvtrixMan::~MelvtrixMan()
{
  Serial.printf("[~MelvtrixMan()] called\n");
  if (_matrix) {
    delete _matrix;
    _matrix = nullptr;
  }
}


bool MelvtrixMan::createMatrix()
{
  if (_matrix) {
    delete _matrix;
    _matrix = nullptr;
  }
  _matrix = new Melvtrix(_grid_x, _grid_y, _matrixconfig);
  if (_matrix) {
    return true;
  } else {
    return false;
  }
}

bool MelvtrixMan::addJson(JsonObject & root)
{
  JsonObject& matrixjson = root.createNestedObject("Matrix");

  matrixjson["enabled"] = (_matrix) ? true : false;
  matrixjson["x"] = _grid_x;
  matrixjson["y"] = _grid_y;
  matrixjson["multiple"] = _multiplematrix;
  matrixjson["config"] = _matrixconfig;

}

bool MelvtrixMan::parseJson(JsonObject & root)
{
  Serial.printf("[MelvtrixMan::parseJson] called\n");

  bool changed = false;

  if (!root.containsKey("Matrix") ) {
    Serial.printf("[MelvtrixMan::parseJson] No Matrix Key\n");

    return false;
  }

  JsonObject& matrixjson = root["Matrix"];

  if (matrixjson.containsKey("x")) {
    if (_grid_x != matrixjson["x"]) {
      _grid_x = matrixjson["x"];
      changed = true;
    }
  }
  if (matrixjson.containsKey("y")) {
    if (_grid_y != matrixjson["y"]) {
      _grid_y = matrixjson["y"];
      changed = true;
    }
  }
  if (matrixjson.containsKey("config")) {
    if (_matrixconfig != matrixjson["config"]) {
      _matrixconfig = matrixjson["config"];
      changed = true;
    }
  }
  if (matrixjson.containsKey("multiple")) {
    if (_multiplematrix != matrixjson["multiple"]) {
      _multiplematrix = matrixjson["multiple"];
      changed = true;
    }
  }

  //if (matrixjson.containsKey("enabled") || changed) {
  //  if (matrixjson["enabled"] == true) {
  if (createMatrix()) {
    changed = true;
  }
  // } else {
  // not sure yet....
  // }



  Serial.printf("[MelvtrixMan::parseJson] _grid_x = %u, _grid_y = %u, config = %u\n", _grid_x, _grid_y, _matrixconfig);


  return changed;

}





// bool MelvtrixMan::parseHTTPargs(ESP8266WebServer & HTTP)
// {

// }
/*
  if (_HTTP.hasArg("grid_x") && _HTTP.hasArg("grid_y")) {
    grid(_HTTP.arg("grid_x").toInt(), _HTTP.arg("grid_y").toInt() );
    page = "layout";
  }

  if (_HTTP.hasArg("matrixmode")) {
    page = "layout";
    uint8_t matrixvar = 0;
    if (_HTTP.arg("matrixmode") == "singlematrix") { multiplematrix = false; }
    if (_HTTP.arg("firstpixel") == "topleft") { matrixvar += NEO_MATRIX_TOP + NEO_MATRIX_LEFT; }
    if (_HTTP.arg("firstpixel") == "topright") { matrixvar += NEO_MATRIX_TOP + NEO_MATRIX_RIGHT; }
    if (_HTTP.arg("firstpixel") == "bottomleft") { matrixvar += NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT; }
    if (_HTTP.arg("firstpixel") == "bottomright") { matrixvar += NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT; }

    if (_HTTP.arg("axis") == "rowmajor") { matrixvar += NEO_MATRIX_ROWS; }
    if (_HTTP.arg("axis") == "columnmajor") { matrixvar += NEO_MATRIX_COLUMNS ; }

    if (_HTTP.arg("sequence") == "progressive") { matrixvar += NEO_MATRIX_PROGRESSIVE ; }
    if (_HTTP.arg("sequence") == "zigzag") { matrixvar += NEO_MATRIX_ZIGZAG ; }

    if (_HTTP.arg("matrixmode") == "multiplematrix") {
      multiplematrix = true;
      if (_HTTP.arg("multimatrixtile") == "topleft") { matrixvar += NEO_TILE_TOP + NEO_TILE_LEFT; }
      if (_HTTP.arg("multimatrixtile") == "topright") { matrixvar += NEO_TILE_TOP + NEO_TILE_RIGHT; }
      if (_HTTP.arg("multimatrixtile") == "bottomleft") { matrixvar += NEO_TILE_BOTTOM + NEO_TILE_LEFT; }
      if (_HTTP.arg("multimatrixtile") == "bottomright") { matrixvar += NEO_TILE_BOTTOM + NEO_TILE_RIGHT; }
      if (_HTTP.arg("multimatrixaxis") == "rowmajor") { matrixvar += NEO_TILE_ROWS ; }
      if (_HTTP.arg("multimatrixaxis") == "columnmajor") { matrixvar += NEO_TILE_COLUMNS ; }
      if (_HTTP.arg("multimatrixseq") == "progressive") { matrixvar += NEO_TILE_PROGRESSIVE ; }
      if (_HTTP.arg("multimatrixseq") == "zigzag") { matrixvar += NEO_TILE_ZIGZAG ; }
    }

    DebugMelvanimatef("NEW Matrix params: %u\n", matrixvar);
    setmatrix(matrixvar);
  }

*/





