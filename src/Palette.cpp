#include "Palette.h"

// ALL = 0, COMPLEMENTARY, MONOCHROMATIC, ANALOGOUS, SPLITCOMPLEMENTS, TRIADIC, TETRADIC, MULTI, WHEEL};
const char * palettes_strings[9] = { "off", "complementary", "monochromatic", "analogous", "splitcomplements", "triadic", "tetradic", "multi", "wheel"};
const char * random_mode_strings[4] = {"off", "totalrandom", "timebased", "randomafterloop"} ;

Palette::Palette(const char * name): _mode(OFF), _total(0), _available(0), _position(0), _random(TOTAL_RANDOM), _input(RgbColor(0)), _delay(0), _name(name)
{
	Palette(OFF, 10);
}

Palette::Palette(palette_type mode, uint16_t total, const char * name) : _mode(OFF), _total(0), _available(0), _position(0), _random(NOT_RANDOM), _input(RgbColor(0)), _delay(0), _name(name)
{
	_mode = mode;
	_total = total;
	_available = available(mode, total);
	_random = NOT_RANDOM;
	_position = 0;
	_input = RgbColor(0);

}
Palette::~Palette()
{
	//if (_palette) delete[] _palette;
}

void Palette::mode(palette_type mode)
{
	_mode = mode;
	_available = available(_mode, _total);
	_total = _available;
	_position = 0 ;
	PaletteDebugf("[Palette::mode] _mode = %u\n", (uint8_t)_mode);
}

void Palette::randommode(const char * mode)
{
	PaletteDebugf("[Palette::randommode] Random mode called\n");
	if (mode) {
		for (uint8_t i = 0; i < NUMBER_OF_RANDOM_MODES; i++) {
			PaletteDebugf("[Palette::randommode] comparing %s\n", random_mode_strings[i] );
			if (strcmp(mode, random_mode_strings[i]) == 0) {
				randommode( (random_mode)i );
				PaletteDebugf("Found: %u, %s\n", i, random_mode_strings[i]);
			}
		}
	} else {
		PaletteDebugf("[Palette::randommode] NULL POINTER PASSED\n");
	}
}

random_mode Palette::randommodeStringtoEnum(const char * mode)
{
	if (mode) {
		for (uint8_t i = 0; i < NUMBER_OF_RANDOM_MODES; i++) {
			if (strcmp(mode, random_mode_strings[i]) == 0) {
				return (random_mode)i ;
			}
		}
	}

	return NOT_RANDOM;
}



void Palette::mode(const char * in)
{
	for (uint8_t i = 0; i < 9; i++ ) {
		if (strcmp(in, palettes_strings[i]) == 0) {
			mode(  (palette_type)i);
		}
	}
}

Palette::palette_type Palette::stringToEnum(const char * in)
{

	for (uint8_t i = 0; i < 9; i++ ) {
		if (strcmp(in, palettes_strings[i]) == 0) {
			return (palette_type)i;
		}
	}

	return OFF;

}


uint8_t Palette::available(palette_type mode, uint16_t total)
{
	switch (mode) {
	case OFF:
		return 0;
		break;
	case COMPLEMENTARY:
		return 2;
		break;
	case MONOCHROMATIC: // not implemented
		return 2;
	case ANALOGOUS: // this might be return total...
		return 3;
		break;
	case SPLITCOMPLEMENTS:
		return 3;
	case TRIADIC:
		return 3;
		break;
	case TETRADIC:
		return 4;
		break;
	case MULTI:
		return total;
		break;
	case WHEEL:
		return 255;
		break;
	}
}

RgbColor Palette::current()
{



	switch (_mode) {
	case OFF:
		return _input;
		break;
	case COMPLEMENTARY:
		return comlementary(_input, _position);
		break;
	case MONOCHROMATIC: // not implemented
		return 0;
	case ANALOGOUS: // this might be return total...
		return analogous(_input, _position,  _total, _range);
		break;
	case SPLITCOMPLEMENTS:
		return splitcomplements(_input, _position, _range);
	case TRIADIC:
		return triadic(_input, _position);
		break;
	case TETRADIC:
		return tetradic(_input, _position);
		break;
	case MULTI:
		return multi( _input, _position, _total);
		break;
	case WHEEL:
		return wheel(_position);
		break;
	}
}



const char * Palette::getModeString()
{
	//Serial.printf("Current palette [%u] %s\n", _mode,palettes_strings[_mode] );
	return palettes_strings[_mode];
}


RgbColor Palette::next()
{
	// _position++;
	if (_mode == OFF) { return _input; }
	bool changeinputflag = false;
	RgbColor output;

	uint16_t jump_size = (_total < _available) ?  _available / _total : 1;


	_position += jump_size;

	switch (_random) {
		
	case TIME_BASED_RANDOM: {
		if (millis() - _randtimertick > (_delay * 1000) ) {
			PaletteDebugf("[Palette::current] _input changed: TIME_BASED_RANDOM\n ");
			//_input = wheel(random(0, 255));
			changeinputflag = true;
			_randtimertick = millis();
		}

		break;
	}
	case TOTAL_RANDOM: {
		changeinputflag = true;
		//_input = wheel(random(0, 255));
		_position = random(0, _available);
		break;
	}
	case RANDOM_AFTER_LOOP: {
		if (_position >= _available) {
			changeinputflag = true;
			//_input = wheel(random(0, 255));
			PaletteDebugf("[Palette::current] _input changed: RANDOM_AFTER_LOOP\n ");
		}
		break;
	}

	}

	_position %= _available;
	
	output = current();

	if (changeinputflag) {
		_input = wheel(random(0, 255));
	}

	PaletteDebugf("[Palette] next() _input (%u,%u,%u), _total:%u, _available:%u, jump_size=%u, _position=%u => (%u,%u,%u) \n", _input.R, _input.G, _input.B, _total, _available, jump_size, _position, output.R, output.G, output.B);

	return output;
}

RgbColor Palette::previous()
{
	_position--;
	return current();
}

RgbColor Palette::comlementary(RgbColor Value, uint16_t position)
{
	if (position == 0) { return Value; }
	HslColor original = HslColor(Value);
	original.H += 0.5;
	if (original.H > 1.0) { original.H -= 1.0; }
	return RgbColor(original);
}

// 3 colors..
RgbColor Palette::splitcomplements(RgbColor Value, uint16_t position, float range)
{
	if (position == 0) { return Value; }
	HslColor original = HslColor(Value);
	float HUE = original.H + 0.5;
	HUE = HUE - (range / 2.0);
	HUE = HUE + ( float(position) * range );
	if (HUE < 0) { HUE += 1; }
	if (HUE > 1) { HUE -= 1; }
	original.H = HUE;
	return RgbColor(original);
}

RgbColor Palette::analogous(RgbColor Value, uint16_t position, uint16_t total, float range)
{
	if (position == 0) { return Value; }
	HslColor original = HslColor(Value);
	float HUE = original.H;
	float HUE_lower = HUE - (range / 2.0);
	float steps = range / float(total);
	HUE = HUE_lower + ( float(position) * float(steps) );
	if (HUE < 0) { HUE += 1; }
	if (HUE > 1) { HUE -= 1; }
	original.H = HUE;
	return RgbColor(original);
}

RgbColor  Palette::multi(RgbColor Value, uint16_t position, uint16_t total)
{

	HslColor original = HslColor(Value);
	float HUE = original.H;
	float HUE_gap = 1.0 / float(total); // HUE - (range / 2.0);
	HUE = HUE + (position * HUE_gap);
	if (HUE > 1) { HUE -= 1; }
	original.H = HUE;
	return RgbColor(original);

}

//
//	Credit to Adafruit for the wheel function in their AdaLight Lib.
//
RgbColor Palette::wheel (uint8_t position)
{

	position = 255 - position;
	if (position < 85) {
		return  RgbColor(255 - position * 3, 0, position * 3);
	} else if (position < 170) {
		position -= 85;
		return RgbColor(0, position * 3, 255 - position * 3);
	} else {
		position -= 170;
		return  RgbColor(position * 3, 255 - position * 3, 0);
	}
}
/*
	RgbColor _last;
	uint16_t _position;
	uint16_t _total; 		// sets number of colours in palette.  not always used.
	uint16_t _available;	// some palettes have fixed number of colours available
	palette_type _mode;
	//bool _random = false;
	random_mode _random;
	RgbColor _input;
	float _range = 0.2f; // spread of palettes...
	uint32_t _delay;
*/

bool Palette::addJson(JsonObject& root)
{
	//JsonObject& root = in;
	PaletteDebugf("[Palette::addJson] Func HIT, _mode = %u\n", (uint8_t)_mode );

	JsonObject& palette = root.createNestedObject(_name);
	palette["name"] = getModeString();
	palette["mode"] = (uint8_t)_mode;
	palette["total"] = _total;
	palette["available"] = _available;
	palette["randmode"] = (uint8_t)_random;
	palette["randmodeString"] = randommodeAsString();

	JsonArray& incol = palette.createNestedArray("inputcolor");
	incol.add(_input.R);
	incol.add(_input.G);
	incol.add(_input.B);
	palette["range"] = _range;
	palette["delay"] = _delay;
	//palette["last"] = _last;  //RgbColor

	PaletteDebugf("[Palette::addJson] Func DONE\n");

	return true;

}

bool Palette::parseJson(JsonObject& root)
{
	PaletteDebugf("[Palette::parseJson] Func HIT\n");
	root.prettyPrintTo(Serial);
	Serial.println();

	bool changed = false;

	if (!root.containsKey(_name)) { return false; }

	JsonObject& palette = root[_name];

	if (palette.containsKey("mode")) {

		palette_type modevar = (palette_type)palette["mode"].as<long>();

		if (_mode != modevar ) {

			mode((palette_type)modevar); //  = mode;
			changed = true;
			PaletteDebugf("[Palette::parseJson] mode = %u\n", (uint8_t)_mode );
		}
	}
	if (palette.containsKey("total")) {
		if (_total != palette["total"]) {
			_total = palette["total"];
			changed = true;
		}

	}
	if (palette.containsKey("available")) {
		if (_available != palette["available"]) {
			_available = palette["available"];
			changed = true;
		}

	}
	if (palette.containsKey("randmode")) {
		random_mode mode = (random_mode)palette["randmode"].as<long>();
		if (mode != _random) {
			randommode(mode);
			changed = true;
		}
	}
	if (palette.containsKey("inputcolor")) {

		uint8_t R = palette["inputcolor"][0];
		uint8_t G = palette["inputcolor"][1];
		uint8_t B = palette["inputcolor"][2];

		if (_input.R != R) {
			_input.R = R;
			changed = true;
		}
		if (_input.G != G) {
			_input.G = G;
			changed = true;
		}
		if (_input.B != B) {
			_input.B = B;
			changed = true;
		}
	}
	if (palette.containsKey("range")) {
		if (_range != palette["range"]) {
			_range = palette["range"];
			changed = true;
		}
	}
	if (palette.containsKey("delay")) {
		if (_delay != palette["delay"]) {
			_delay = palette["delay"];
		}
	}

	return changed;

}
