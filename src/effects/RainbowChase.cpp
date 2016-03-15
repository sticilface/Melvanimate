

#include "effects/RainbowChase.h"
#include <NeopixelBus.h>
//#include <NeoPixelAnimator.h>
#include "mybus.h"
#include "helperfunc.h"
#include "Palette.h"

using namespace helperfunc;

extern MyPixelBus * strip;
//extern NeoPixelAnimator * animator;


bool RainbowChase::Start()
{
 //  think about a fade in sequence?
}

bool RainbowChase::Run()
{
	uint8_t speedvar = (speed())? speed():1; 

	uint32_t timeout = map(speedvar, 0, 255, 0 , 1000);

	if (millis() - _vars->lasttime < timeout || !strip) { return false; }

	if (wholerainbow()) {
		_rainbowCycle();
	} else {
		_rainbow();
	}

	_vars->lasttime = millis();

	return true;
}

bool RainbowChase::Stop()
{
	if (_vars) {
		delete _vars;
		_vars = nullptr;
	}
}
// void rainbow(uint8_t wait) {
//   uint16_t i, j;

//   for(j=0; j<256; j++) {
//     for(i=0; i<strip.numPixels(); i++) {
//       strip.setPixelColor(i, Wheel((i+j) & 255));
//     }
//     strip.show();
//     delay(wait);
//   }
// }
void RainbowChase::_rainbow()
{

	for (uint16_t pixel = 0; pixel < strip->PixelCount(); pixel++) {
		RgbColor color = dim(Palette::wheel((pixel + _vars->position) & 255), brightness() );
		strip->SetPixelColor(pixel, color);
	}

	if (_vars->position++ == 256) { _vars->position = 0; }

}

// // Slightly different, this makes the rainbow equally distributed throughout
// void rainbowCycle(uint8_t wait) {
//   uint16_t i, j;

//   for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
//     for(i=0; i< strip.numPixels(); i++) {
//       strip.setPixelColor(i, Wheel((        (i * 256 / strip.numPixels()   ) + j) & 255));
//     }
//     strip.show();
//     delay(wait);
//   }
// }
void RainbowChase::_rainbowCycle()
{

	for (uint16_t pixel = 0; pixel < strip->PixelCount(); pixel++) {
		RgbColor color = dim(Palette::wheel( ((   pixel * 256  / strip->PixelCount() ) + _vars->position) & 255), brightness() );
		strip->SetPixelColor(pixel, color);
	}

	if (_vars->position++ == 256  * 5 ) { _vars->position = 0; }

}






