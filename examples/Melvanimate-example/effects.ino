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
*                 Timing Test
*
*------------------------------------------------*/

void TimingFn(effectState& state, EffectHandler* ptr)
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


void DummyFn(effectState& state, EffectHandler* ptr)
{
  static uint32_t tick = 0;
  if (millis() - tick < 1000) return;
  tick = millis();
  //Serial.println("Dummy Run");

}

void CascadeEffectFn(effectState& state, EffectHandler* ptr)
{
  static uint32_t tick = 0;
  if (millis() - tick < 1000) return;
  tick = millis();
  //Serial.println("Dummy Run");

}
/*-----------------------------------------------
*
*                 AdaLight
*
*------------------------------------------------*/
// forward declaration
/*

void Adalight_function();
void  Adalight_Flash();


void AdaLightFn(effectState& state, EffectHandler* ptr)
{

  AdalightEffect* effect = static_cast<AdalightEffect*>(ptr);

  if (effect) {
    switch (state) {

    case PRE_EFFECT: {

      if (Serial) {
        Serial.flush();
        delay(500);
 //       Serial.end();
      }
      uint32_t speed = 115200;

      if (effect->getSerialspeed(speed) )

        Serial.begin(speed);

      Serial.println("Init: Adalight");
      lights.SetTimeout(0);

      if (millis() > 30000) Adalight_Flash();
    }

    break;
    case RUN_EFFECT: {
      Adalight_function();
    }
    break;

    case POST_EFFECT: {
      Serial.println("End: Adalight");
      animator->FadeTo(250, 0);
    }
    break;
    case EFFECT_REFRESH: {
      state = PRE_EFFECT;
    }
    }
  }
}



void  Adalight_Flash()
{


  for (int pixel = 0; pixel < strip->PixelCount(); pixel++) {

    RgbColor originalcolor = strip->GetPixelColor(pixel);

    AnimUpdateCallback animUpdate = [pixel, originalcolor] (float progress) {
      RgbColor updatedColor;

      if (progress < 0.25) {
        updatedColor = RgbColor::LinearBlend(originalcolor, RgbColor(100, 0, 0), progress * 4 );
      } else if (progress < 0.5) {
        updatedColor = RgbColor::LinearBlend(RgbColor(100, 0, 0), RgbColor(0, 100, 0) , (progress - 0.25) * 4 );
      } else if (progress < 0.75) {
        updatedColor = RgbColor::LinearBlend(RgbColor(0, 100, 0), RgbColor(0, 0, 100), (progress - 0.5) * 4 );
      } else {
        updatedColor = RgbColor::LinearBlend(RgbColor(0, 0, 100), RgbColor(0, 0, 0), (progress - 0.75) * 4 );
      }
      strip->SetPixelColor(pixel, updatedColor);
    };

    StartAnimation(pixel, 2000, animUpdate);

  }

}

void Adalight_function ()      //  uint8_t prefix[] = {'A', 'd', 'a'}, hi, lo, chk, i;
{

  uint8_t prefix[] = {'A', 'd', 'a'}, hi, lo, chk, i;
  static boolean Adalight_configured;
  static uint16_t effectbuf_position = 0;
  enum mode { MODE_INITIALISE = 0, MODE_HEADER, MODE_CHECKSUM, MODE_DATA, MODE_SHOW, MODE_FINISH};
  static mode state = MODE_HEADER;
  static int effect_timeout = 0;
  static uint8_t prefixcount = 0;
  static unsigned long ada_sent = 0;
  static unsigned long pixellatchtime = 0;
  const unsigned long serialTimeout = 15000; // turns LEDs of if nothing recieved for 15 seconds..
  static bool SendFailhere = false;

  switch (state) {

  case MODE_HEADER:

    effectbuf_position = 0; // reset the buffer position for DATA collection...

    if (Serial.available()) { // if there is serial available... process it... could be 1  could be 100....

      for (int i = 0; i < Serial.available(); i++) {  // go through every character in serial buffer looking for prefix...

        if (Serial.read() == prefix[prefixcount]) { // if character is found... then look for next...
          prefixcount++;
        } else prefixcount = 0;  //  otherwise reset....  ////

        if (prefixcount == 3) {
          effect_timeout = millis(); // generates START TIME.....
          state = MODE_CHECKSUM;
          prefixcount = 0;
          break;
        } // end of if prefix == 3
      } // end of for loop going through serial....
    } else if (!Serial.available() && (ada_sent + 5000) < millis()) {
      Serial.print("Ada\n"); // Send "Magic Word" string to host
      ada_sent = millis();
    } // end of serial available....

    break;

  case MODE_CHECKSUM:

    if (Serial.available() >= 3) {
      hi  = Serial.read();
      lo  = Serial.read();
      chk = Serial.read();
      if (chk == (hi ^ lo ^ 0x55)) {
        state = MODE_DATA;
      } else {
        state = MODE_HEADER; // ELSE RESET.......
      }
    }

    if ((effect_timeout + 1000) < millis()) state = MODE_HEADER; // RESET IF BUFFER NOT FILLED WITHIN 1 SEC.

    break;

  case MODE_DATA:

    //  this bit is what might... be causing the flashing... as it extends past memory stuctures....
    while (Serial.available() && effectbuf_position < 3 * strip->PixelCount()) {  // was <=

      stripBuffer[effectbuf_position++] = Serial.read();
    }

    if (effectbuf_position >= 3 * strip->PixelCount()) { // goto show when buffer has recieved enough data...
      state = MODE_SHOW;
      break;
    }

    if ((effect_timeout + 1000) < millis()) state = MODE_HEADER; // RESET IF BUFFER NOT FILLED WITHIN 1 SEC.


    break;

  case MODE_SHOW:

  {
    strip->Dirty(); // MUST USE if you're using the direct buffer copy...
    pixellatchtime = millis();
    Show_pixels(true);
    state = MODE_HEADER;
  }
  break;

  }

}

*/

/*-----------------------------------------------
*
*                 UDP
*
*------------------------------------------------*/


void UDPFn(effectState state, EffectHandler* ptr)
{
  int packetSize;

  switch (state) {

  case PRE_EFFECT: {

    lights.SetTimeout(0);

//    if (millis() > 60000) Adalight_Flash();
    Udp.beginMulticast(WiFi.localIP(), multicast_ip_addr, UDPlightPort);

    break;
  }
  case RUN_EFFECT: {

    if (!Udp) Udp.beginMulticast(WiFi.localIP(), multicast_ip_addr, UDPlightPort); // restart listening if it stops...

    packetSize = Udp.parsePacket();

    if  (Udp.available())  {
      for (int i = 0; i < packetSize; i = i + 3) {
        if (i > strip->PixelCount() * 3) break;         // Stops reading if LED count is reached.
        stripBuffer[i + 1] = Udp.read();   // direct buffer is GRB,
        stripBuffer[i]     = Udp.read();
        stripBuffer[i + 2] = Udp.read();
      }
      Udp.flush();
      strip->Dirty();
      //strip->Show();

      Show_pixels(true);
      lights.timeoutvar = millis();

    }

    if (millis() - lights.timeoutvar > 5000)  {
      strip->ClearTo(0, 0, 0);
      lights.timeoutvar = millis();
    }

    break;
  }
  case POST_EFFECT: {
    Udp.stop();
    animator->FadeTo(250, 0);

    break;
  }

  }

}




/*-----------------------------------------------
*
*                 DMX
*
*------------------------------------------------*/


void  DMXfn (effectState state, EffectHandler* ptr)
{

  int packetSize;
//TODO: Dynamically allocate seqTracker to support more than 4 universes w/ PIXELS_MAX change
  static uint8_t         *seqTracker;    /* Current sequence numbers for each Universe */
  static uint8_t         ppu, uniTotal, universe, channel_start, uniLast;
  static uint16_t        count, bounds ;
  static uint32_t        *seqError;      /* Sequence error tracking for each universe */
  static uint32_t timeout_data = 0;

  ppu = 170;
  universe = 1;
  channel_start = 1;

  switch (state) {

  case PRE_EFFECT:

    lights.SetTimeout(0);
    e131 = new E131;
    Debugln("DMX Effect Started");
//    if (millis() > 30000) Adalight_Flash();

    count = strip->PixelCount() * 3;
    bounds = ppu * 3;
    if (count % bounds)
      uniLast = universe + count / bounds;
    else
      uniLast = universe + count / bounds - 1;

    uniTotal = (uniLast + 1) - universe;

    if (seqTracker) free(seqTracker);
    if ((seqTracker = (uint8_t *)malloc(uniTotal)))
      memset(seqTracker, 0x00, uniTotal);

    if (seqError) free(seqError);
    if ((seqError = (uint32_t *)malloc(uniTotal * 4)))
      memset(seqError, 0x00, uniTotal * 4);

    Debugf("Count = %u, bounds = %u, uniLast = %u, uniTotal = %u\n", count, bounds, uniLast, uniTotal);


    e131->begin( E131_MULTICAST , universe ) ; // E131_MULTICAST // universe is optional and only used for Multicast configuration.


    break;

  case RUN_EFFECT:

// if(e131.parsePacket()) {
//        if (e131.universe == WS2812_Settings.Effect_Option) {
//            for (uint16_t i = 0; i < pixelCount; i++) {
//                uint16_t j = i * 3 + (CHANNEL_START - 1);
//                strip->SetPixelColor(i, e131.data[j], e131.data[j+1], e131.data[j+2]);
//            }
//            strip->Show();
//            timeout = millis();
//        }
//    }


    if (e131->parsePacket()) {
      if ((e131->universe >= universe) && (universe <= uniLast)) {
        /* Universe offset and sequence tracking */
        uint8_t uniOffset = (e131->universe - universe);
        if (e131->packet->sequence_number != seqTracker[uniOffset]++) {
          seqError[uniOffset]++;
          seqTracker[uniOffset] = e131->packet->sequence_number + 1;
        }

        /* Find out starting pixel based off the Universe */
        uint16_t pixelStart = uniOffset * ppu;

        /* Calculate how many pixels we need from this buffer */
        uint16_t pixelStop = strip->PixelCount();
        if ((pixelStart + ppu) < pixelStop)
          //pixelStop = pixelStart + config.ppu;
          pixelStop = pixelStart + ppu;

        /* Offset the channel if required for the first universe */
        uint16_t offset = 0;
        if (e131->universe == universe)
          offset = channel_start - 1;

        /* Set the pixel data */
        uint16_t buffloc = 0;
        for (uint16_t i = pixelStart; i < pixelStop; i++) {
          uint16_t j = buffloc++ * 3 + offset;
          //pixels.setPixelColor(i, e131.data[j], e131.data[j+1], e131.data[j+2]);
          strip->SetPixelColor(i, e131->data[j], e131->data[j + 1], e131->data[j + 2]);
        }

        /* Refresh when last universe shows up  or within 10ms if missed */
        if ((e131->universe == uniLast) || (millis() - lights.timeoutvar > 10)) {
          //if (e131.universe == uniLast) {
          //if (millis() - lastPacket > 25) {
          lights.timeoutvar = millis();
          Show_pixels(true);

        }
      }
    }



//  Set to black if 30 seconds passed...
    if (millis() - lights.timeoutvar > 30000)  {
      strip->ClearTo(0, 0, 0);
      lights.timeoutvar = millis();
    }


//  Print out errors...
    if (millis() - timeout_data > 30000) {

      //   for (int i = 0; i < ((uniLast + 1) - universe); i++)
      //     seqErrors =+ seqError[i];

      uint32_t seqErrors = 0, packet_rate = 0;
      static uint32_t packets_last = 0;
      for (int i = 0; i < ((uniLast + 1) - universe); i++)
        seqErrors = + seqError[i];

      packet_rate = ( e131->stats.num_packets - packets_last ) / 30;
      Debugf("DMX: Total Packets = %u, Sequence errors = %u, Rate = %u /s \n", e131->stats.num_packets, seqErrors, packet_rate);
      timeout_data = millis();
      packets_last = e131->stats.num_packets;

    }

    break;

  case POST_EFFECT:

    FadeTo(250, 0);
    if (e131) {
      delete e131;
    }
    e131 = nullptr;

    if (seqTracker) free(seqTracker);
    seqTracker = nullptr;

    if (seqError) free(seqError);
    seqError = nullptr;
    break;

  }
}

/*-----------------------------------------------
*
*                 SimpleColorFn
*
*------------------------------------------------*/

void SimpleColorFn(effectState& state, EffectHandler* ptr)
{
  SimpleEffect* effect = static_cast<SimpleEffect*>(ptr);

  if (effect) {

    switch (state) {

    case PRE_EFFECT: {
      Serial.println("[SimpleColorFn] PRE_EFFECT");
      lights.SetTimeout(10000);
      lights.autoWait(); //  this causes the manager to wait before latching over to next effect, or state...
      
        FadeTo( lights.dim(effect->_color, effect->_brightness));
      
    }

    break;
    case RUN_EFFECT: {
      //FadeTo(2000, lights.Current()->getColor());
    }
    break;

    case POST_EFFECT: {
      Serial.println("[SimpleColorFn] POST_EFFECT");
      lights.autoWait();
      FadeTo(0);
    }
    break;

    case EFFECT_REFRESH: {
      state = PRE_EFFECT;
      Serial.println("[SimpleColorFn] Refresh Called");
      break;
    }
    }
  }
}

/*-----------------------------------------------
*
*                      offFn
*
*------------------------------------------------*/


void offFn(effectState &state, EffectHandler* ptr)
{


  switch (state) {

  case PRE_EFFECT: {
    Serial.println("[OffFn] PRE_EFFECT");
    lights.SetTimeout(10000);
    lights.autoWait();
    FadeTo(0);
  }

  break;
  case RUN_EFFECT: {
    strip->ClearTo(0);
  }
  break;

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


