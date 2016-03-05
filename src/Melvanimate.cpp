#include "Melvanimate.h"

#include <MD5Builder.h>
#include <ArduinoJson.h>
#include "BufferedPrint.h"

NeoPixelBus * strip = nullptr;
NeoPixelAnimator * animator = nullptr;

Melvanimate::Melvanimate(ESP8266WebServer & HTTP, uint16_t pixels, uint8_t pin): _HTTP(HTTP), _pixels(pixels), _pin(pin)
	, _grid_x(8), _grid_y(8), _matrixconfig(0), _matrix(nullptr)
	, _settings_changed(false)
{
	_matrixconfig = ( NEO_MATRIX_TOP + NEO_MATRIX_LEFT +  NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE );
	setWaitFn ( std::bind (&Melvanimate::returnWaiting, this)  );   //  this callback gives bool to Effectmanager... "am i waiting..."

}


bool Melvanimate::begin()
{



	DebugMelvanimatef("Begin Melvana called\n");

	_HTTP.on("/data.esp", HTTP_ANY, std::bind (&Melvanimate::_handleWebRequest, this));
	_HTTP.serveStatic("/jqColorPicker.min.js", SPIFFS, "/jqColorPicker.min.js", "max-age=86400");

	_loadGeneral();
	_init_LEDs();
	_init_matrix();

	fillPresetArray();
}


void Melvanimate::loop()
{
	_process(); //  this is function from EffectManager that has to be run.
	_timer.run();
	_saveGeneral();

}

void Melvanimate::_init_matrix()
{
	if (_matrix) { delete _matrix; _matrix = nullptr; }
	_matrix = new Melvtrix(_grid_x, _grid_y, _matrixconfig);

}

void Melvanimate::_init_LEDs()
{
	if (strip) {
		delete strip;
		strip = nullptr;
	}
	if (animator) {
		delete animator;
		animator = nullptr ;
	}

	if (_pixels) {
		strip = new NeoPixelBus(_pixels, DEFAULT_WS2812_PIN);
	}

	setmatrix(_matrixconfig);

	if (strip) {
		strip->Begin();
		strip->Show();
	}

}



// RgbColor  Melvanimate::dim(RgbColor input, const uint8_t brightness)
// {
// 	if (brightness == 0) { return RgbColor(0); }
// 	if (brightness == 255) { return input; }
// 	if (input.R == 0 && input.G == 0 && input.B == 0 ) { return input; }
// 	HslColor originalHSL = HslColor(input);
// 	originalHSL.L =  originalHSL.L   * ( float(brightness) / 255.0 ) ;
// 	return RgbColor( HslColor(originalHSL.H, originalHSL.S, originalHSL.L )  );
// }


void        Melvanimate::grid(const uint16_t x, const uint16_t y)
{
	if ( x * y > _pixels) { return; } // bail if grid is too big for pixels.. not sure its required
	if (_grid_x == x && _grid_y == y) { return; } // return if unchanged
	Start("Off");
	_grid_x = x;
	_grid_y = y;
	DebugMelvanimatef("NEW grids (%u,%u)\n", _grid_x, _grid_y);
	_settings_changed = true;
	if (_matrix) { delete _matrix; _matrix = nullptr; }
	_matrix = new Melvtrix(_grid_x, _grid_y, _matrixconfig);
}
void        Melvanimate::setmatrix(const uint8_t i)
{
	if (_matrixconfig == i && _matrix) { return; } //  allow for first initialisation with params = initialised state.
	Start("Off");
	_matrixconfig = i;
	DebugMelvanimatef("NEW matrix Settings (%u)\n", _matrixconfig);
	_settings_changed = true;
	_init_matrix();
}

void        Melvanimate::setPixels(const uint16_t pixels)
{
	if (pixels == _pixels) { return; }
	DebugMelvanimatef("NEW Pixels: %u\n", _pixels);
	strip->ClearTo(0);
	_pixels = pixels;
	_settings_changed = true;
	_init_LEDs();
	DebugMelvanimatef("HEAP: %u\n", ESP.getFreeHeap());
}

//  This is a callback that when set, checks to see if current animation has ended.
// set using setWaitFn ( std::bind (&Melvana::returnWaiting, this)  ); in initialisation
bool Melvanimate::returnWaiting()
{
	if (!_waiting) { return false; }

	if (animator && _waiting == 2) {

		if (!animator->IsAnimating()) {
			DebugMelvanimatef("[Melvanimate::returnWaiting] Autowait END (%u)\n", millis());
			_waiting = false;
			return false;
		}
	}

	// saftey, in case of faulty effect
	if (millis() - _waiting_timeout > EFFECT_WAIT_TIMEOUT) {
		_waiting = false;
		_waiting_timeout = 0;
		return false;
	}
	return true;

}

int Melvanimate::getTimeLeft()
{
	if (_timerState >= 0) {
		return _timer.getTimeLeft(_timerState);
	} else {
		return 0;
	}
}


void Melvanimate::autoWait()
{
	DebugMelvanimatef("[Melvanimate::autoWait] Auto wait set (%u)\n", millis());
	_waiting_timeout = millis();
	_waiting = 2;
}

void Melvanimate::setWaiting(bool wait)
{
	if (wait) {
		DebugMelvanimatef("[Melvanimate::setWaiting] Set wait true (%u)\n", millis());
		_waiting_timeout = millis();
		_waiting = true;
	} else {
		DebugMelvanimatef("[Melvanimate::setWaiting] Set wait false (%u)\n", millis());
		_waiting = false;
		_waiting_timeout = 0;
	}
}


bool Melvanimate::_saveGeneral(bool override)
{
	File _settings;

	if (!_settings_changed && !override) { return false; }

	DebugMelvanimatef("Saving Settings: ");
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	JsonObject& globals = root.createNestedObject("globals");
	{
		globals["pixels"] = _pixels ;
		globals["matrixconfig"] = _matrixconfig ;
		globals["gridx"] = _grid_x ;
		globals["gridy"] = _grid_y ;
		globals["rotation"] = _matrix->getRotation();
	}





	//  need to check this... think it overwrites leaving old stuff behind...
	// if (!_settings) {
	// 	DebugMelvanimatef("ERROR File NOT open!\n");
		//_settings = SPIFFS.open(MELVANA_SETTINGS, "r+");
		//if (!_settings) {
			_settings = SPIFFS.open(MELVANA_SETTINGS, "w+");
			//DebugMelvanimatef("Failed to create empty file too\n");
			if (!_settings) { return false; }
		//}
	//}

	_settings.seek(0, SeekSet);
	root.prettyPrintTo(_settings);
	_settings_changed = false;
	_settings.close();
	return true;
}


bool Melvanimate::_loadGeneral()
{
	File _settings;

	DynamicJsonBuffer jsonBuffer;
	if (!_settings) {
		DebugMelvanimatef("[Melvanimate::load] ERROR File NOT open!\n");
		_settings = SPIFFS.open(MELVANA_SETTINGS, "r");
		if (!_settings) {
			DebugMelvanimatef("[Melvanimate::load] No Settings File Found\n");
			return false;
		}
	}

	char * data = nullptr;

	if (_settings.size()) {

		data = new char[_settings.size()];

	} else {
		DebugMelvanimatef("[Melvanimate::load] Fail: buffer size 0\n");
		return false;
	}

	// prevent nullptr exception if can't allocate
	if (data) {
		DebugMelvanimatef("[Melvanimate::load] buffer size %u\n", _settings.size());

		//  This method give a massive improvement in file reading speed for SPIFFS files..

		int bytesleft = _settings.size();
		int position = 0;
		while ((_settings.available() > -1) && (bytesleft > 0)) {

			// get available data size
			int sizeAvailable = _settings.available();
			if (sizeAvailable) {
				int readBytes = sizeAvailable;

				// read only the asked bytes
				if (readBytes > bytesleft) {
					readBytes = bytesleft ;
				}

				// get new position in buffer
				char * buf = &data[position];
				// read data
				int bytesread = _settings.readBytes(buf, readBytes);
				bytesleft -= bytesread;
				position += bytesread;

			}
			// time for network streams
			delay(0);
		}


		JsonObject& root = jsonBuffer.parseObject(data);

		if (!root.success()) {
			DebugMelvanimatef("[Melvanimate::load] Parsing settings file Failed!\n");
			return false;
		} else { DebugMelvanimatef("[Melvanimate::load] Parse successfull\n"); }
// global variables
		if (root.containsKey("globals")) {

			JsonObject& globals = root["globals"];

			_pixels = globals["pixels"].as<long>() ;
			_matrixconfig = globals["matrixconfig"].as<long>()  ;
			_grid_x  = globals["gridx"].as<long>();
			_grid_y  = globals["gridy"].as<long>();


		} else { DebugMelvanimatef("[Melvanimate::load] No Globals\n"); }
// current settings
		if (root.containsKey("current")) {

			JsonObject& current = root["current"];

		} else { DebugMelvanimatef("[Melvanimate::load] No Current\n"); }
// effect settings
		if (root.containsKey("effectsettings")) {
			JsonObject& effectsettings = root["effectsettings"];



		} else { DebugMelvanimatef("[Melvanimate::load] No effect settings\n"); }

		_settings.close();

		return true;

	} else {
		DebugMelvanimatef("[Melvanimate::load] Unable to Malloc for settings\n");
	}

};

bool Melvanimate::setTimer(int timeout, String command, String option)
{
	// delete current timer if set
	if (_timerState != -1) {
		_timer.deleteTimer(_timerState);
		_timerState = -1;
		DebugMelvanimatef("[Melvanimate::setTimer] Timer Cancelled\n");
	}

	timeout *= (1000 * 60); // convert timout to milliseconds from minutes...

	// set new timer if there is an interval
	if (timeout) {

		_timerState = _timer.setTimeout(timeout, [command, option, this]() {
			DynamicJsonBuffer jsonBuffer;
			JsonObject& root = jsonBuffer.createObject();

			if (command.equalsIgnoreCase("off")) {
				Start("Off");
			} else if (command.equalsIgnoreCase("start")) {
				Start(option);
			} else if (command.equalsIgnoreCase("brightness")) {
				if (Current()) {
					root["brightness"] = option.toInt();
					Current()->parseJson(root);
				}
			} else if (command.equalsIgnoreCase("speed")) {
				if (Current()) {
					root["speed"] = option.toInt();
					Current()->parseJson(root);
				}
			} else if (command.equalsIgnoreCase("loadpreset")) {
				DebugMelvanimatef("[Melvanimate::setTimer] Load preset: %u\n", option.toInt());
				Load(option.toInt());
			}
			_timerState = -1 ; //  get ride of flag to timer!
		});

		if (_timerState > -1 ) { DebugMelvanimatef("[Melvanimate::setTimer] Started (%s,%s)\n", command.c_str(), option.c_str()); }

	} else {
		DebugMelvanimatef("[Melvanimate::setTimer] No Timeout, so timer cancelled\n");
	}

}

void Melvanimate::_sendData(String page, int8_t code)
{
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	root["code"] = code;

	/*
	      Home page
	*/

	if (page == "homepage" || page == "palette" || page == "all") {
		JsonArray& modes = root.createNestedArray("modes");
		//Serial.printf("Total effects: %u\n", total());
		for (uint8_t i = 0; i < total(); i++) {
			modes.add(getName(i));
		}
		// creates settings node for web page
		JsonObject& settings = root.createNestedObject("settings");
		// adds minimum current effect name, if there if addJson returns false.
		if (Current()) {

			settings["currentpreset"] = Current()->preset();
//			settings["currentpresetFile"] = Current()->saveFileID;

			if (!Current()->addJson(settings)) {
				settings["effect"] = Current()->name();
			}

			if (!settings.containsKey("effect")) {
				settings["effect"] = Current()->name();
			}


			addCurrentPresets(root);


		}



// *  Not needed as palette is added within the add of a secific effect..

		// Palette * palette = Current()->getPalette();
		// if (palette) {
		//   root["palettename"] = String(palette->getModeString());
		// }



	}
	/*
	      Layout Page

	[ARG:3] matrixmode = singlematrix
	[ARG:4] firstpixel = topleft
	[ARG:5] axis = rowmajor
	[ARG:6] sequence = progressive
	[ARG:7] multimatrixtile = topleft
	[ARG:8] multimatrixaxis = rowmajor
	[ARG:9] multimatrixseq = progressive
	*/
	if (page == "layout" || page == "all") {
		root["pixels"] = getPixels();
		root["grid_x"] = getX();
		root["grid_y"] = getY();
		root["multiplematrix"] = multiplematrix;

		root["matrixconfig"] = getmatrix();

		uint8_t matrixconfig = getmatrix();
		bool bottom = (matrixconfig & NEO_MATRIX_BOTTOM) ;
		bool right = (matrixconfig & NEO_MATRIX_RIGHT) ;

// single matrix
		if (!bottom && !right) { root["firstpixel"] = "topleft"; }
		if (!bottom && right) { root["firstpixel"] = "topright"; }
		if (bottom && !right) { root["firstpixel"] = "bottomleft"; }
		if (bottom && right ) { root["firstpixel"] = "bottomright"; }

		if ((matrixconfig & NEO_MATRIX_AXIS) == NEO_MATRIX_ROWS) {
			root["axis"] = "rowmajor";
		} else {
			root["axis"] = "columnmajor";
		}

		if ((matrixconfig & NEO_MATRIX_SEQUENCE) == NEO_MATRIX_PROGRESSIVE) {
			root["sequence"] = "progressive";
		} else {
			root["sequence"] = "zigzag";
		}


// Tiles

		bottom = (matrixconfig & NEO_TILE_BOTTOM) ;
		right = (matrixconfig & NEO_TILE_RIGHT) ;

		if (!bottom && !right) { root["multimatrixtile"] = "topleft"; }
		if (!bottom && right) { root["multimatrixtile"] = "topright"; }
		if (bottom && !right) { root["multimatrixtile"] = "bottomleft"; }
		if (bottom && right ) { root["multimatrixtile"] = "bottomright"; }

		if ((matrixconfig & NEO_TILE_AXIS) == NEO_TILE_ROWS) {
			root["multimatrixaxis"] = "rowmajor";
		} else {
			root["multimatrixaxis"] = "columnmajor";
		}


		if ((matrixconfig & NEO_TILE_SEQUENCE) == NEO_TILE_PROGRESSIVE) {
			root["multimatrixseq"] = "progressive";
		} else {
			root["multimatrixseq"] = "zigzag";
		}


	}

	/*
	      palette page
	*/

	// Palette is now handled by each effect handler... WICKED

	// if (page == "palette" || page == "all") {

	//   Palette * palette = Current()->getPalette();

	//   if (palette) {

	//     if (page != "all") { root["palette"] = String(palette->getModeString()); } // ignore if already sent
	//     root["paletterandom"] = String(palette->randommodeAsString());
	//     root["palettespread"] = String(palette->range());
	//     root["palettedelay"] = String(palette->delay());

	//     if (palette->addJson( root)) {
	//       Serial.println("[send_data] palette data added");
	//       JsonObject& Palette = root["Palette"];
	//       Palette["modeString"] = String(palette->getModeString()); //  This adds it as string.. saves having it saved to SPIFFS.
	//     }

	//   }
	// }

	if (page == "timer" || page == "all" || page == "homepage") {

		JsonObject& timerobj = root.createNestedObject("timer");
		timerobj["running"] = (getTimeLeft() > 0) ? true : false;
		if (getTimeLeft() > 0) {
			JsonArray& remaining = timerobj.createNestedArray("remaining");
			int minutes = getTimeLeft() / ( 1000 * 60) ;
			int seconds = getTimeLeft() / 1000 ;
			seconds %= 60;
			remaining.add(minutes);
			remaining.add(seconds);
		}
		// only add them all for the actual timer page...
		if (page == "timer") {
			addAllpresets(root);
		}
	}

	if (page == "presetspage") {

		JsonObject& settings = root.createNestedObject("settings");
		// adds minimum current effect name, if there if addJson returns false.
		if (Current()) {

			settings["currentpreset"] = Current()->preset();
			settings["currentpresetname"] = Current()->name();
//			settings["currentpresetFile"] = Current()->saveFileID;

			// if (!Current()->addJson(settings)) {
			//   settings["effect"] = Current()->name();
			// }

			// if (!settings.containsKey("effect")) {
			//   settings["effect"] = Current()->name();
			// }

			// if (_numberofpresets) {
			// 	JsonObject& currentpresets = root.createNestedObject("currentpresets");
			// 	for (uint8_t i = 0; i < _numberofpresets; i++ ) {
			// 		currentpresets[ String(_presets[i])] = _preset_names[i];
			// 	}
			// }
			addCurrentPresets(root);


		}

		addAllpresets(root);
	}

	// Serial.println("JSON REPLY");
	// root.prettyPrintTo(Serial);
	// Serial.println();

	_sendJsontoHTTP(root, _HTTP);

}

template <class T> void Melvanimate::_sendJsontoHTTP( const T & root, ESP8266WebServer & HTTP)
{

	size_t jsonlength = root.measureLength();
	HTTP.setContentLength(jsonlength);
	HTTP.send(200, "text/json" );
	BufferedPrint<HTTP_DOWNLOAD_UNIT_SIZE> proxy(HTTP);
	root.printTo(proxy);
	proxy.flush();
	proxy.stop();

}


void Melvanimate::_handleWebRequest()
{
	uint32_t start_time = millis();
	String page = "homepage";
	int8_t code = -1;

	//  this fires back an OK, but ignores the request if all the args are the same.  uses MD5.
	if (_check_duplicate_req()) { _HTTP.setContentLength(0); _HTTP.send(200); return; }

	DebugMelvanimatef("\n");

	for (uint8_t i = 0; i < _HTTP.args(); i++) {
		DebugMelvanimatef("[ARG:%u] %s = %s\n", i, _HTTP.argName(i).c_str(), _HTTP.arg(i).c_str());
	}

	DebugMelvanimatef("Heap = [%u]\n", ESP.getFreeHeap());


	if (_HTTP.hasArg("plain")) {
		//  ABANDONED
		// DynamicJsonBuffer jsonBufferplain;
		// JsonObject& root = jsonBufferplain.parseObject(_HTTP.arg("plain").c_str());
		// if (root.success()) {

		//   if (Current()) {
		//     if (Current()->parseJsonArgs(root)) {
		//       Serial.println("[handle] JSON (via Plain) Setting applied");
		//     }
		//   }

		// }
	}

	if (_HTTP.hasArg("enable")) {
		if (_HTTP.arg("enable").equalsIgnoreCase("on")) {
			code = Start();
		} else if (_HTTP.arg("enable").equalsIgnoreCase("off")) {
			code = Start("Off");
		}
	}

	if (_HTTP.hasArg("mode")) {
		code = Start(_HTTP.arg("mode"));
	}


	if (_HTTP.hasArg("preset")) {
		uint8_t preset = _HTTP.arg("preset").toInt();
		if (Load(preset)) {
			//  try to switch current effect to preset...
			DebugMelvanimatef("[handle] Loaded preset %u\n", preset);
			code = 1;
		}

	}



	// puts all the args into json...
	// might be better to send pallette by json instead..

	DynamicJsonBuffer jsonBuffer;
	JsonObject & root = jsonBuffer.createObject();

	for (uint8_t i = 0; i < _HTTP.args(); i++) {
		root[_HTTP.argName(i)] = _HTTP.arg(i);
	}



	if (_HTTP.hasArg("nopixels") && _HTTP.arg("nopixels").length() != 0) {
		setPixels(_HTTP.arg("nopixels").toInt());
		page = "layout";

	}

	if (_HTTP.hasArg("palette")) {
		//palette().mode(_HTTP.arg("palette").c_str());
		page = "palette"; //  this line might not be needed... palette details are now handled entirely by the effect for which they belong

		/*
		[ARG:0] palette = complementary
		[ARG:1] palette-random = timebased
		[ARG:2] palette-spread =
		[ARG:3] palette-delay =

		  palette["mode"] = (uint8_t)_mode;
		  palette["total"] = _total;
		  palette["available"] = _available;
		  palette["randmode"] = (uint8_t)_random;
		  palette["range"] = _range;
		  palette["delay"] = _delay;
		*/


		//  this is a bit of a bodge...  Capital P for object with all parameters...
		JsonObject & palettenode = root.createNestedObject("Palette");

		palettenode["mode"] = (uint8_t)Palette::stringToEnum(_HTTP.arg("palette").c_str());


		if (_HTTP.hasArg("palette-random")) {
			palettenode["randmode"] = (uint8_t)Palette::randommodeStringtoEnum(_HTTP.arg("palette-random").c_str());
		}

		if (_HTTP.hasArg("palette-spread")) {
			palettenode["range"] = _HTTP.arg("palette-spread");

		}

		if (_HTTP.hasArg("palette-delay")) {
			palettenode["delay"] = _HTTP.arg("palette-delay");

		}
		// Serial.println("[handle_data] JSON dump");
		// root.prettyPrintTo(Serial);
		// Serial.println();

	}




//  this has to go last for the JSON to be passed to the current effect
	if (Current()) {
		if (Current()->parseJson(root)) {
//      Serial.println("[handle] JSON Setting applied");
			code = 1;
		}
	}

// matrixmode stuff
// #define NEO_MATRIX_TOP         0x00 // Pixel 0 is at top of matrix
// #define NEO_MATRIX_BOTTOM      0x01 // Pixel 0 is at bottom of matrix
// #define NEO_MATRIX_LEFT        0x00 // Pixel 0 is at left of matrix
// #define NEO_MATRIX_RIGHT       0x02 // Pixel 0 is at right of matrix
// #define NEO_MATRIX_CORNER      0x03 // Bitmask for pixel 0 matrix corner
// #define NEO_MATRIX_ROWS        0x00 // Matrix is row major (horizontal)
// #define NEO_MATRIX_COLUMNS     0x04 // Matrix is column major (vertical)
// #define NEO_MATRIX_AXIS        0x04 // Bitmask for row/column layout
// #define NEO_MATRIX_PROGRESSIVE 0x00 // Same pixel order across each line
// #define NEO_MATRIX_ZIGZAG      0x08 // Pixel order reverses between lines
// #define NEO_MATRIX_SEQUENCE    0x08 // Bitmask for pixel line order

// #define NEO_TILE_TOP           0x00 // First tile is at top of matrix
// #define NEO_TILE_BOTTOM        0x10 // First tile is at bottom of matrix
// #define NEO_TILE_LEFT          0x00 // First tile is at left of matrix
// #define NEO_TILE_RIGHT         0x20 // First tile is at right of matrix
// #define NEO_TILE_CORNER        0x30 // Bitmask for first tile corner
// #define NEO_TILE_ROWS          0x00 // Tiles ordered in rows
// #define NEO_TILE_COLUMNS       0x40 // Tiles ordered in columns
// #define NEO_TILE_AXIS          0x40 // Bitmask for tile H/V orientation
// #define NEO_TILE_PROGRESSIVE   0x00 // Same tile order across each line
// #define NEO_TILE_ZIGZAG        0x80 // Tile order reverses between lines
// #define NEO_TILE_SEQUENCE      0x80 // Bitmask for tile line order

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


	if (_HTTP.hasArg("flashfirst")) {
		page = "layout";
		Start("Off");
		Stop();
		strip->ClearTo(0);
		AnimUpdateCallback animUpdate = [] (float progress) {
			strip->SetPixelColor(0, Palette::wheel( (uint8_t)(progress * 255) ));
			if (progress == 1.0) { strip->SetPixelColor(0, 0); }
		};

//   StartAnimation(0, 5000 , animUpdate);



	}

	if (_HTTP.hasArg("revealorder")) {
		page = "layout";
		Start("Off");
		Stop();
		strip->ClearTo(0);
		// ToDo
		float ratio = 1.0 / strip->PixelCount();

		for (uint16_t pixel = 0; pixel < strip->PixelCount() ; pixel++) {

			AnimUpdateCallback animUpdate = [this, ratio, pixel] (float progress) {
				if ( (uint8_t)(progress * 100) == (uint8_t)(pixel * ratio * 100)) {
					strip->SetPixelColor(pixel, Palette::wheel( (uint8_t)(ratio * 255)));
					strip->SetPixelColor( (pixel > 2) ? pixel - 2 : 0 , 0 );

				}
				if (progress == 1.0) { Start("Off"); }
			};
//      StartAnimation(pixel, 5000 , animUpdate);
		}


	}



	// if (_HTTP.hasArg("palette-random")) {
	//   palette().randommode(_HTTP.arg("palette-random").c_str());
	//   page = "palette";
	// }


	// if (_HTTP.hasArg("palette-spread")) {
	//   palette().range(_HTTP.arg("palette-spread").toFloat());
	//   page = "palette";
	// }

	// if (_HTTP.hasArg("palette-delay")) {
	//   palette().delay(_HTTP.arg("palette-delay").toInt());
	//   page = "palette";
	// }


	if (_HTTP.hasArg("data")) {
		_sendData(_HTTP.arg("data"), 0); // sends JSON data for whatever page is currently being viewed
		return;
	}

	if (_HTTP.hasArg("enabletimer")) {
		page = "timer";
		if (_HTTP.arg("enabletimer") == "on") {

			if (_HTTP.hasArg("timer") && _HTTP.hasArg("timercommand")) {

				String effect =  (_HTTP.hasArg("timeroption")) ? _HTTP.arg("timeroption") : String();

				if (setTimer(_HTTP.arg("timer").toInt(), _HTTP.arg("timercommand"), effect )) {
					DebugMelvanimatef("[handle] Timer command accepted\n");
				}
			}
		} else if (_HTTP.arg("enabletimer") == "off") {
			setTimer(0, "off");
		}

	}




	if (_HTTP.hasArg("presetcommand")) {

		//String in = _HTTP.arg("selectedeffect").toInt()
		//uint8_t File = in.substring(0, in.indexOf(".")).toInt();
		//uint8_t ID = in.substring(in.indexOf(".") + 1, in.length()).toInt();


		if (_HTTP.arg("presetcommand") == "load") {
			code = Load(_HTTP.arg("selectedeffect").toInt());
		} else if (_HTTP.arg("presetcommand") == "new" ) {
			code = Save(0, _HTTP.arg("presetsavename").c_str());
		} else if (_HTTP.arg("presetcommand") == "overwrite" ) {
			code = Save(_HTTP.arg("selectedeffect").toInt(), _HTTP.arg("presetsavename").c_str(), true);
		} else if (_HTTP.arg("presetcommand") == "delete" ) {
			code = removePreset(_HTTP.arg("selectedeffect").toInt());
		} else if (_HTTP.arg("presetcommand") == "deleteall" ) {
			removeAllpresets(); 
		}



	}
	//_HTTP.setContentLength(0);
	//_HTTP.send(200); // sends OK if were just receiving data...

	_sendData(page, code);

	DebugMelvanimatef("[handle] time %u: [Heap] %u\n", millis() - start_time, ESP.getFreeHeap());
	return;

}


//  this is required as some
bool Melvanimate::_check_duplicate_req()
{
	static uint32_t last_time = 0;
	static char last_request[16] = {0};
	if (_HTTP.hasArg("data")) { return false; }

	MD5Builder md5;
	md5.begin();

	for (uint8_t args = 0; args < _HTTP.args(); args++) {
		String req = _HTTP.argName(args) + _HTTP.arg(args);
		md5.add(req);
	}

	md5.calculate();
	bool match = false;
	//Serial.printf("[MD5] %s\n", md5.toString().c_str());
	char this_request[16] = {0};
	md5.getChars(this_request);

	if (memcmp(last_request, this_request, 16) == 0) {
		match = true;
		DebugMelvanimatef("Request ignored: duplicate");
	}

	memcpy(last_request, this_request, 16);

	bool time_elapsed = (millis() - last_time > 10000) ? true : false;
	last_time = millis();

	return match & !time_elapsed;

}


