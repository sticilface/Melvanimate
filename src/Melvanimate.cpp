#include "Melvanimate.h"

#include <MD5Builder.h>
#include <ArduinoJson.h>

NeoPixelBus * strip = nullptr;
NeoPixelAnimator * animator = nullptr;

Melvanimate::Melvanimate(uint16_t pixels, uint8_t pin): _pixels(pixels), _pin(pin)
	, _grid_x(8), _grid_y(8), _matrixconfig(0), _matrix(nullptr)
	, _settings_changed(false)
{
	_matrixconfig = ( NEO_MATRIX_TOP + NEO_MATRIX_LEFT +  NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE );
	setWaitFn ( std::bind (&Melvanimate::returnWaiting, this)  );   //  this callback gives bool to Effectmanager... "am i waiting..."
}


bool Melvanimate::begin()
{
	DebugMelvanimatef("Begin Melvana called\n");

	_settings = SPIFFS.open(MELVANA_SETTINGS, "r+");

	if (!_settings) {
		DebugMelvanimatef("ERROR File open for failed!\n");
		_settings = SPIFFS.open(MELVANA_SETTINGS, "w+");
		if (!_settings) { DebugMelvanimatef("Failed to create empty file too\n"); }
	}

	_loadGeneral();
	_init_LEDs();
	_init_matrix();
}


void Melvanimate::Loop()
{
	_process();

	_timer.run();

	_saveGeneral(); 

	// if (_save_flag) {
	// 	if (millis() - _save_flag > 100) {
	// 		_save_flag = 0;
	// 		saveGeneral(); //  will only save if actually required.
	// 	}
	// }

}

void Melvanimate::_init_matrix()
{
	if (_matrix) { delete _matrix; _matrix = nullptr; }
	_matrix = new Melvtrix(_grid_x, _grid_y, _matrixconfig);

}

void Melvanimate::_init_LEDs()
{
	if (strip) { delete strip;  strip = nullptr; }
	if (animator) { delete animator; animator = nullptr ;}

	strip = new NeoPixelBus(_pixels, DEFAULT_WS2812_PIN);

	// if (_pixels <= maxLEDanimations) {
	// 	animator = new NeoPixelAnimator(strip);
	// 	_animations = true;
	// } else _animations = false;

	// not sure if this bit is working...
	if (!_pixels) {
		DebugMelvanimatef("MALLOC failed for pixel bus\n");
		return;
	}

	//stripBuffer = (uint8_t*)strip->Pixels();
	setmatrix(_matrixconfig);
	if (strip) {
		strip->Begin();
		strip->Show();
	}
}



const RgbColor  Melvanimate::dim(RgbColor input, const uint8_t brightness)
{
	if (brightness == 0) { return RgbColor(0); }
	if (brightness == 255) { return input; }
	if (input.R == 0 && input.G == 0 && input.B == 0 ) { return input; }
	HslColor originalHSL = HslColor(input);
	originalHSL.L =  originalHSL.L   * ( float(brightness) / 255.0 ) ;
	return RgbColor( HslColor(originalHSL.H, originalHSL.S, originalHSL.L )  );
}


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


bool        Melvanimate::_saveGeneral(bool override)
{

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


	if (!_settings) {
		DebugMelvanimatef("ERROR File NOT open!\n");
		_settings = SPIFFS.open(MELVANA_SETTINGS, "r+");
		if (!_settings) {
			_settings = SPIFFS.open(MELVANA_SETTINGS, "w+");
			DebugMelvanimatef("Failed to create empty file too\n");
			if (_settings) { return false; }
		}
	}

	_settings.seek(0, SeekSet);
	root.prettyPrintTo(_settings);
	_settings_changed = false;
	return true;
}


bool        Melvanimate::_loadGeneral()
{

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
				DebugMelvanimatef("[Melvanimate::setTimer] Load preset: not done yet\n");
			}
			_timerState = -1 ; //  get ride of flag to timer!
		});

		if (_timerState > -1 ) { DebugMelvanimatef("[Melvanimate::setTimer] Started (%s,%s)\n", command.c_str(), option.c_str()); }

	} else {
		DebugMelvanimatef("[Melvanimate::setTimer] No Timeout, so timer cancelled\n");
	}

}






