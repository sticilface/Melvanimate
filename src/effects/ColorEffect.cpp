#include "ColorEffect.h"

#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include "mybus.h"
#include "helperfunc.h"

using namespace helperfunc;

extern MyPixelBus * strip;
extern NeoPixelAnimator * animator;



bool ColorEffect::InitVars(){
        addVar(new Variable<uint8_t>("brightness", 50));
        addVar(new Variable<RgbColor>("color1", RgbColor(0)));
        addVar(new Variable<Palette*>("Palette", Palette::OFF));

        if (animator) {
                delete animator;
                animator = nullptr;
        }

        //_pixels =  ( strip->PixelCount() < MAX_NUMBER_OF_ANIMATIONS ) ? strip->PixelCount() : MAX_NUMBER_OF_ANIMATIONS;

        animator = new NeoPixelAnimator( 1  );


}

bool ColorEffect::Start(){

        _actual_color = color();
        _nextColor = _actual_color;

        Serial.println("START CALLED");

        if (palette()) {
                palette()->randommode(Palette::TIME_BASED_RANDOM);
                palette()->input(color());

                palette()->attachCallback( [this] () {
                        this->_nextColor = palette()->next();
                        setVar("color1", _nextColor);
                        Serial.printf("Pallet CB: _nextColor = (%u,%u,%u)\n", _nextColor.R, _nextColor.G, _nextColor.B);
                        Draw();
                });
        }

        Draw();

}

bool ColorEffect::Stop(){
        Serial.println("STOP CALLED");

        if (animator) {
          if (animator->IsAnimating()) {
            Serial.println("Animator is Animating");
          }
        }
        // if (strip) {
        //         strip->ClearTo(RgbColor(0));
        // }
}

bool ColorEffect::Run() {

        if (!strip || !animator) {
                return false;
        }

        if (palette()) {
                palette()->loop();
        }

}

void ColorEffect::Refresh() {
        Serial.println("REFRESH CALLED");

        _nextColor = color();
        Draw();

}

void ColorEffect::Draw() {

        if (!animator) {
          return;
        }

        RgbColor oldcolor = _actual_color;

        _actual_color = dim(_nextColor, brightness());

        Serial.printf("DRAWING: [%u:%u:%u]\n", _nextColor.R, _nextColor.G, _nextColor.B );


        AnimUpdateCallback animUpdate = [ = ](const AnimationParam & param) {

                                                float progress = param.progress;

                                                RgbColor updatedColor = RgbColor::LinearBlend( oldcolor, _actual_color, progress);
                                                if (strip) {
                                                        for (uint16_t pixel = 0; pixel < strip->PixelCount(); pixel++) {
                                                                strip->SetPixelColor(pixel, updatedColor);
                                                        }
                                                }

                                        };

        animator->StartAnimation(0, 1000, animUpdate);

}
