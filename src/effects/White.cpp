#include "White.h"
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include "mybus.h"
#include "helperfunc.h"
#include "Melvanimate.h" // required for the MAX_NUMBER_OF_ANIMATIONS definition


using namespace helperfunc;

extern MyPixelBus * strip;
extern NeoPixelAnimator * animator;


bool White::Start()
{
	if (animator) {

		delete animator;
		animator = nullptr;
	}

	animator = new NeoPixelAnimator(  ( strip->PixelCount() < MAX_NUMBER_OF_ANIMATIONS) ? strip->PixelCount() : MAX_NUMBER_OF_ANIMATIONS  );


	if (animator) {

		AnimEaseFunction easing = NeoEase::QuadraticInOut;

	//	for (uint16_t pixel = 0; pixel < strip->PixelCount(); pixel++) {

			//RgbwColor originalColor = strip->GetPixelColor(pixel);
			MyColorFeature::ColorObject originalColor = RgbColor(0); // strip->GetPixelColor(pixel);

			AnimUpdateCallback animUpdate = [ = ](const AnimationParam & param) {
				//float progress = easing(param.progress);
				float progress = param.progress;
				//MyColorFeature::ColorObject  updatedColor = RgbwColor::LinearBlend(originalColor, RgbwColor(0, 0, 0, brightness() ), progress);
				 MyColorFeature::ColorObject  updatedColor = MyColorFeature::ColorObject::LinearBlend(originalColor, MyColorFeature::ColorObject(brightness()), progress);
				//if (strip->PixelsSize() == 4) {
				if (strip) {
				for (uint16_t pixel = 0; pixel < strip->PixelCount(); pixel++) {
					strip->SetPixelColor(pixel, updatedColor);
				}
			}
				//}
			};

			animator->StartAnimation(0, 1000, animUpdate);

//		}
	}
}

bool White::Run() {

	if (animator && !animator->IsAnimating()) {
		delete animator;
		animator = nullptr;
	}

}
