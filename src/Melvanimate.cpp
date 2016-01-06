#include "Melvanimate.h"


const uint16_t TOTALPIXELS = 64;

NeoPixelBus * strip = nullptr;
NeoPixelAnimator * animator = nullptr;
uint8_t* stripBuffer = NULL;
WiFiUDP Udp;
const IPAddress multicast_ip_addr(224, 0, 0, 0); // Multicast broadcast address
const uint16_t UDPlightPort = 8888;
E131* e131 = nullptr;
SimpleTimer timer;

Melvanimate::Melvanimate(): _brightness(255), _color(0, 0, 0), _color2(0, 0, 0), _speed(50), _pixels(TOTALPIXELS)
	, _grid_x(8), _grid_y(8), _serialspeed(115200), _matrixconfig(0), _matrix(nullptr)
	, _settings_changed(false), timeoutvar(0), effectposition(0), _palette(nullptr)
{
	_matrixconfig = ( NEO_MATRIX_TOP + NEO_MATRIX_LEFT +  NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE );
	setWaitFn ( std::bind (&Melvanimate::returnWaiting, this)  );   //  this callback gives bool to Effectmanager... "am i waiting..."
	_palette = new Palette;
}


bool        Melvanimate::begin()
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



const RgbColor  Melvanimate::dim(RgbColor input, const uint8_t brightness)
{
	if (brightness == 0) return RgbColor(0);
	if (brightness == 255) return input;
	if (input.R == 0 && input.G == 0 && input.B == 0 ) return input;
	HslColor originalHSL = HslColor(input);
	originalHSL.L =  originalHSL.L   * ( float(brightness) / 255.0 ) ;
	return RgbColor( HslColor(originalHSL.H, originalHSL.S, originalHSL.L )  );
}

void      Melvanimate::setBrightness(const uint8_t bright)
{
	if (bright == _brightness) {
		return;
	}
	_brightness = bright;


	//  new methods

	if (_currentHandle) {
		_currentHandle->setBrightness(bright); 
	}

	Refresh();
	_settings_changed = true;
}

void      Melvanimate::color(const RgbColor color)
{
	_color = color;
	_palette->input(color);

	if (_currentHandle) {
		_currentHandle->setColor(color);
	}


	_settings_changed = true;
	Refresh();
}
void      Melvanimate::color2(const RgbColor color)
{
	_color2 = color;
	_settings_changed = true;
	Refresh();
}


void      Melvanimate::serialspeed(const int speed)
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
void        Melvanimate::grid(const uint16_t x, const uint16_t y)
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
void        Melvanimate::setmatrix(const uint8_t i)
{
	if (_matrixconfig == i && _matrix) { return; } //  allow for first initialisation with params = initialised state.
	Start("Off");
	_matrixconfig = i;
	Debugf("NEW matrix Settings (%u)\n", _matrixconfig);
	_settings_changed = true;
	_init_matrix();
}

void        Melvanimate::setPixels(const uint16_t pixels)
{
	if (pixels == _pixels) { return; }
	Debugf("NEW Pixels: %u\n", _pixels);
	strip->ClearTo(0);
	_pixels = pixels;
	_settings_changed = true;
	_init_LEDs();
	Debugf("HEAP: %u\n", ESP.getFreeHeap());
}

const char * Melvanimate::getText()
{
	return _marqueetext.c_str();
}

void Melvanimate::setText(String var)
{
	_marqueetext = var;
	_settings_changed = true;
	Refresh();
}


//  This is a callback that when set, checks to see if current animation has ended.
// set using setWaitFn ( std::bind (&Melvana::returnWaiting, this)  ); in initialisation
bool Melvanimate::returnWaiting()
{
	if (!_waiting) return false;

	if (animator && _waiting == 2) {

		if (!animator->IsAnimating()) {
			Serial.println("Autowait END");
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
	Serial.println("Auto wait set");
	_waiting_timeout = millis();
	_waiting = 2;
}

void Melvanimate::setWaiting(bool wait)
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


bool        Melvanimate::save(bool override)
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
bool        Melvanimate::load()
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

bool Melvanimate::setTimer(int timeout, String command, String option)
{
	// delete current timer if set
	if (_timer != -1) {
		timer.deleteTimer(_timer);
		_timer = -1;
		Serial.println("Timer Cancelled");
	}

	timeout *= (1000 * 60); // convert timout to milliseconds from minutes...

	// set new timer if there is an interval
	if (timeout) {

		_timer = timer.setTimeout(timeout, [command, option, this]() {

			if (command.equalsIgnoreCase("off")) {
				Start("Off");
			} else if (command.equalsIgnoreCase("start")) {
				Start(option);
			} else if (command.equalsIgnoreCase("brightness")) {
				setBrightness(option.toInt());
			} else if (command.equalsIgnoreCase("speed")) {
				speed(option.toInt());
			} else if (command.equalsIgnoreCase("loadpreset")) {
				Serial.println("Load preset: not done yet");
			}
			_timer = -1 ; //  get ride of flag to timer!
		});

		if (_timer > -1 ) { Serial.printf("Timer Started (%s,%s)\n", command.c_str(), option.c_str()); }

	} else {
		Serial.println("No Timeout, so timer cancelled");
	}

}
