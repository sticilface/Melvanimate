#include "EQvisualiser.h"
#include <NeoPixelBus.h>
#include "Palette.h"


extern MyPixelBus * strip;

using namespace helperfunc;

bool EQvisualiser::InitVars()
{
	addVar(new Variable<uint8_t>("brightness", 255));
}

bool EQvisualiser::Start()
{
	//StartEQ();

	for (uint8_t i = 0; i < 7; i++) {
		_colours[i] = random(256);
	}
}

bool EQvisualiser::Run()
{

	if (millis() - _tick > 30) {

		GetEQ(_spectrumValue);

		for (int i = 0; i < 7; i++) {
			_spectrumValue[i] = constrain(_spectrumValue[i], _filter, 1023);  //was 1023
			int LEDs = map(_spectrumValue[i], 80, 1023, 0, 9);
			for (int j = 0; j < 8; j++) { 
				strip->SetPixelColor( (i * 8) + j, RgbColor(0)); 
			}
			for (int j = 0; j < LEDs; j ++) { 
				strip->SetPixelColor( (i * 8) + j, dim(  Palette::wheel(_colours[i]), brightness()) ); 
			}
		}

		_tick = millis();
	}

}

bool EQvisualiser::Stop()
{
	//EndEQ();
}


