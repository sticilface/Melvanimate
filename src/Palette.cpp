#include "Palette.h"

// ALL = 0, COMPLEMENTARY, MONOCHROMATIC, ANALOGOUS, SPLITCOMPLEMENTS, TRIADIC, TETRADIC, MULTI, WHEEL};
const char * palettes_strings[9] = { "off", "complementary", "monochromatic", "analogous", "splitcomplements", "triadic", "tetradic", "multi", "wheel"};


Palette::Palette(): _mode(OFF), _total(0), _available(0), _position(0), _random(NOT_RANDOM), _input(RgbColor(0))
{
	Palette(OFF, 10);
}

Palette::Palette(palette_type mode, uint16_t total) : _mode(OFF), _total(0), _available(0), _position(0), _random(NOT_RANDOM), _input(RgbColor(0))
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
}


void Palette::mode(const char * in)
{
	for (uint8_t i = 0; i < 9; i++ ) {
		if (strcmp(in, palettes_strings[i]) == 0) {
			mode(  (palette_type)i);
		}
	}
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
	_position %= _available;

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
	if (_mode == OFF) return _input;
	uint16_t jump_size = (_total < _available) ?  _available / _total : 1;
	_position = _position + jump_size;

	if (_random == TOTAL_RANDOM) {
		_input = wheel(random(0, 255));
		_position = random(0, _available);
	}
	return current();
}

RgbColor Palette::previous()
{
	_position--;
	return current();
}

RgbColor Palette::comlementary(RgbColor Value, uint16_t position)
{
	if (position == 0) return Value;
	HslColor original = HslColor(Value);
	original.H += 0.5;
	if (original.H > 1.0) original.H -= 1.0;
	return RgbColor(original);
}

RgbColor Palette::splitcomplements(RgbColor Input, uint16_t position, float range)
{
	if (position == 0) return Input;
	HslColor original = HslColor(Input);
	float HUE = original.H + 0.5;
	HUE = HUE - (range / 2.0);
	HUE = HUE + ( float(position) * range );
	if (HUE < 0) HUE += 1;
	if (HUE > 1) HUE -= 1;
	original.H = HUE;
	return RgbColor(original);
}

RgbColor Palette::analogous(RgbColor Value, uint16_t position, uint16_t total, float range)
{

	HslColor original = HslColor(Value);
	float HUE = original.H;
	float HUE_lower = HUE - (range / 2.0);
	float steps = range / float(total);
	HUE = HUE_lower + ( float(position) * float(steps) );
	if (HUE < 0) HUE += 1;
	if (HUE > 1) HUE -= 1;
	original.H = HUE;
	return RgbColor(original);
}

RgbColor  Palette::multi(RgbColor Value, uint16_t position, uint16_t total)
{

	HslColor original = HslColor(Value);
	float HUE = original.H;
	float HUE_gap = 1.0 / float(total); // HUE - (range / 2.0);
	HUE = HUE + (position * HUE_gap);
	if (HUE > 1) HUE -= 1;
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
