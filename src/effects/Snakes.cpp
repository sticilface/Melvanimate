#include "Snakes.h"
#include <NeopixelBus.h>
#include <NeoPixelAnimator.h>
#include "mybus.h"
#include "helperfunc.h"
#include "Palette.h"

using namespace helperfunc;

extern MyPixelBus * strip;
extern NeoPixelAnimator * animator;



bool Snakes::InitVars()
{
	addVar(new Variable<uint8_t>("brightness", 10));
	addVar(new Variable<uint8_t>("speed", 10));
	addVar(new Variable<bool>("use_matrix", true));
	addVar(new Variable<MelvtrixMan*>("Matrix"));  // must be called Matrix.  very importnat...
	addVar(new Variable<Palette*>("Palette", WHEEL));
	addVar(new Variable<uint8_t>("effectnumber", 1));
	addVar(new Variable<uint8_t>("size", 3));
	addVar(new Variable<uint8_t>("shapemode", 1));

	if (_vars) {
		delete _vars;
	}
	_vars = new SnakesVars;
}


bool Snakes::Start()
{
	strip->ClearTo(0);


	matrixMan()->enable();


	if (_vars->manager) {
		delete _vars->manager;
	}

	_vars->manager = new EffectGroup;

	for (uint8_t i = 0; i < effectnumber(); i++) {
		_vars->manager->Add(i, 100 , new AnimatedEffectObject(matrix())); // if its 2d then no need to hold so many pixels
	}


	for (uint8_t obj = 0; obj < effectnumber(); obj++) {

		AnimatedEffectObject * current =  static_cast<AnimatedEffectObject*>(_vars->manager->Get(obj));  // cast handler to the Blobs class...

		if (!current) { break; }

		current->x = random(0, matrix()->width() );
		current->y = random(0, matrix()->height() );
		current->size = size();


		current->SetObjectUpdateCallback( [ current, this ]() {

			uint16_t pixel_count = 0;

			//  count the number of pixels... not sure of a better way to get this from adafruit lib...


//  use this

//			getPixel(x,y); 
			

			matrix()->setShapeFn( [ &pixel_count ] (uint16_t pixel, int16_t x, int16_t y) {
				pixel_count++;
			});


			current->create(pixel_count);
			pixel_count = 0;

			matrix()->setShapeFn( [ current, &pixel_count ] (uint16_t pixel, int16_t x, int16_t y) {
				if (current->pixels()) {
					current->pixels()[pixel_count++] = pixel;
				}
			});




			// pixels chosen check not in use by another animator...

			for (uint16_t i = 0; i < current->total(); i++) {
				if (_vars->manager->Inuse(current,  current->pixels()[i] ) ) {
					return false;
				}
			}

			if (!current->pixels()) { return false ; } //  break if there are no pixels to draw.

			RgbColor targetColor = palette()->next();

			AnimUpdateCallback animUpdate = [ targetColor, current, this ](const AnimationParam & param) {
				// progress will start at 0.0 and end at 1.0
				// we convert to the curve we want
				float progress = param.progress;

				RgbColor updatedColor;

				if (progress < 0.5) {
					updatedColor = RgbColor::LinearBlend(0, targetColor, progress * 2.0f);
				} else {
					updatedColor = RgbColor::LinearBlend(targetColor, 0, (progress - 0.5f) * 2.0f);
				}

				if (current->pixels()) {
					for (uint16_t i = 0; i < current->total(); i++) {
						strip->SetPixelColor( current->pixels()[i] , dim(updatedColor, brightness()));
					}
				}
				///  maybe need to delete pixels() at end of the effect here...
				//  to remove it from the in use....

				//if (param.state == AnimationState_Completed) {
				//	current->end();
				//}

			};

			// now use the animation properties we just calculated and start the animation
			// which will continue to run and call the update function until it completes

			uint32_t lower = map( speed(), 0, 255, 100, 10000 );
			uint32_t upper = map( speed(), 0, 255, lower, lower + 10000 );

			uint32_t timefor = random(lower, upper );

			current->Timeout(timefor);

			animator->StartAnimation(current->id(), timefor - 50 , animUpdate);
			return true;

		});

	}

}

bool Snakes::Run()
{
	if (_vars) {
		if (_vars->manager) {
			_vars->manager->Update();
		}
	}
}

bool Snakes::Stop()
{
	if (animator) {
		delete animator;
		animator = nullptr;
	}

	if (_vars->manager) {
		delete _vars->manager;
	}

	if (_vars) {
		delete _vars;
		_vars = nullptr;
	}
}









