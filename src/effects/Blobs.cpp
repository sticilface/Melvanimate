#include "effects/Blobs.h"
#include <NeopixelBus.h>
#include <NeoPixelAnimator.h>
#include "mybus.h"
#include "helperfunc.h"
#include "Palette.h"

using namespace helperfunc;

extern MyPixelBus * strip;
extern NeoPixelAnimator * animator;

void Blobs::shape(EffectObjectHandler * obj)
{
	if (_shape) {
		(_shape)(obj);
	}
}


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

	for (uint8_t i = 0; i < effectnumber(); i++) {
		_vars->manager->Add(i, 100 , new SimpleEffectObject()); // if its 2d then no need to hold so many pixels
	}

	Serial.printf("[Blobs::Start] size = %u\n", size() / 2 );

	////////////

	setshape( std::bind(&Blobs::drawCircle, this, _1 )); 

	// need to make array of new pixels temporary... to exclude it from the inuse search...
	// or exclude current handle form it...

	////////////

	//shapefn(shape);

	for (uint8_t obj = 0; obj < effectnumber(); obj++) {

		SimpleEffectObject * current =  static_cast<SimpleEffectObject*>(_vars->manager->Get(obj));  //    pointer

		// nullptr protection
		if (!current) { Serial.printf("[Blobs::Start] !current bailing\n"); ; break; }

		current->SetObjectUpdateCallback( [ current, this ]() {


			uint16_t pixel_count = 0;

			//  count the number of pixels... not sure of a better way to get this from adafruit lib...
			if (matrix()) {

				if (size() * 2 < matrix()->width() || size() * 2 < matrix()->height() ) {
					current->x = random(0, matrix()->width() - size() + 1);
					current->y = random(0, matrix()->height() - size() + 1);

				} else {

					current->x = random(0, matrix()->width() );
					current->y = random(0, matrix()->height() );

				}

				current->size = size(); 

				matrix()->setShapeFn( [ &pixel_count ] (uint16_t pixel, int16_t x, int16_t y) {
					pixel_count++;
				});

				//drawfunc(matrix());
				//matrix()->fillCircle(current->x, current->y, size() / 2, 0); //  fills shape with
				shape(current);

				//matrix()->drawCircle(x, y, size(), 0); //  fills shape with

				current->create(pixel_count);

				pixel_count = 0;

				matrix()->setShapeFn( [ current, &pixel_count ] (uint16_t pixel, int16_t x, int16_t y) {
					if (current->pixels()) {
						current->pixels()[pixel_count++] = pixel;
					}
				});

				//drawfunc(matrix());

				//matrix()->fillCircle(current->x, current->y, size() / 2 , 0); //  fills shape with
				shape(current);

			} else {
				//  linear points creation
				current->create(size());

				current->x = random(0, strip->PixelCount() - size() + 1);

				for (uint16_t pixel = 0; pixel < size(); pixel++) {
					current->pixels()[pixel] = current->x + pixel;
				}

			}



			// pixels chosen check not in use by another animator...

			for (uint16_t i = 0; i < current->total(); i++) {

				if (_vars->manager->Inuse(current,  current->pixels()[i] ) ) {
					//Serial.printf("[inuse yes]\n");
					return false;
					//current->end();
					//break;
				}
			}

			if (!current->pixels()) { return false ; } //  break if there are no pixels to draw.

//			Serial.printf("[Blobs::Start] after pixels return effect %u\n", current->id());


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

				if (param.state == AnimationState_Completed) {
					current->end();
				}

			};

			// now use the animation properties we just calculated and start the animation
			// which will continue to run and call the update function until it completes

			uint32_t lower = map( speed(), 0, 255, 100, 5000 );
			uint32_t upper = map( speed(), 0, 255, lower, lower + 5000 );

			uint32_t timefor = random(lower, upper );

			current->Timeout(timefor);

			animator->StartAnimation(current->id(), timefor - 50 , animUpdate);
			return true;

		});

	}


	Serial.printf("[Blobs::Start] Done\n");

}

bool Blobs::Run()
{
	if (_vars) {

		//if (millis() - _vars->lasttick > 1000) {

		setshape( std::bind(&Blobs::drawCircle, this, _1 )); 


		if (_vars->manager) {

			_vars->manager->Update();
		}

		//_vars->lasttick = millis();
		//}
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


void Blobs::fillcircle(EffectObjectHandler * Object) 
{
	matrix()->fillCircle(Object->x, Object->y, Object->size / 2, 0); //  fills shape with

}

void Blobs::drawCircle(EffectObjectHandler * Object)  
{
	matrix()->drawCircle(Object->x, Object->y, Object->size, 0); //  fills shape with
}

void Blobs::drawRect(EffectObjectHandler * Object)  
{
    matrix()->drawRect(Object->x, Object->y,  Object->size, Object->size, 0); //  fills shape with
}
void Blobs::fillRect(EffectObjectHandler * Object)  
{
    matrix()->fillRect(Object->x, Object->y,  Object->size, Object->size, 0); //  fills shape with
}


