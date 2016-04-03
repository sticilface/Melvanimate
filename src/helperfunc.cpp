


#include "helperfunc.h"
#include <FS.h>
#include <NeoPixelAnimator.h>

extern MyPixelBus * strip;
extern NeoPixelAnimator * animator;




RgbColor  helperfunc::dim(RgbColor input, const uint8_t brightness)
{
	if (brightness == 0) { return RgbColor(0); }
	if (brightness == 255) { return input; }
	if (input.R == 0 && input.G == 0 && input.B == 0 ) { return input; }
	HslColor originalHSL = HslColor(input);
	originalHSL.L =  originalHSL.L   * ( float(brightness) / 255.0 ) ;
	return RgbColor( HslColor(originalHSL.H, originalHSL.S, originalHSL.L )  );
}

bool helperfunc::convertcolor(JsonObject & root, const char * node)
{
	if (root.containsKey(node)) {

		String colorstring = root[node];
		JsonArray& colorroot = root.createNestedArray(node);
		uint8_t R = colorstring.substring(0, colorstring.indexOf(",")).toInt();
		colorstring = colorstring.substring( colorstring.indexOf(",") + 1, colorstring.length());
		uint8_t G = colorstring.substring(0, colorstring.indexOf(",")).toInt();
		uint8_t B = colorstring.substring( colorstring.indexOf(",") + 1, colorstring.length()).toInt();
		colorroot.add(R);
		colorroot.add(G);
		colorroot.add(B);
		return true;
	}
	return false;
}


//  could try and package this up... maybe using a struct... ... maybe...
bool helperfunc::parsespiffs(char *& data,  DynamicJsonBuffer & jsonBuffer, JsonObject *& root, const char * file_name)
{
	uint32_t starttime = millis();
	uint32_t filesize;

	//DynamicJsonBuffer jsonBuffer;

	File f = SPIFFS.open(file_name, "r");
	bool success = false;

	if (!f) {
		//DebugEffectManagerf("[parsespiffs] File open Failed\n");
	}

	if (f && f.size()) {
		filesize = f.size();
		//Serial.println("[parsespiffs] pre-malloc");

		data = new char[f.size()];
		// prevent nullptr exception if can't allocate
		if (data) {
			//DebugEffectManagerf("[parsespiffs] Buffer size %u\n", f.size());

			//  This method give a massive improvement in file reading speed for SPIFFS files..

			int bytesleft = f.size();
			int position = 0;
			while ((f.available() > -1) && (bytesleft > 0)) {

				// get available data size
				int sizeAvailable = f.available();
				if (sizeAvailable) {
					int readBytes = sizeAvailable;

					// read only the asked bytes
					if (readBytes > bytesleft) {
						readBytes = bytesleft ;
					}

					// get new position in buffer
					char * buf = &data[position];
					// read data
					int bytesread = f.readBytes(buf, readBytes);
					bytesleft -= bytesread;
					position += bytesread;

				}
				// time for network streams
				delay(0);
			}


			root = &jsonBuffer.parseObject(data);

			if (root->success()) {
				success = true;
			}

		} else {
			//DebugEffectManagerf("[parsespiffs] malloc failed\n");
		}
	}

	f.close();

	if (success) {
		//DebugEffectManagerf("[parsespiffs] heap: %u, FileName: %s, FileSize: %u, parsetime: %u, jsonBufferSize: %u\n", ESP.getFreeHeap(), file_name, filesize, millis() - starttime, jsonBuffer.size());
		return true;
	} else {
		return false;
	}
}


bool helperfunc::parsespiffs(char *& data,  DynamicJsonBuffer & jsonBuffer, JsonArray *& root, const char * file_name)
{
	uint32_t starttime = millis();
	uint32_t filesize;

	//DynamicJsonBuffer jsonBuffer;

	File f = SPIFFS.open(file_name, "r");
	bool success = false;

	if (!f) {
		//DebugEffectManagerf("[parsespiffs] File open Failed\n");
	}

	if (f && f.size()) {
		filesize = f.size();
		//Serial.println("[parsespiffs] pre-malloc");

		data = new char[f.size()];
		// prevent nullptr exception if can't allocate
		if (data) {
			//DebugEffectManagerf("[parsespiffs] Buffer size %u\n", f.size());

			//  This method give a massive improvement in file reading speed for SPIFFS files..

			int bytesleft = f.size();
			int position = 0;
			while ((f.available() > -1) && (bytesleft > 0)) {

				// get available data size
				int sizeAvailable = f.available();
				if (sizeAvailable) {
					int readBytes = sizeAvailable;

					// read only the asked bytes
					if (readBytes > bytesleft) {
						readBytes = bytesleft ;
					}

					// get new position in buffer
					char * buf = &data[position];
					// read data
					int bytesread = f.readBytes(buf, readBytes);
					bytesleft -= bytesread;
					position += bytesread;

				}
				// time for network streams
				delay(0);
			}


			root = &jsonBuffer.parseArray(data);

			if (root->success()) {
				success = true;
			}
		} else {
			//DebugEffectManagerf("[parsespiffs] malloc failed\n");
		}
	}

	f.close();

	//DebugEffectManagerf("[parsespiffs] heap: %u, FileName: %s, FileSize: %u, parsetime: %u, jsonBufferSize: %u\n", ESP.getFreeHeap(), file_name, filesize, millis() - starttime, jsonBuffer.size());

	if (success) {
		return true;
	} else {
		return false;
	}
}

void  helperfunc::Adalight_Flash()
{

	if (!animator) return; 
	
	AnimEaseFunction easing = NeoEase::QuadraticInOut;
	AnimUpdateCallback animUpdate = [ = ] (const AnimationParam & param) {

		RgbColor updatedColor;
		float progress = easing(param.progress);

		if (progress < 0.25) {
			updatedColor = RgbColor::LinearBlend(RgbColor(0), RgbColor(100, 0, 0), progress * 4 );
		} else if (progress < 0.5) {
			updatedColor = RgbColor::LinearBlend(RgbColor(100, 0, 0), RgbColor(0, 100, 0) , (progress - 0.25) * 4 );
		} else if (progress < 0.75) {
			updatedColor = RgbColor::LinearBlend(RgbColor(0, 100, 0), RgbColor(0, 0, 100), (progress - 0.5) * 4 );
		} else {
			updatedColor = RgbColor::LinearBlend(RgbColor(0, 0, 100), RgbColor(0, 0, 0), (progress - 0.75) * 4 );
		}

		for (int pixel = 0; pixel < strip->PixelCount(); pixel++) {
			strip->SetPixelColor(pixel, updatedColor);
		}
	};

		animator->StartAnimation(0, 1200, animUpdate);
}


bool helperfunc::expandMatrixConfigToJson(JsonObject & root)
{
  if (!root.containsKey("Matrix")) {
    return false;
  }

  JsonObject & matrixjson = root["Matrix"]; 

  uint8_t _matrixconfig = matrixjson["config"]; 
  bool _multiplematrix = matrixjson["multiple"];  

  bool bottom = (_matrixconfig & NEO_MATRIX_BOTTOM) ;
  bool right = (_matrixconfig & NEO_MATRIX_RIGHT) ;

// single matrix
  if (!bottom && !right) { matrixjson["firstpixel"] = "topleft"; }
  if (!bottom && right) { matrixjson["firstpixel"] = "topright"; }
  if (bottom && !right) { matrixjson["firstpixel"] = "bottomleft"; }
  if (bottom && right ) { matrixjson["firstpixel"] = "bottomright"; }

  if ((_matrixconfig & NEO_MATRIX_AXIS) == NEO_MATRIX_ROWS) {
    matrixjson["axis"] = "rowmajor";
  } else {
    matrixjson["axis"] = "columnmajor";
  }

  if ((_matrixconfig & NEO_MATRIX_SEQUENCE) == NEO_MATRIX_PROGRESSIVE) {
    matrixjson["sequence"] = "progressive";
  } else {
    matrixjson["sequence"] = "zigzag";
  }


// Tiles

  if (_multiplematrix) {

    bottom = (_matrixconfig & NEO_TILE_BOTTOM) ;
    right = (_matrixconfig & NEO_TILE_RIGHT) ;

    if (!bottom && !right) { matrixjson["multimatrixtile"] = "topleft"; }
    if (!bottom && right) { matrixjson["multimatrixtile"] = "topright"; }
    if (bottom && !right) { matrixjson["multimatrixtile"] = "bottomleft"; }
    if (bottom && right ) { matrixjson["multimatrixtile"] = "bottomright"; }

    if ((_matrixconfig & NEO_TILE_AXIS) == NEO_TILE_ROWS) {
      matrixjson["multimatrixaxis"] = "rowmajor";
    } else {
      matrixjson["multimatrixaxis"] = "columnmajor";
    }

    if ((_matrixconfig & NEO_TILE_SEQUENCE) == NEO_TILE_PROGRESSIVE) {
      matrixjson["multimatrixseq"] = "progressive";
    } else {
      matrixjson["multimatrixseq"] = "zigzag";
    }

  }

  return true;
}

