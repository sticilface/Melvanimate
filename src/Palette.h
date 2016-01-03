/*

		Palette Class....


*/

#pragma once
#include <RgbColor.h>
#include <HslColor.h>
#include <HsbColor.h>


union storedColor {
	RgbColor rgb;
	HslColor hsl;
	HsbColor hsb;
};

enum palette_type { OFF = 0, COMPLEMENTARY, MONOCHROMATIC, ANALOGOUS, SPLITCOMPLEMENTS, TRIADIC, TETRADIC, MULTI, WHEEL};
enum random_mode { NOT_RANDOM = 0 , TOTAL_RANDOM, TIME_BASED_RANDOM, RANDOM_AFTER_LOOP};

class Palette
{

public:
	Palette();
	Palette(palette_type mode, uint16_t total);
	~Palette();

	RgbColor next();
	RgbColor previous();
	RgbColor current();

	void refresh();

	void input(RgbColor input)
	{
		//if (_random) _random = NOT_RANDOM; // not sure i want this...
		_position = 0;
		_input = input;
	};

	// need to think about this.. if mode is changed then _available can be rubbish....  
	// maybe _total should not be exposed.. but wanted can be... 

	bool mode(palette_type mode) { _mode = mode; _available = available(_mode, _total); _total = _available;  _position = 0 ;}
	palette_type mode() {return _mode;};

	random_mode randommode() { return _random; };
	void randommode(random_mode random) { _random = random;}
	uint16_t getavailable() { return _available; }

	void total(uint16_t total)
	{
		_total = total;
		_available = available(_mode, _total);
	}

	uint16_t total() {return _total; }

	float range() { return _range; }
	void range(float range) { _range = range; }
	void range (uint8_t range) { _range = float( range / 255 ); }

	RgbColor get(uint8_t position) {};

	static RgbColor comlementary(RgbColor input, uint16_t position);
	static RgbColor monochromatic(RgbColor input, uint16_t position);
	static RgbColor analogous(RgbColor input, uint16_t position, uint16_t total, float range);
	static RgbColor splitcomplements(RgbColor input, uint16_t position, float range);
	static RgbColor triadic(RgbColor input, uint16_t position) // return multiple (total set to 3)
	{
		return multi(input, position, 3);
	}
	static RgbColor tetradic(RgbColor input, uint16_t position) // return multiple set to 4.. might tweak this a bit..
	{
		return multi(input, position, 4);
	}
	static RgbColor multi(RgbColor input, uint16_t position, uint16_t total) ;
	static RgbColor wheel(uint8_t position);

	static uint8_t available(palette_type mode, uint16_t total);

private:
	RgbColor _last;
	uint16_t _position;
	uint16_t _total; 		// sets number of colours in palette.  not always used.
	uint16_t _available;	// some palettes have fixed number of colours available
	palette_type _mode;
	//bool _random = false;
	random_mode _random;
	RgbColor _input;
	float _range = 0.2f;
};