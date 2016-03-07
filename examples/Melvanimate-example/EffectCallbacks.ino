/* -- TEMPLATE

void SnakesFn(SwitchEffect::effectState state)
{
  static uint32_t tick = 0;
  if (millis() - tick < 1000) return;
  tick = millis();

  switch (state)
  {

  case SwitchEffect::PRE_EFFECT:
  {
    Serial.println("Init: Test");

  }

  break;
  case SwitchEffect::RUN_EFFECT:
  {
    Serial.println("Run: Test");

  }
  break;

  case SwitchEffect::POST_EFFECT:
  {
    Serial.println("End: Test");

  }
  break;
  }
  strip.Show();
}

*/


/*-----------------------------------------------
*
*                      offFn
*
*------------------------------------------------*/


void offFn(effectState &state, EffectHandler* ptr)
{

  if (ptr) {

    SwitchEffect& effect = *static_cast<SwitchEffect*>(ptr);

    switch (state) {

    case PRE_EFFECT: {

      if (animator) {
        delete animator;
      }

      // have to be careful of number of pixels..
      if (strip->PixelCount() < MAX_NUMBER_OF_ANIMATIONS ) {
        animator = new NeoPixelAnimator(strip->PixelCount());
      }

      effect.SetTimeout(1000); //  set speed through the effect
      lights.autoWait(); //  halts progress through states untill animator has finished..

      if (animator) {

        AnimEaseFunction easing = NeoEase::QuadraticInOut;

        for (uint16_t pixel = 0; pixel < strip->PixelCount(); pixel++) {

          RgbColor originalColor = strip->GetPixelColor(pixel);

          AnimUpdateCallback animUpdate = [ = ](const AnimationParam & param) {
            //float progress = easing(param.progress);
            float progress = param.progress;
            RgbColor updatedColor = RgbColor::LinearBlend(originalColor, RgbColor(0), progress);
            strip->SetPixelColor(pixel, updatedColor);
          };

          animator->StartAnimation(pixel, 1000, animUpdate);

        }
      }

    }

    break;
    case RUN_EFFECT: {
      strip->ClearTo(0);
      if (animator) {
        delete animator;
        animator = nullptr;
      }

    }
    break;
    case POST_EFFECT: {

    }
    }
  }
}

/*-----------------------------------------------
*
*                      offFn
*
*------------------------------------------------*/

void NeoPixDemoFn(effectState &state, EffectHandler* ptr)
{

  if (ptr) {

    SwitchEffect& effect = *static_cast<SwitchEffect*>(ptr);

    switch (state) {

    case PRE_EFFECT: {
      Serial.println("[NeoPixDemoFn] PRE_EFFECT");

      if (animator) {
        delete animator;
      }

      animator = new NeoPixelAnimator(1);

      if (strip) {
        strip->ClearTo(RgbColor(0, 0, 0));
      }
    }
    break;
    case RUN_EFFECT: {

      if (animator && !animator->IsAnimating()) {

        static uint8_t randomvar = 0;

        //strip->ClearTo(RgbColor(0, 0, 0));
        const uint8_t peak = 128;

        RgbColor targetColor = RgbColor(random(peak), random(peak), random(peak));

        // pick a random duration of the animation for this pixel
        // since values are centiseconds, the range is 1 - 4 seconds
        uint16_t time = 5000; //random(100, 400);

        // each animation starts with the color that was present
        RgbColor originalColor = strip->GetPixelColor(0);
        // and ends with a random color
        // with the random ease function
        AnimEaseFunction easing;

        // switch (randomvar) {
        // case 0:
        //   easing = NeoEase::QuadraticInOut;
        //   break;
        // case 1:
        //   easing = NeoEase::CubicInOut;
        //   break;
        // case 2:
        //   easing = NeoEase::QuarticInOut;
        //   break;
        // case 3:
        //   easing = NeoEase::QuinticInOut;
        //   break;
        // case 4:
        //   easing = NeoEase::SinusoidalInOut;
        //   break;
        // case 5:
        //   easing = NeoEase::ExponentialInOut;
        //   break;
        // case 6:
        //   easing = NeoEase::CircularInOut;
        //   break;
        // }

        easing = NeoEase::QuadraticInOut;

        /*
        enum AnimationState
        {
        AnimationState_Started,
        AnimationState_Progress,
        AnimationState_Completed
        };

        struct AnimationParam
        {
        float progress;
        uint16_t index;
        AnimationState state;
        };

        */

        AnimUpdateCallback animUpdate = [ = ](const AnimationParam & param) {
          // progress will start at 0.0 and end at 1.0
          // we convert to the curve we want
          float progress = easing(param.progress);
          RgbColor updatedColor;



          if (progress < 0.5) {
            updatedColor = RgbColor::LinearBlend(originalColor, targetColor, progress * 2.0f);
          } else {
            updatedColor = RgbColor::LinearBlend(targetColor, 0, (progress - 0.5f) * 2.0f);
          }

          // use the curve value to apply to the animation
          for (uint16_t pixel = 0; pixel < strip->PixelCount(); pixel++) {

            strip->SetPixelColor(pixel, updatedColor);

          }

        };

        // now use the animation properties we just calculated and start the animation
        // which will continue to run and call the update function until it completes
        animator->StartAnimation(0, time, animUpdate);






      } // if animator is animating


    } // end of run effect
    break;

    } // end of switch


  } // end of if(ptr)
} // end of Fn







/*-----------------------------------------------
*
*                 Timing Test
*
*------------------------------------------------*/

void TimingFn(effectState & state, EffectHandler * ptr)
{

  switch (state) {
    static uint32_t tock = 0;
  case PRE_EFFECT: {
    Serial.println("PRE_EFFECT - START");
    tock = millis();
    lights.setWaiting(true);
    timer.setTimeout(5000, []() {
      lights.setWaiting(false);
      Serial.println("PRE_EFFECT - END");

    } );

  }

  break;
  case RUN_EFFECT: {
    static uint32_t tick = 0;
    if (millis() - tick > 1000) {
      Serial.println("RUN");
      tick = millis();
    }


    if (millis() - tock > 10000) {
      Serial.println("RESET");
      state = PRE_EFFECT;
      tock = millis();
    }

  }
  break;

  case POST_EFFECT: {
    Serial.println("POST_EFFECT - START");
    lights.setWaiting(true);
    timer.setTimeout(5000, []() {
      Serial.println("POST_EFFECT - END");
      lights.setWaiting(false);
    } ) ;

  }
  break;
  }
}


void DummyFn(effectState & state, EffectHandler * ptr)
{
  static uint32_t tick = 0;
  if (millis() - tick < 1000) { return; }
  tick = millis();
  //Serial.println("Dummy Run");

}

void CascadeEffectFn(effectState & state, EffectHandler * ptr)
{
  static uint32_t tick = 0;
  if (millis() - tick < 1000) { return; }
  tick = millis();
  //Serial.println("Dummy Run");

}

/*-----------------------------------------------
*
*                 SimpleColorFn
*
*------------------------------------------------*/

void SimpleColorFn(effectState &state, EffectHandler* ptr)
{

  if (ptr) {

    SimpleEffect& effect = *static_cast<SimpleEffect*>(ptr);
    RgbColor newColor = dim( effect.color(), effect.brightness() );

    switch (state) {

    case PRE_EFFECT: {
//     Serial.println("[SimpleColorFn] PRE_EFFECT Called");

      if (animator) {
        delete animator;
      }

      // have to be careful of number of pixels..
      if (strip->PixelCount() < MAX_NUMBER_OF_ANIMATIONS ) {
        animator = new NeoPixelAnimator(strip->PixelCount());
      }

      effect.SetTimeout(2000); //  set speed through the effect

      lights.autoWait(); //  halts progress through states untill animator has finished..

      if (animator) {
//       Serial.println("[SimpleColorFn] PRE_EFFECT animator exists");
        AnimEaseFunction easing = NeoEase::QuadraticInOut;

        for (uint16_t pixel = 0; pixel < strip->PixelCount(); pixel++) {

          RgbColor originalColor = strip->GetPixelColor(pixel);

          AnimUpdateCallback animUpdate = [ = ](const AnimationParam & param) {
            //float progress = easing(param.progress);
            float progress = param.progress;
            RgbColor updatedColor = RgbColor::LinearBlend(originalColor, newColor, progress);
            strip->SetPixelColor(pixel, updatedColor);
          };

          animator->StartAnimation(pixel, 1000, animUpdate);

        }
      }
      break;
    }

    case RUN_EFFECT: {
      if (strip) {
        strip->ClearTo(newColor);
      }
      if (animator) {
        delete animator;
        animator = nullptr; 
      }
      break;
    }
    case POST_EFFECT: {
      if (animator) {
        delete animator;
        animator = nullptr;
      }
      break;
    }
    case EFFECT_REFRESH: {
      state = PRE_EFFECT;
//      Serial.println("[SimpleColorFn] Refresh Called");
      break;
    }

    }
  }
}

/*-----------------------------------------------
*
*                      Marquee
*
*------------------------------------------------*/
/*
void displaytext(const char * text, uint16_t timeout, RgbColor color);

void MarqueeFn(effectState state, EffectHandler* ptr)
{

  // cast handler to correct derived class... very handy...

  MarqueeEffect* effect = static_cast<MarqueeEffect*> (ptr);

  Palette palette = effect->_palette;

  if (effect) {

    switch (state) {

    case PRE_EFFECT: {
      Serial.println("[MarqueeFn] PRE_EFFECT");
      strip->ClearTo(0);
      lights.matrix()->setRotation(effect->getRotation());

      //lights.palette().mode(pal);
      //lights.palette().total(255);



    }

    break;
    case RUN_EFFECT: {



      uint8_t speed = effect->_speed;
      const char * text = effect->getText();

      palette.input( effect->_color );

      RgbColor dimmedcolor = lights.dim( palette.next() , effect->_brightness);

      lights.SetTimeout( speed * 10);


      displaytext(text, speed * 10,  dimmedcolor);

    }
    break;

    case POST_EFFECT: {
      lights.autoWait();
      FadeTo(500, 0);
    }
    break;
    case EFFECT_REFRESH: {
      Serial.println("Refresh called");
      lights.timeoutvar = lights.getX();
      lights.matrix()->setRotation(effect->getRotation());
      strip->ClearTo(0);
      break;
    }

    }
  }

}

// Use the fade to and back callback!
void displaytext(const char * text, uint16_t timeout, RgbColor color)
{

  static bool reset = true;

  if (reset) {
    lights.timeoutvar = lights.getX();
    reset = false;
  }
  const uint16_t len = strlen(text) * 7;

  Melvtrix & matrix = *lights.matrix();
  matrix.setTextWrap(false);

  if (lights.timeoutvar < lights.getX()) {
    matrix.setShapeFn( [] (uint16_t pixel, int16_t x, int16_t y) {
      strip->SetPixelColor(pixel, 0);
    });

    matrix.setCursor( lights.timeoutvar + 1, lights.getY() - 8  );
    matrix.print(text);
  }


  matrix.setShapeFn( [color] (uint16_t pixel, int16_t x, int16_t y) {
    strip->SetPixelColor(pixel, color  );
  });

  matrix.setCursor( lights.timeoutvar--,  lights.getY() - 8  );
  matrix.print(text);

  if (lights.timeoutvar < -len) {
    reset = true;
  }

}
*/

/*-----------------------------------------------
*
*                      Rainbow Cycle
*
*------------------------------------------------*/


// void RainbowCycleFn(effectState state)
// {

//   Melvtrix & matrix = *lights.matrix();

//   switch (state) {

//   case PRE_EFFECT: {

//     lights.autoWait();
//     Debugf(" Matrix height: %u\n", matrix.height());
//     Debugf(" Matrix width: %u\n", matrix.width());


//     Debugf(" Add x: %s\n", ( (lights.matrix())->width() > 1 ) ? "true" : "false" );

//     matrix.setShapeFn( [] (uint16_t pixel, int16_t x, int16_t y) {



//       uint16_t seqnumber = ( (lights.matrix())->width() > 1 ) ? (x * y) + x : (x * y) + y;

//       RgbColor original = strip->GetPixelColor(pixel);
//       RgbColor color = lights.dim(Palette::wheel( ((seqnumber * 256 / lights.getPixels()) + lights.effectposition) & 255) );
//       AnimUpdateCallback animUpdate = [ pixel, color, original ](float progress) {
//         RgbColor updatedColor = RgbColor::LinearBlend(original, color ,  progress) ;
//         strip->SetPixelColor(pixel, updatedColor);
//       };

//       StartAnimation(pixel, 1000, animUpdate);

//     });

//     for (int x = 0; x < matrix.width(); x++) {
//       for (int y = 0; y < matrix.height(); y++ ) {
//         matrix.drawPixel(x, y); // Adafruit drawPixel has been overloaded without color for callback use
//       }
//     }

//     lights.effectposition++;

//   }

//   break;
//   case RUN_EFFECT: {
//     //  allows per effect tuning of the timeout
//     uint32_t timeout = map(lights.speed(), 0, 255, 0 , 10000);
//     lights.SetTimeout( timeout);




//     matrix.setShapeFn( [] (uint16_t pixel, int16_t x, int16_t y) {


//       uint16_t seqnumber = ( (lights.matrix())->width() > 1 ) ? (x * y) + x : (x * y) + y;

//       RgbColor color = lights.dim(Palette::wheel( ((seqnumber * 256 / lights.getPixels()) + lights.effectposition) & 255) );
//       strip->SetPixelColor(pixel, color);
//     });

//     for (int x = 0; x < matrix.width(); x++) {
//       for (int y = 0; y < matrix.height(); y++ ) {
//         matrix.drawPixel(x, y); // Adafruit drawPixel has been overloaded without color for callback use
//       }
//     }

//     lights.effectposition++;
//     if (lights.effectposition == 256 * 5) lights.effectposition = 0;

//   }
//   break;

//   case POST_EFFECT: {
//     lights.autoWait();
//     FadeTo( lights.getBrightness() * 8, 0);
//   }
//   break;

//   case EFFECT_REFRESH: {

//     break;
//   }

//   }

// }


/*-----------------------------------------------
*
*                      Rainbow
*
*------------------------------------------------*/


// void RainbowFn(effectState state)
// {

//   Melvtrix & matrix = *lights.matrix();

//   switch (state) {

//   case PRE_EFFECT: {

//     lights.autoWait();

//     Debugf(" Matrix height: %u\n", matrix.height());
//     Debugf(" Matrix width: %u\n", matrix.width());

//     matrix.setShapeFn( [] (uint16_t pixel, int16_t x, int16_t y) {
//       uint16_t seqnumber = ( (lights.matrix())->width() > 1 ) ? (x * y) + x : (x * y) + y;
//       RgbColor original = strip->GetPixelColor(pixel);
//       RgbColor color = lights.dim(Palette::wheel( (seqnumber + lights.effectposition) & 255 ));
//       AnimUpdateCallback animUpdate = [ pixel, color, original ](float progress) {
//         RgbColor updatedColor = RgbColor::LinearBlend(original, color ,  progress) ;
//         strip->SetPixelColor(pixel, updatedColor);
//       };

//       StartAnimation(pixel, 1000, animUpdate);

//     });

//     for (int x = 0; x < matrix.width(); x++) {
//       for (int y = 0; y < matrix.height(); y++ ) {
//         matrix.drawPixel(x, y); // Adafruit drawPixel has been overloaded without color for callback use
//       }
//     }

//     lights.effectposition++;

//   }

//   break;
//   case RUN_EFFECT: {
//     //  allows per effect tuning of the timeout
//     uint32_t timeout = map(lights.speed(), 0, 255, 0 , 10000);
//     lights.SetTimeout( timeout);
//     matrix.setShapeFn( [] (uint16_t pixel, int16_t x, int16_t y) {
//       uint16_t seqnumber = ( (lights.matrix())->width() > 1 ) ? (x * y) + x : (x * y) + y;
//       RgbColor color = lights.dim(Palette::wheel( (seqnumber + lights.effectposition) & 255  ));
//       strip->SetPixelColor(pixel, color);
//     });

//     for (int x = 0; x < matrix.width(); x++) {
//       for (int y = 0; y < matrix.height(); y++ ) {
//         matrix.drawPixel(x, y); // Adafruit drawPixel has been overloaded without color for callback use
//       }
//     }

//     lights.effectposition++;
//     if (lights.effectposition == 256 * 5) lights.effectposition = 0;

//   }
//   break;

//   case POST_EFFECT: {
//     lights.autoWait();
//     FadeTo(lights.getBrightness() * 8, 0);
//   }
//   break;

//   case EFFECT_REFRESH: {

//     break;
//   }

//   }

// }


/*-----------------------------------------------
*
*                      Snakes  OLD...
*
*------------------------------------------------*/




// void SnakesFn()
// {

//   typedef std::function<void()> AniObjectCallback;

//   struct AnimationVars {
//     uint16_t position = 0;
//     RgbColor colour = RgbColor(0, 0, 0);
//     XY coordinates = toXY(0, 0);
//     AniObjectCallback ObjUpdate = NULL;
//     RgbColor oldcolour = RgbColor(0, 0, 0);
//     RgbColor newcolour = RgbColor(0, 0, 0);
//     bool effectchanged = false;
//   };

//   static AnimationVars* _vars = NULL;
//   static bool Effect_Refresh_colour, Effect_Refresh_position;

//   static uint8_t animationCount;
//   static uint32_t effect_timer;
//   //static uint8_t static_colour;
//   //static uint8_t old_R, old_G, old_B;

//   static bool triggered;

//   Snakes.Timeout(30);

//   switch (Snakes.getState()) {

//   case INIT: {
//     bool overlap = false;
//     //if (!Enable_Animations) { Current_Effect_State = POST_EFFECT ; HoldingOpState = OFF; break;  } //  DO NOT RUN IF ANIMATIONS DISABLED
//     animator->FadeTo(500, RgbColor(0, 0, 0)); // fade out current effect
//     animationCount = WS2812_Settings.Effect_Count;  // assign this variable as requires re-initilisation of effect.
//     //     initialiseAnimationObject(animationCount);  // initialise animation object with correct number of animations.

//     if (_vars != NULL) delete[] _vars;
//     _vars = new AnimationVars[animationCount];  // memory for all the animated object properties...

//     for (uint8_t i = 0; i < animationCount; i++ ) {

//       AnimationVars* pVars;
//       pVars = &_vars[i];
//       pVars->coordinates.x = random ( 0, WS2812_Settings.Total_X );
//       pVars->coordinates.y = random ( 0, return_total_y ( WS2812_Settings.Total_X ) ) ;

//       if (WS2812_Settings.Palette_Choice == WHEEL) pVars->position = random(255);

//       AniObjectCallback ObjectUpdate = [pVars, overlap]()
//                                        //        ObjectCallback ObjectUpdate = [pVars]() //  lamda func passes READ only pointer to the stuct containing the animation vars.. these can be written to in animation...
//       {

//         int16_t pixel;
//         bool OK = false;
//         uint8_t counter = 0;
//         do {
//           counter++;
//           XY returned_XY = return_adjacent(pVars->coordinates);
//           pixel = return_pixel(returned_XY.x, returned_XY.y, WS2812_Settings.Total_X);

//           // true checking
//           if (pixel > -1 &&  !animator->IsAnimating(pixel) ) {
//             pVars->coordinates = returned_XY;
//             OK = true;
//           }

//           // // skip animating effects.
//           // if (pixel > -1 &&  !animator->IsAnimating(pixel) && WS2812_Settings.Effect_Option && counter > 2 ) {
//           //     pVars->coordinates = returned_XY;
//           //     //OK = true;
//           //     counter = 0;
//           //     }

//           // allows overlap bailout, but only after trying not to.
//           if (pixel > -1 && counter > 9 && overlap) {
//             pVars->coordinates = returned_XY;
//             OK = true;
//           }

//         } while (!OK && counter < 10) ;

//         RgbColor Fixed_Colour = pVars->colour;

//         if (OK) {

//           RgbColor temptestOLD = strip->GetPixelColor(pixel);
//           AnimUpdateCallback animUpdate = [pVars, pixel, temptestOLD, Fixed_Colour](float progress) {
//             RgbColor updatedColor, NewColour;
//             (WS2812_Settings.Effect_Option == 0) ? NewColour = pVars->colour : NewColour = Fixed_Colour;
//             if (progress < 0.5) updatedColor = HsbColor::LinearBlend(temptestOLD, NewColour,  progress * 2.0f);
//             if (progress > 0.5) updatedColor = HsbColor::LinearBlend(NewColour, RgbColor(0, 0, 0) , (progress * 2.0f) - 1.0f );
//             strip->SetPixelColor(pixel, updatedColor);
//           };

//           animator->StartAnimation(pixel, map( WS2812_Settings.Effect_Max_Size, 0, 255, WS2812_Settings.Timer * 2 , 20000 ) , animUpdate);
//         };

//       };

//       //animatedobject->Add(ObjectUpdate);
//       pVars->ObjUpdate = ObjectUpdate;
//     }; // end of multiple effect count generations...


//   }
//   break;
//   case REFRESH: {
//     Effect_Refresh_position = true;
//     Effect_Refresh_colour = true;
//   }
//   break;

//   case RUN: {
//     AnimationVars* pVars;
//     if (animationCount != WS2812_Settings.Effect_Count) Snakes.Start(); // Restart only if animation number changed
//     const uint32_t new_colour_time = map (WS2812_Settings.Timer, 0, 255, 20000, 300000) ;


//     if (!triggered || Effect_Refresh_colour || random_colour_timer(new_colour_time)) {

//       if (WS2812_Settings.Palette_Choice == WHEEL)  {

//         for (uint8_t i = 0; i < animationCount; i++) {
//           pVars = &_vars[i];
//           pVars->colour = dim(Wheel(pVars->position++ % 255) );
//           pVars->effectchanged = false;
//         }

//       }  else  {

//         for (uint8_t i = 0; i < animationCount; i++) {
//           pVars = &_vars[i];
//           pVars->oldcolour = pVars->colour;
//           pVars->newcolour = dim(Return_Palette(WS2812_Settings.Color, i) );
//           pVars->effectchanged = true;
//         }

//         effect_timer = millis() ;
//         triggered = true;

//       }
//       Effect_Refresh_colour = false;
//     }


// //  Update the blending colours for each effect outside of the  other loops

//     for (uint8_t i = 0; i < animationCount; i++) {

//       pVars = &_vars[i];
//       if (pVars->effectchanged == true) {

//         const uint32_t transitiontime2 = 5000; // map (WS2812_Settings.Timer, 0, 255, 20000, 300000) ;
//         const uint32_t _time = (millis() - effect_timer);
//         float _delta = float (_time) /  float(transitiontime2)  ; // WS2812_Settings.Timer * 10 ) ;

//         if (_delta < 1.0) {
//           pVars->colour = HslColor::LinearBlend(  pVars->oldcolour , pVars->newcolour, _delta);
//         } else {
//           pVars->colour = pVars->newcolour;
//         }

//         if (_delta > 1) {
//           pVars->effectchanged = false;
//         }

//       }

//     }

// // push effects out...

//     if (  millis() - lasteffectupdate >  WS2812_Settings.Timer || Effect_Refresh_position)  {
//       // update POSITION...
//       for (uint8_t i = 0; i < animationCount; i++) {
//         pVars = &_vars[i];
//         pVars->ObjUpdate();   //
//       }

//       lasteffectupdate = millis();
//       Effect_Refresh_position = false;
//     }
//   }
//   break;
//   case END: {
//     if (_vars)

//     {
//       delete[] _vars;
//       _vars = NULL;
//     }

//     Debugln("End of Effect");
//   }
//   break;

//   }

// }



/*-----------------------------------------------
*
*                      Snakes  NEW...
*
*------------------------------------------------*/


// void SnakesFn(effectState state)
// {

//   switch (state) {

//   case PRE_EFFECT: {
//     Serial.println("PRE_EFFECT - SNAKES");
//     lights.autoWait();



//   }

//   break;
//   case RUN_EFFECT: {
//     static uint32_t tick = 0;
//     if (millis() - tick > 5000) {
//       Serial.println("RUN");
//       tick = millis();
//     }

//   }
//   break;

//   case POST_EFFECT: {
//     Serial.println("POST_EFFECT - SNAKES");
//     lights.setWaiting();

//   }
//   break;
//   }
// }


/*-----------------------------------------------
*
*                      Bobbly Squares  NEW...
*
*------------------------------------------------*/



// struct EFFECT_s {

//   struct position_s {
//     uint16_t x = 0;
//     uint16_t y = 0;
//   } ;
//   position_s * pPosition;

//   EFFECT_s(uint8_t _count, uint8_t LEDs)//, uint8_t colorscount = 0)
//   {
//     count = _count;
//     manager = new EffectGroup; // create effect group
//     pPosition = new position_s[_count];
//     matrix = lights.matrix();
//     pGroup = new EffectObjectHandler* [count];

//     for (uint8_t i = 0; i < count; i++) {
//       pGroup[i] =  manager->Add(i, lights.speed() , new EffectObject( LEDs ) );
//       if (!pGroup[i]) { Serial.println("[EFFECT_s] nullptr returned"); }
//     }


//   }
//   ~EFFECT_s()
//   {
//     delete manager;
//     //delete[] colors;
//     delete[] pGroup;
//     delete[] pPosition;
//   }
//   void Run()
//   {
//     if (manager) { manager->Update(); }
//   }
//   EffectGroup* manager;
//   //RgbColor * colors;
//   EffectObjectHandler ** pGroup;
//   Melvtrix * matrix;
//   uint8_t count = 0;

//   position_s & position(uint16_t i) { return pPosition[i];}


// };
// //  Generates random squares, no fill...
// //  Does
// void BobblySquaresFn_create(struct EFFECT_s *&, bool, bool);

// void BobblySquaresFn(effectState & state)
// {
//   static EFFECT_s* EFFECT = nullptr; // dont forget to initialise pointers... ARGHHHHHHHH


//   switch (state) {

//   case PRE_EFFECT: {
//     Serial.printf("[BobblySquaresFn] Creating Objects (%u)\n", ESP.getFreeHeap());
//     lights.SetTimeout( 0);
//     lights.palette().mode(WHEEL);
//     lights.palette().total(255) ;

//     if (EFFECT) {
//       delete EFFECT;
//       EFFECT = nullptr;

//     }

//     EFFECT = new EFFECT_s(5, 25);

//     if (EFFECT) {
//       BobblyShapeFn_create(EFFECT, true, true, random(0, 3));
//     }
//   }

//   break;
//   case RUN_EFFECT: {

//     if (EFFECT) { EFFECT->Run(); }

//   }
//   break;

//   case POST_EFFECT: {
//     Serial.println("[BobblySquaresFn] End");
//     if (EFFECT) {
//       delete EFFECT;
//       EFFECT = nullptr;
//     }
//   }
//   break;
//   case EFFECT_REFRESH: {
//     Serial.println("[BobblySquaresFn] Refresh");
//     state = PRE_EFFECT;
//   }
//   break;

//   }

// }

// void BobblyShapeFn_create(struct EFFECT_s *& EFFECT, bool random1, bool random2, uint8_t shape)
// {


//   for (uint8_t obj = 0; obj < EFFECT->count; obj++) {

//     EffectObjectHandler * current =  EFFECT->pGroup[obj];  //    pointer to current group of pixels...

//     // nullptr protection
//     if (!current) break;

//     current->SetObjectUpdateCallback( [ current, EFFECT, obj, random1, random2, shape ]() {

//       current->reset(); // new set of pixels...

//       EFFECT->matrix->setShapeFn( [ EFFECT, obj, current, random1 ] (uint16_t pixel, int16_t x, int16_t y) {
//         current->Addpixel(pixel);
//       });

//       uint8_t size = random(2, 6);
//       uint16_t x = EFFECT->position(obj).x = random(0, lights.matrix()->width() - size + 1);
//       uint16_t y = EFFECT->position(obj).y = random(0, lights.matrix()->height() - size + 1);

//       uint16_t add_factor = (random1) ? random(5, 10) : 10;
//       current->Timeout( lights.speed() * add_factor); // update speed of current effect!

//       switch (shape) {
//       case 0:
//         EFFECT->matrix->drawRect(x, y,  size, size, 0); //  fills shape with
//         break;
//       case 1:
//         EFFECT->matrix->drawCircle(x, y, size, 0); //  fills shape with
//         break;
//       case 2:
//         EFFECT->matrix->drawTriangle(x, y, x + size, y, x + (size / 2), y + size, 0); //  fills shape with
//         break;
//       case 3:
//         EFFECT->matrix->fillTriangle(x, y, x + size, y, x + (size / 2), y + size , 0); //  fills shape with
//         break;
//       }

//       //EFFECT->matrix->drawRect(x, y,  size, size, 0); //  fills shape with

//     });

//     current->SetPixelUpdateCallback( [random2] (uint16_t n, uint16_t pixel) {
//       uint16_t add_factor = (random2) ? random(5, 10) : 10;
//       FadeToAndBack(pixel, lights.nextcolor(), lights.speed() * random(5, 10) );
//       //FadeToAndBack(pixel, RgbColor(5,0,0), lights.speed() * random(5, 10) );

//     });
//   }

// }


