#include "helperfunc.h"

#include <NeoPixelAnimator.h>

extern MyPixelBus * strip;
extern NeoPixelAnimator * animator;

#include <FS.h>



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