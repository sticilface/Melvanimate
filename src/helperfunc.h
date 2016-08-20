#pragma once

//include <RgbColor.h>
//#include <HslColor.h>
#include <internal/RgbColor.h>
#include <internal/HslColor.h>
#include "mybus.h"
#include "Melvtrix.h"


#include <ArduinoJson.h>

namespace helperfunc {


	RgbColor dim( RgbColor input, const uint8_t brightness);
	bool convertcolor(JsonObject & root, const char * colorstring);

	bool parsespiffs(char *& data, DynamicJsonBuffer& jsonBuffer, JsonObject *& root, const char * file);
	bool parsespiffs(char *& data, DynamicJsonBuffer& jsonBuffer, JsonArray *& root, const char * file);
	void Adalight_Flash();
  bool expandMatrixConfigToJson(JsonObject & root);

	void cpuCycleTimer(bool = false);

}
