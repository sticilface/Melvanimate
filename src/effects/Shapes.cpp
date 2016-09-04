#include "Shapes.h"
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include "mybus.h"
#include "helperfunc.h"
#include "Palette.h"
#include "EQ.h"

using namespace helperfunc;

extern MyPixelBus * strip;
extern NeoPixelAnimator * animator;

void Shapes::shape(SimpleEffectObject * obj)
{
	if (_shape) {
		(_shape)(obj);
	}
}


bool Shapes::InitVars()
{
	addVar(new Variable<uint8_t>("brightness", 10));
	addVar(new Variable<uint8_t>("speed", 10));
	addVar(new Variable<RgbColor>("color1", RgbColor(0)));
	addVar(new Variable<bool>("use_matrix", true));
	addVar(new Variable<MelvtrixMan*>("Matrix"));  // must be called Matrix.  very importnat...
	addVar(new Variable<Palette*>("Palette", Palette::WHEEL));
	addVar(new Variable<uint8_t>("effectnumber", 1));
	addVar(new Variable<uint8_t>("size", 3));
	addVar(new Variable<uint8_t>("shapemode", 1));
	addVar(new Variable<EQ*>(1000, 3000)); //  EQ(samples, sampletime) //  sets the defaults for EQ beat detection...  Start with no params for just graphic equaliser settings.

	if (_vars) {
		delete _vars;
	}
	_vars = new ShapesVars;
}


bool Shapes::Start()
{
	if (!strip) {
		return false;
	}

	strip->ClearTo(0);

	if (matrixMan()) {
		if (usematrix()) {
			matrixMan()->enable();
		} else {
			matrixMan()->disable();
		}
	}

	if (animator) {
		delete animator;
		animator = nullptr;
	}

	animator = new NeoPixelAnimator(effectnumber());

	if (_vars->manager) {
		delete _vars->manager;
	}

	_vars->manager = new EffectGroup;

	for (uint8_t i = 0; i < effectnumber(); i++) {
		_vars->manager->Add(i, new SimpleEffectObject()); // if its 2d then no need to hold so many pixels
	}

	setshape( shapemode() );

	for (uint8_t obj = 0; obj < effectnumber(); obj++) {

		SimpleEffectObject * current =  static_cast<SimpleEffectObject*>(_vars->manager->Get(obj));  // cast handler to the Blobs class...

		if (!current) { break; }

		current->end();

		current->SetObjectUpdateCallback( [ current, this ]() {

			uint32_t starttime = micros();

			palette()->input(color());

			uint16_t pixel_count = 0;

			//  count the number of pixels... not sure of a better way to get this from adafruit lib...
			if (matrix()) {

				setshape(shapemode());

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

				shape(current); //  this calls function above to populate pixel_count....
				current->create(pixel_count); //  create the memory for that number of pixels
				pixel_count = 0;

				matrix()->setShapeFn( [ current, &pixel_count ] (uint16_t pixel, int16_t x, int16_t y) {
					if (current->pixels()) {
						current->pixels()[pixel_count++] = pixel;
					}
				});

				shape(current); // calls the above function to draw it...

			} else {
				//  linear points creation
				//  This kicks in if the Matrix is disabled to give simple linear effects...
				current->create(size());
				current->x = random(0, strip->PixelCount() - size() + 1);

				for (uint16_t pixel = 0; pixel < size(); pixel++) {
					current->pixels()[pixel] = current->x + pixel;
				}

			}

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

				if (progress < 0.5f) {
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

				// if (param.state == AnimationState_Completed) {
				// 	current->end();
				// }

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

bool Shapes::Run()
{
	if (_vars) {
		if (_vars->manager) {

			_vars->manager->Update();
			//cpuCycleTimer();

		}
	}
}

bool Shapes::Stop()
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

void Shapes::setshape(Shapetype shape)
{
	using namespace std::placeholders;


	switch ( shape ) {

	case RANDOM: {
		setshape( (Shapetype)random(1, 7));
		break;
	}
	case FILLCIRCLE: {
		setshape( std::bind(&Shapes::fillCircle, this, _1 ));
		break;
	}
	case DRAWCIRCLE: {
		setshape( std::bind(&Shapes::drawCircle, this, _1 ));
		break;
	}
	case FILLSQUARE: {
		setshape( std::bind(&Shapes::fillRect, this, _1 ));
		break;
	}
	case DRAWSQUARE: {
		setshape( std::bind(&Shapes::drawRect, this, _1 ));
		break;
	}
	case FILLTRIANGLE: {
		setshape( std::bind(&Shapes::fillTriangle, this, _1 ));
		break;
	}
	case DRAWTRIANGLE: {
		setshape( std::bind(&Shapes::drawTriangle, this, _1 ));
		break;
	}

	}
}

void Shapes::fillCircle(SimpleEffectObject * Object)
{
	if (!Object || !matrix()) { return; }
	matrix()->fillCircle(Object->x, Object->y, Object->size / 2, 0); //  fills shape with
}

void Shapes::drawCircle(SimpleEffectObject * Object)
{
	if (!Object || !matrix()) { return; }
	matrix()->drawCircle(Object->x, Object->y, Object->size / 2, 0); //  fills shape with
}

void Shapes::drawRect(SimpleEffectObject * Object)
{
	if (!Object || !matrix()) { return; }
	matrix()->drawRect(Object->x, Object->y,  Object->size, Object->size, 0); //  fills shape with
}
void Shapes::fillRect(SimpleEffectObject * Object)
{
	if (!Object || !matrix()) { return; }
	matrix()->fillRect(Object->x, Object->y,  Object->size, Object->size, 0); //  fills shape with
}

void Shapes::fillTriangle(SimpleEffectObject * Object)
{
	if (!Object || !matrix()) { return; }
	matrix()->fillTriangle(Object->x, Object->y, Object->x + Object->size, Object->y, Object->x + (Object->size / 2), Object->y + Object->size , 0); //  fills shape with
}

void Shapes::drawTriangle(SimpleEffectObject * Object)
{
	if (!Object || !matrix()) { return; }
	matrix()->drawTriangle(Object->x, Object->y, Object->x + Object->size, Object->y, Object->x + (Object->size / 2), Object->y + Object->size , 0); //  fills shape with
}
