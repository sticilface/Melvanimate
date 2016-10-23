#include "SunRise.h"
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include "mybus.h"
#include "helperfunc.h"
#include "Melvanimate.h" // required for the MAX_NUMBER_OF_ANIMATIONS definition


using namespace helperfunc;

extern MyPixelBus * strip;
extern NeoPixelAnimator * animator;


bool SunRise::Start()
{

	_starttime = millis();

	if (animator) {

		delete animator;
		animator = nullptr;
	}

	strip->ClearTo(0);

	animator = new NeoPixelAnimator(  ( strip->PixelCount() < MAX_NUMBER_OF_ANIMATIONS) ? strip->PixelCount() : MAX_NUMBER_OF_ANIMATIONS  );

	_progress = 0.0f;
	_endtime = _starttime + ( time() * 60 * 1000 );
	_timeout = (time() * 60 * 1000 ) /  ( (strip->PixelCount() / 2 ) ) / 3;

	//Serial.printf( " _starttime = %u, _endtime = %u, _timeout = %u\n", _starttime, _endtime, _timeout);


	HslColor red = HslColor(0.0f, 1.0f, 0.5f);
	HslColor yellow = HslColor(0.08f, 1.0f, 0.5f);

	if (animator) {

		animator->setTimeScale(NEO_CENTISECONDS);

		AnimEaseFunction easing = NeoEase::QuadraticInOut;

		MyColorFeature::ColorObject originalColor = red; // strip->GetPixelColor(pixel);

		_animUpdate = [ = ](const AnimationParam & param) {
			//float progress = param.progress;

			float progress = NeoEase::CubicIn(param.progress);
			//MyColorFeature::ColorObject  updatedColor = MyColorFeature::ColorObject::LinearBlend(originalColor, MyColorFeature::ColorObject(yellow), progress);
			//NeoHueBlendShortestDistance
			//MyColorFeature::ColorObject  updatedColor = MyColorFeature::ColorObject::LinearBlend<NeoHueBlendCounterClockwiseDirection>( originalColor, MyColorFeature::ColorObject(yellow), progress );
			MyColorFeature::ColorObject  updatedColor;
			//HslColor updatedColor;

			float gowhite = 0.9f;

			if (progress < 0.2f) {

				float rescaledprogress = progress / 0.2f; 

				updatedColor = MyColorFeature::ColorObject::LinearBlend( RgbColor(0), dim(red,brightness()), rescaledprogress);


			} else if (progress < gowhite) {
				
				float rescaledprogress = ( progress - 0.2f) / (gowhite - 0.2f); 

				updatedColor = HslColor::LinearBlend<NeoHueBlendShortestDistance>( dim(red, brightness()), dim(yellow,brightness()), rescaledprogress);


			} else {

				if (strip->PixelSize() == 3) {


					float left = (progress - gowhite) / (1.0f - gowhite) ;  //  scales what ever is left of progress to take 0.5 to add

					updatedColor = dim( HslColor( yellow.H, yellow.S - left,  yellow.L), brightness());
				} else {

					float rescaledprogress = (progress - gowhite) / (1.0f - gowhite); 
					updatedColor = RgbColor::LinearBlend( dim(yellow, brightness()), RgbColor(0), rescaledprogress  );
					uint8_t brightness = rescaledprogress * 255 ; 
					MyColorFeature::ColorObject  temp =  updatedColor; 

					//updatedColor = MyColorFeature::ColorObject( temp.R, temp.G, temp.B);

					updatedColor = MyColorFeature::ColorObject(brightness);
					updatedColor.R = temp.R;
					updatedColor.G = temp.G;
					updatedColor.B = temp.B; 



				}




			}



			// if (strip) {
			// 	for (uint16_t pixel = 0; pixel < 8; pixel++) {
			strip->SetPixelColor(param.index, updatedColor);
			// 	}
			// }

		};

		//animator->StartAnimation(0, 20000, animUpdate);

	}
}


void SunRise::Refresh()
{
	Start();

}



bool SunRise::Run()
{
	if (_progress < 1.0f && millis() - _tick > _timeout) {

		_progress =  float(  (  millis() - _starttime ) ) /  float ((_endtime - _starttime) ) ;

		uint16_t count = strip->PixelCount();

		if (strip->PixelCount() % 2 == 0 ) {
			count--;
		}

		count = count / 2;

		uint16_t step = _progress * count  + 1;


		if (step == 1 && strip->PixelCount() % 2 == 0) {
			animator->StartAnimation( count, time() * 60 * 100 , _animUpdate);
		}


		animator->StartAnimation( count - step, time() * 60 * 100 , _animUpdate);
		animator->StartAnimation( count + step , time() * 60 * 100 , _animUpdate);

		_tick = millis();
	}



}


bool SunRise::Stop()
{
	strip->ClearTo(0);
	_progress = 0;

	if (animator && !animator->IsAnimating()) {
		delete animator;
		animator = nullptr;
	}
}