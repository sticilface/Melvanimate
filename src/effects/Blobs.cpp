#include "effects/Blobs.h"
#include <NeopixelBus.h>
#include <NeoPixelAnimator.h>
#include "mybus.h"
#include "helperfunc.h"
#include "Palette.h"

using namespace helperfunc;

extern MyPixelBus * strip;
extern NeoPixelAnimator * animator;




bool Blobs::InitVars()
{
	addVar(new Variable<uint8_t>("brightness", 10));
	addVar(new Variable<uint8_t>("speed", 10));
	addVar(new Variable<bool>("use_matrix", true));
	addVar(new Variable<MelvtrixMan*>("Matrix"));  // must be called Matrix.  very importnat...
	addVar(new Variable<Palette*>("Palette", WHEEL));
	addVar(new Variable<uint8_t>("effectnumber", 1));
	addVar(new Variable<uint8_t>("size", 3));

	if (_vars) {
		delete _vars;
	}
	_vars = new BlobsVars;
}


bool Blobs::Start()
{

	strip->ClearTo(0);
	//palette->mode(WHEEL);

	Serial.printf("[Blobs::Start] Creating Objects (%u)\n", ESP.getFreeHeap());

	if (matrixMan()) {
		if (usematrix()) {
			Serial.printf("[Blobs::Start] Enable matrix\n");
			matrixMan()->enable();
		} else {
			Serial.printf("[Blobs::Start] Disable matrix\n");
			matrixMan()->disable();
		}
	}

	if (animator) {
		delete animator;
		animator = nullptr;
	}
	Serial.printf("[Blobs::Start] Create Animator\n");
	animator = new NeoPixelAnimator(effectnumber());

	if (_vars->manager) {
		delete _vars->manager;
	}
	Serial.printf("[Blobs::Start] Create manager\n");

	_vars->manager = new EffectGroup;

	// if (_vars->pPosition) {
	// 	delete[] _vars->pPosition;
	// }
	// Serial.printf("[Blobs::Start] Create pPosition\n");
	// _vars->pPosition = new position_s[effectnumber()];

	// if (_vars->pGroup) {
	// 	delete[] _vars->pGroup;
	// }
	// Serial.printf("[Blobs::Start] Create pGroup\n");

	// _vars->pGroup = new EffectObjectHandler* [effectnumber()];

	// Serial.printf("[Blobs::Start] Fill pGroup..");

	for (uint8_t i = 0; i < effectnumber(); i++) {
		// EffectObjectHandler* Add(uint16_t i, uint32_t timeout, EffectObjectHandler* Fn);
		//_vars->pGroup[i] =  _vars->manager->Add(i, 100 , new EffectObject( (usematrix()) ? size() * size() : size()) ); // if its 2d then no need to hold so many pixels
		_vars->manager->Add(i, 100 , new SimpleEffectObject()); // if its 2d then no need to hold so many pixels

	}
	Serial.printf("[Blobs::Start] size = %u\n", size() / 2 );



	////////////
	////////////

	for (uint8_t obj = 0; obj < effectnumber(); obj++) {

		SimpleEffectObject * current =  static_cast<SimpleEffectObject*>(_vars->manager->Get(obj));  //    pointer

		// nullptr protection
		if (!current) { Serial.printf("[Blobs::Start] !current bailing\n"); ; break; }

		current->SetObjectUpdateCallback( [ current, this, obj ]() {


			uint16_t pixel_count = 0;

			//  count the number of pixels... not sure of a better way to get this from adafruit lib...
			if (matrix()) {

				uint16_t x = current->x = random(0, matrix()->width() - size() + 1);
				uint16_t y = current->y = random(0, matrix()->height() - size() + 1);

				matrix()->setShapeFn( [ &pixel_count ] (uint16_t pixel, int16_t x, int16_t y) {
					pixel_count++;
				});

				//drawfunc(matrix());
				matrix()->fillCircle(x, y, size() / 2, 0); //  fills shape with

				//matrix()->drawCircle(x, y, size(), 0); //  fills shape with

				current->create(pixel_count);
				pixel_count = 0;

				matrix()->setShapeFn( [ current, &pixel_count ] (uint16_t pixel, int16_t x, int16_t y) {
					current->pixels[pixel_count++] = pixel;
				});

				//drawfunc(matrix());

				matrix()->fillCircle(x, y, size() / 2 , 0); //  fills shape with

			} else {

				current->create(size());

				uint16_t x = current->x = random(0, strip->PixelCount() - size() + 1);

				for (uint16_t pixel = 0; pixel < size(); pixel++) {
					current->pixels[pixel] = x + pixel;
				}

			}

			//matrix()->fillRect(x, y, x + size(), y + size(), 0); //  fills shape with

			//fillCircle
			//drawCircle
			//fillRect


			RgbColor targetColor = palette()->next();

			AnimUpdateCallback animUpdate = [ targetColor, current, this  ](const AnimationParam & param) {
				// progress will start at 0.0 and end at 1.0
				// we convert to the curve we want
				float progress = param.progress;

				RgbColor updatedColor;

				if (progress < 0.5) {
					updatedColor = RgbColor::LinearBlend(0, targetColor, progress * 2.0f);
				} else {
					updatedColor = RgbColor::LinearBlend(targetColor, 0, (progress - 0.5f) * 2.0f);
				}

				for (uint16_t i = 0; i < current->total; i++) {
					strip->SetPixelColor( current->pixels[i] , dim(updatedColor, brightness()));
				}

			};

			// now use the animation properties we just calculated and start the animation
			// which will continue to run and call the update function until it completes
			animator->StartAnimation(obj, 1000, animUpdate);




		});

	}


	Serial.printf("[Blobs::Start] Done\n");

}

bool Blobs::Run()
{
	if (_vars) {

		if (millis() - _vars->lasttick > 2000) {

			if (_vars->manager) {

				_vars->manager->Update();
			}
			_vars->lasttick = millis();
		}
	}
}

bool Blobs::Stop()
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

// void Blobs::drawfunc(Melvtrix* matrix, ) {

// 	if (matrix) {
// 	 matrix()->drawRect(x, y,  size(), size(), 0); //  fills shape with
// 	}

// }




