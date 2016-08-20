

#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include "mybus.h"
#include "helperfunc.h"
#include "Palette.h"
#include "Melvanimate.h" // required for the MAX_NUMBER_OF_ANIMATIONS definition

#include "ColorBlend.h"

using namespace helperfunc;

extern MyPixelBus * strip;
extern NeoPixelAnimator * animator;

bool ColorBlend::InitVars()
{
	addVar(new Variable<uint8_t>("brightness", 50));
	addVar(new Variable<RgbColor>("color1", RgbColor(0)));
	addVar(new Variable<RgbColor>("color2", RgbColor(0)));
	addVar(new Variable<uint8_t>("blendmode", 0));
	addVar(new Variable<uint8_t>("speed", 0));


	addVar(new Variable<bool>("use_matrix", false));
	addVar(new Variable<MelvtrixMan*>("Matrix"));  // must be called Matrix.  very importnat...
	addVar(new Variable<Palette*>("Palette", Palette::OFF));

	if (animator) {
		delete animator;
		animator = nullptr;
	}

	_pixels =  ( strip->PixelCount() < MAX_NUMBER_OF_ANIMATIONS ) ? strip->PixelCount() : MAX_NUMBER_OF_ANIMATIONS;

	animator = new NeoPixelAnimator( _pixels  );

}


bool ColorBlend::Start()
{


	c1 = dim( color1(), brightness() );
	c2 = dim( color2(), brightness() );

	if ( palette()->mode() != Palette::OFF) {

		Serial.println("[ColorBlend::Start] Callback attached!");

		palette()->input( color1()   ); 

		// //  this callback handles changes drven by timer!!!
		// palette()->attachCallback(  [this]() {

		 	this->c1 = dim(palette()->next(), brightness() );
		 	this->c2 = dim(palette()->next(), brightness() );
		// 	this->Draw();
		// });

	 }

	Draw();

}

bool ColorBlend::Run()
{

	if (palette()) {
		palette()->delay( speed() - 1 ); 
		//palette()->loop();
	}

	if (speed() && millis() - _timeout > speed()) {
		Serial.println("[ColorBlend::Run] timeouthit"); 
		Start();
	}

}

void ColorBlend::Refresh()
{
	Start();
}

void ColorBlend::Draw()
{

	Serial.printf("[ColorBlend::Draw] C1 (%u,%u,%u), C2(%u,%u,%u) \n", c1.R, c1.G, c1.B, c2.R, c2.G, c2.B ); 

	if (usematrix() && matrix() ) {

		uint16_t width = matrix()->width();
		uint16_t height = matrix()->height();

		uint16_t x, y, pixel;

		for ( x = 0; x < width; x ++  ) {

			for ( y = 0; y < height; y++) {

				pixel = matrix()->getPixel(x, y);

				RgbColor originalcolor = myPixelColor(strip->GetPixelColor(pixel));
				float xposition = (float)x / (float)width;
				float yposition = (float)y / (float)height;

				RgbColor targetcolor = RgbColor::BilinearBlend ( c1, c2, c2, c1, xposition, yposition  );

				AnimUpdateCallback animUpdate = [ this, targetcolor, originalcolor, pixel ](const AnimationParam & param) {
					float progress = param.progress;
					RgbColor updatedColor;
					//updatedColor = RgbColor::LinearBlend(originalcolor, targetcolor, progress);
					updatedColor =  HslColor::LinearBlend<NeoHueBlendShortestDistance>(originalcolor, targetcolor, progress);

					strip->SetPixelColor(pixel, updatedColor);
				};

				animator->StartAnimation(pixel,  1000 , animUpdate);

			}
		}



	} else {

		for (uint16_t i = 0; i < _pixels; i++) {
			RgbColor originalcolor = myPixelColor(strip->GetPixelColor(i));
			float position = (float)i / (float)_pixels;

			RgbColor targetcolor = RgbColor::LinearBlend(c1, c2, position   );


			AnimUpdateCallback animUpdate = [ this, targetcolor, originalcolor, i ](const AnimationParam & param) {
				float progress = param.progress;
				RgbColor updatedColor;
				//updatedColor = RgbColor::LinearBlend(originalcolor, targetcolor, progress);
				updatedColor =  HslColor::LinearBlend<NeoHueBlendShortestDistance>(originalcolor, targetcolor, progress);
				strip->SetPixelColor(i, updatedColor);
			};

			animator->StartAnimation(i,  1000 , animUpdate);
		}


	}

	_timeout = millis(); 
}



bool ColorBlend::Stop()
{

	//strip->ClearTo(0);

	// for (uint16_t i = 0; i < _pixels; i++) {
	// 	RgbColor originalcolor = strip->GetPixelColor(i);
	// 	AnimUpdateCallback animUpdate = [ this, originalcolor, i ](const AnimationParam & param) {
	// 		float progress = param.progress;
	// 		RgbColor updatedColor;
	// 		updatedColor = RgbColor::LinearBlend(originalcolor, 0, progress);
	// 		strip->SetPixelColor(i, updatedColor);
	// 	};

	// 	animator->StartAnimation(i,  1000 , animUpdate);
	// }

	if (animator) {
		delete animator;
		animator = nullptr;
	}

}



