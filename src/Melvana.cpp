#include "Melvana.h"


const uint16_t TOTALPIXELS = 64;

NeoPixelBus * strip = nullptr;
NeoPixelAnimator * animator = nullptr;
uint8_t* stripBuffer = NULL;

Melvana::Melvana(): _brightness(255), _color(0, 0, 0), _color2(0, 0, 0), _speed(50), _pixels(TOTALPIXELS)
	, _grid_x(8), _grid_y(8), _serialspeed(115200), _matrixconfig(0), _matrix(nullptr)
	, _settings_changed(false), timeoutvar(0), effectposition(0), _palette(nullptr)
{
	_matrixconfig = ( NEO_MATRIX_TOP + NEO_MATRIX_LEFT +  NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE );
	setWaitFn ( std::bind (&Melvana::returnWaiting, this)  );   //  this callback gives bool to Effectmanager... "am i waiting..."
	_palette = new Palette;
}


bool        Melvana::begin()
{
	Debugln("Begin Melvana called");

	_settings = SPIFFS.open(MELVANA_SETTINGS, "r+");

	if (!_settings) {
		Debugln("ERROR File open for failed!");
		_settings = SPIFFS.open(MELVANA_SETTINGS, "w+");
		if (!_settings) Debugln("Failed to create empty file too");
	}

	load();
	_init_LEDs();
	_init_matrix();

}


void Melvana::_init_matrix()
{
	if (_matrix) { delete _matrix; _matrix = nullptr; }
	_matrix = new Melvtrix(_grid_x, _grid_y, _matrixconfig);

}

void Melvana::_init_LEDs()
{
	if (strip) { delete strip;  strip = nullptr; }
	if (animator) { delete animator; animator = nullptr ;}

	strip = new NeoPixelBus(_pixels, DEFAULT_WS2812_PIN);

	if (_pixels <= maxLEDanimations) {
		animator = new NeoPixelAnimator(strip);
		_animations = true;
	} else _animations = false;

	// not sure if this bit is working...
	if (!_pixels) {
		Debugln("MALLOC failed for pixel bus");
		return;
	}

	stripBuffer = (uint8_t*)strip->Pixels();
	setmatrix(_matrixconfig);
	strip->Begin();
	strip->Show();
}



const RgbColor  Melvana::dim(RgbColor input, const uint8_t brightness)
{
	if (brightness == 0) return RgbColor(0);
	if (brightness == 255) return input;
	if (input.R == 0 && input.G == 0 && input.B == 0 ) return input; 
	HslColor originalHSL = HslColor(input);
	originalHSL.L =  originalHSL.L   * ( float(brightness) / 255.0 ) ;
	return RgbColor( HslColor(originalHSL.H, originalHSL.S, originalHSL.L )  );
}

void      Melvana::setBrightness(const uint8_t bright)
{
	if (bright == _brightness) { return; }
	_brightness = bright; Refresh();
	_settings_changed = true;
}

void      Melvana::color(const RgbColor color)
{
	_color = color;
	_palette->input(color);
	_settings_changed = true;
	Refresh();
}
void      Melvana::color2(const RgbColor color)
{
	_color2 = color;
	_settings_changed = true;
	Refresh();
}


void      Melvana::serialspeed(const int speed)
{
	if (speed == _serialspeed) { return; }
	_serialspeed = speed;
	if (Serial) {
		Debugln("Flushing and Ending Serial 1");
		Serial.flush();
		delay(500);
		Serial.end();
	}
	_settings_changed = true;
	Serial.begin(_serialspeed);
	Debugf("New Serial started speed: %u\n", _serialspeed);
}
void        Melvana::grid(const uint16_t x, const uint16_t y)
{
	if ( x * y > _pixels) { return; } // bail if grid is too big for pixels.. not sure its required
	if (_grid_x == x && _grid_y == y) { return; } // return if unchanged
	Start("Off");
	_grid_x = x;
	_grid_y = y;
	Debugf("NEW grids (%u,%u)\n", _grid_x, _grid_y);
	_settings_changed = true;
	if (_matrix) { delete _matrix; _matrix = nullptr; }
	_matrix = new Melvtrix(_grid_x, _grid_y, _matrixconfig);
}
void        Melvana::setmatrix(const uint8_t i)
{
	if (_matrixconfig == i && _matrix) { return; } //  allow for first initialisation with params = initialised state.
	Start("Off");
	_matrixconfig = i;
	Debugf("NEW matrix Settings (%u)\n", _matrixconfig);
	_settings_changed = true;
	_init_matrix();
}

void        Melvana::setPixels(const uint16_t pixels)
{
	if (pixels == _pixels) { return; }
	Debugf("NEW Pixels: %u\n", _pixels);
	strip->ClearTo(0);
	_pixels = pixels;
	_settings_changed = true;
	_init_LEDs();
	Debugf("HEAP: %u\n", ESP.getFreeHeap());
}

const char * Melvana::getText()
{
	return _marqueetext.c_str();
}

void Melvana::setText(String var)
{
	_marqueetext = var;
	_settings_changed = true;
	Refresh();
}


//  called
bool Melvana::returnWaiting()
{
	if (!_waiting) return false;

	if (animator && _waiting == 2) {

		if (!animator->IsAnimating()) {
			Serial.println("Autowait END");
			_waiting = false;
			return false;
		}
	}
	// saftey
	if (millis() - _waiting_timeout > EFFECT_WAIT_TIMEOUT) {
		_waiting = false;
		_waiting_timeout = 0;
		return false;
	}
	return true;

}

void Melvana::autoWait()
{
	Serial.println("Auto wait set");
	_waiting_timeout = millis();
	_waiting = 2;
}

void Melvana::setWaiting(bool wait)
{
	if (wait) {
		Serial.println("Set wait true");
		_waiting_timeout = millis();
		_waiting = true;
	} else {
		Serial.println("Set wait false");
		_waiting = false;
		_waiting_timeout = 0;
	}
}


bool        Melvana::save(bool override)
{
	if (!_settings_changed && !override) { Serial.println("Settings not changed"); return false; }
	Debug("Saving Settings: ");
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

	JsonObject& current = root.createNestedObject("current");

	current["brightness"] = _brightness ;
	current["speed"] = _speed ;
	current["mode"] = getName();

	JsonObject& jscolor1 = current.createNestedObject("color1");
	jscolor1["R"] = _color.R;
	jscolor1["G"] = _color.G;
	jscolor1["B"] = _color.B;

	JsonObject& jscolor2 = current.createNestedObject("color2");
	jscolor2["R"] = _color2.R;
	jscolor2["G"] = _color2.G;
	jscolor2["B"] = _color2.B;

// specific effect settings

	JsonObject& effectsettings = root.createNestedObject("effectsettings");

// Adalight
	JsonObject& jsAdalight = effectsettings.createNestedObject("Adalight");
	jsAdalight["serialspeed"] = _serialspeed ;

// Marquee
	JsonObject& jsMarquee = effectsettings.createNestedObject("Marquee");
	jsMarquee["marqueetext"] = getText() ;


	if (!_settings) {
		Debugln("ERROR File NOT open!");
		_settings = SPIFFS.open(MELVANA_SETTINGS, "r+");
		if (!_settings) {
			_settings = SPIFFS.open(MELVANA_SETTINGS, "w+");
			Debugln("Failed to create empty file too");
			if (_settings) return false;
		}
	}

	_settings.seek(0, SeekSet);
	root.prettyPrintTo(_settings);
	//f.close();
	//Debugln("Done");
	//Debugf("jsonBuffer SIZE : %u\n", jsonBuffer.size() );

	_settings_changed = false;
	return true;
};
bool        Melvana::load()
{

	DynamicJsonBuffer jsonBuffer(1000);
	if (!_settings) {
		Debugln("ERROR File NOT open!");
		_settings = SPIFFS.open(MELVANA_SETTINGS, "r+");
		if (!_settings) {
			Debugln("No Settings File Found");
			return false;
		}
	}

	_settings.seek(0, SeekSet);

	char data[_settings.size()];

	for (int i = 0; i < _settings.size(); i++) {
		data[i] = _settings.read();
	}

	//f.close();

	JsonObject& root = jsonBuffer.parseObject(data);

	if (!root.success()) {
		Debugln(F("Parsing settings file Failed!"));
		return false;
	} else Debugln("Parse successfull");
// global variables
	if (root.containsKey("globals")) {

		JsonObject& globals = root["globals"];

		_pixels = globals["pixels"].as<long>() ;
		_matrixconfig = globals["matrixconfig"].as<long>()  ;
		_grid_x  = globals["gridx"].as<long>();
		_grid_y  = globals["gridy"].as<long>();
		//_matrix->setRotation(globals["rotation"]); //conundrum... egg or chicken

		// Debugf("Globals:\n _pixels(%u) \n _matrixconfig(%u)\n _gridx(%u)\n _gridy(%u)\n",
		//        _pixels, _matrixconfig, _grid_x, _grid_y);
		// Debugf("Globals READ:\n _pixels(%u) \n _matrixconfig(%u)\n _gridx(%u)\n _gridy(%u)\n",
		//        globals["pixels"].as<long>(), globals["matrixconfig"].as<long>(), globals["gridx"].as<long>(), globals["gridy"].as<long>());

	} else Debugln("No Globals");
// current settings
	if (root.containsKey("current")) {

		JsonObject& current = root["current"];

		if (current.containsKey("brightness")) { _brightness = (uint8_t)current["brightness"].as<long>(); }
		if (current.containsKey("speed")) { _speed = (uint8_t)current["speed"].as<long>(); }

		if (current.containsKey("color1")) {
			JsonObject& jscolor1 = current["color1"];
			_color.R = jscolor1["R"].as<long>();
			_color.G = jscolor1["G"].as<long>();
			_color.B = jscolor1["B"].as<long>();
		}

		if (current.containsKey("color2")) {
			JsonObject& jscolor2 = current["color2"];
			_color2.R = jscolor2["R"].as<long>();
			_color2.G = jscolor2["G"].as<long>();
			_color2.B = jscolor2["B"].as<long>();
		}

		// Debugf("Current:\n _brightness(%u) \n _speed(%u)\n _color1(%u,%u,%u)\n _color2(%u,%u,%u)\n",
		//        _brightness, _speed, _color.R, _color.G, _color.B, _color2.R, _color2.G, _color2.B, _color2.R);
		// Debugf("Current READ:\n _brightness(%u) \n _speed(%u)\n _color1(%u,%u,%u)\n _color2(%u,%u,%u)\n",
		//        current["brightness"].as<long>(), current["speed"].as<long>(), jscolor1["R"].as<long>(), jscolor1["G"].as<long>(), jscolor1["B"].as<long>(),
		//        jscolor2["R"].as<long>(), jscolor2["G"].as<long>(), jscolor2["B"].as<long>());

	} else Debugln("No Current");
// effect settings
	if (root.containsKey("effectsettings")) {
		JsonObject& effectsettings = root["effectsettings"];

		if (effectsettings.containsKey("Adalight")) {
			JsonObject& jsAdalight = effectsettings["Adalight"];
			if (jsAdalight.containsKey("serialspeed")) _serialspeed = jsAdalight["serialspeed"];
		}

		if (effectsettings.containsKey("Marquee")) {
			JsonObject& jsAdalight = effectsettings["Marquee"];
			if (jsAdalight.containsKey("marqueetext"))  setText( jsAdalight["marqueetext"].asString() );
		}
	} else Debugln("No effect settings");


	return true;

};


