#include "BeatsTest.h"
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include "Melvanimate.h" // required for the MAX_NUMBER_OF_ANIMATIONS definition

#include "Palette.h"


extern MyPixelBus * strip;
extern NeoPixelAnimator * animator;

using namespace helperfunc;

bool BeatsTest::InitVars()
{
	addVar(new Variable<uint8_t>("brightness", 100));
	//addVar(new Variable<uint8_t>("filter", 80));
	//addVar(new Variable<float>("beatsratio", 2.1));
	//addVar(new Variable<uint8_t>("beatstimeout", 100));
	addVar(new Variable<EQ*>(1000, 3000)); //  EQ(samples, sampletime) //  sets the defaults for EQ beat detection...  Start with no params for just graphic equaliser settings.
	addVar(new Variable<Palette*>("Palette", Palette::OFF));

	palette()->mode(Palette::WHEEL);

	_EQ = getVar<EQ*>("EQ");


}

bool BeatsTest::Start()
{
	if (strip) {
		strip->ClearTo(0);
	}

	if (animator) {
		delete animator;
		animator = nullptr;
	}

	if (animator) {
		delete animator;
		animator = nullptr;
	}

	_pixels =  ( strip->PixelCount() < MAX_NUMBER_OF_ANIMATIONS ) ? strip->PixelCount() : MAX_NUMBER_OF_ANIMATIONS;

	animator = new NeoPixelAnimator( _pixels  );

	if (_EQ) {

		_EQ->SetBeatCallback( [this](EQParam params) {

			//if (params.channel == ) {
			//	Serial.printf("[Packet rec %u: Packet sent %u] channel: %u, avg: %u level: %u, bpm: %u\n", _EQ->seq(), params.seq_no , params.channel, params.average, params.level, params.bpm);
			//}

			uint8_t channel = params.channel;
			uint8_t size = 8 - channel;
			uint8_t level = params.level;

			bool pixel_busy = false;
			uint16_t pre_pixel = 0;
			uint16_t count = 0;

			do {
				pre_pixel = random(0, _pixels - size);
				count++;
				for (uint16_t i = pre_pixel; i < pre_pixel + size; i++) {
					if (animator->IsAnimationActive(pre_pixel)) {
						pixel_busy = true;
						break;
					}
				}
			} while (pixel_busy && count < 50);

			//  pixel block not busy
			if (!pixel_busy) {

				uint16_t pixel = pre_pixel; 
				RgbColor targetColor = dim( palette()->next() , brightness() );


				for (uint16_t i = pixel; i < pixel + size; i++) {

					AnimUpdateCallback animUpdate = [ = ](const AnimationParam & aniparam) {
						
						float progress = aniparam.progress;

						RgbColor updatedColor = RgbColor::LinearBlend(targetColor, 0,  progress) ;

						strip->SetPixelColor(  i  , updatedColor);
					};

					animator->StartAnimation(i, level * (size / 3),  animUpdate);

				}
			}


		});
	}
}

bool BeatsTest::Run()
{


	if (_EQ) {
		_EQ->loop();
	}

	// if (millis() - _tick > 30) {

	// 	for (int i = 0; i < 7; i++) {
	// 		int LEDs = map(_EQ->data[i], 80, 1023, 0, 8);
	// 		for (int j = 0; j < 8; j++) {
	// 			strip->SetPixelColor( (i * 8) + j, RgbColor(0));
	// 		}
	// 		for (int j = 0; j < LEDs; j ++) {
	// 			strip->SetPixelColor( (i * 8) + j, dim( RgbColor(50, 0, 0), brightness()) );
	// 		}
	// 	}

	// 	_tick = millis();
	// }

}

bool BeatsTest::Stop()
{

	if (strip) {
		strip->ClearTo(0);
	}

	if (animator) {
		delete animator;
		animator = nullptr;
	}
}


