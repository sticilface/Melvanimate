void SimpleFn()
{
  static uint32_t tick = 0;
  if (millis() - tick < 20000) { return; }
  Serial.println("SimpleFn RUN");
  tick = millis();
}




void ComplexFn(char *i, char *j, double k)
{

}

// struct XY_t FookinComplex(bool a , bool b, String c)
// {

// }



void interesting()
{


  //EffectObject * blob = new EffectObject (2);

  // //EffectObject blobO  (3);
  // static EffectObject * blob;

  // Serial.println("interesting called");
  // static bool switched = false;

  // const uint16_t number = 3;

  // if (!switched) {
  //   if (blob) delete blob;
  //   blob = new EffectObject (number);

  //   uint16_t offset = random(5);
  //   for (uint16_t i = 0; i < number; i++) {

  //     blob->Addpixel(i, i + offset);
  //     blob->SetColor(i, RgbColor(255, 0, 0));
  //   }
  //   blob->StartBasicAnimations(500);

  // }
  // else
  // {
  //   for (uint16_t i = 0; i < number; i++) {
  //     blob->SetColor(i, RgbColor(0, 0, 0));
  //   }
  //   blob->StartBasicAnimations(100);
  //   delete blob;
  //   blob = nullptr;
  // }

  // switched = !switched;


}


// void interesting2()
// {

//   EffectObject blob0(2);
//   EffectObject * blob = &blob0;

//   //if (!blob) blob = new EffectObject(2);

//   blob->SetObjectUpdateCallback( [&blob]() {

//     for (uint8_t i = 0; i < 2; i++) {
//       uint16_t pix = random(7);
//       blob->Addpixel(i, pix);
//     }
//   });

//   blob->SetPixelUpdateCallback(  [] (uint16_t position, uint16_t pixel) { // called per pixel of the effect, sent to neopixelbus

//     RgbColor originalcolor = strip->GetPixelColor(pixel);

//     AnimUpdateCallback animUpdate = [pixel, originalcolor] (float progress) {
//       RgbColor updatedColor;

//       if (progress < 0.5) {
//         updatedColor = RgbColor::LinearBlend(originalcolor, RgbColor(255, 0, 0), progress * 2 );
//       } else {
//         updatedColor = RgbColor::LinearBlend(RgbColor(255, 0, 0), 0, (progress - 0.5) * 2 );
//       }
//       strip->SetPixelColor(pixel, updatedColor);
//     };

//     StartAnimation(pixel, 1000, animUpdate);

//   });






//   blob->UpdateObject();
//   blob->StartAnimations();
//   //delete blob;
//   //blob = nullptr;

// }


// void ObjectFn(effectState state)
// {
//   static uint32_t tick = 0;
//   static EffectObject * blob;
//   static Palette* palette;

//   tick = millis();

//   switch (state) {

//   case PRE_EFFECT: {
//     const uint8_t points = 5;
//     const uint32_t timeout = 5000;
//     Serial.println("PRE: Blob");
//     if (blob) { delete blob; }
//     blob = nullptr;
//     blob = new EffectObject(points);

//     if (palette) { delete palette; }
//     palette = new Palette;
//     palette->mode(WHEEL);

// //    lights.SetTimeout(timeout);

//     blob->SetObjectUpdateCallback( []() {
//       uint16_t pixarray[points];
//       for (uint8_t i = 0; i < points; i++) {
//         bool found = true;
//         uint8_t pix;
//         do {
//           pix = random(7);
//           bool therealready = false;
//           for (uint8_t j = 0; j < points; j++) {
//             if (pix == pixarray[j]) {
//               found = false;
//               break;
//             }
//             found = true;
//           }

//         } while (found == false);
//         pixarray[i] = pix;
//       }
//       for (uint8_t i = 0; i < points; i++) {
//         blob->Addpixel(i, pixarray[i]);
//       }
//     });

//     blob->SetPixelUpdateCallback(  [] (uint16_t position, uint16_t pixel) { // called per pixel of the effect, sent to neopixelbus
//       RgbColor originalcolor = strip->GetPixelColor(pixel);
//       AnimUpdateCallback animUpdate = [pixel, originalcolor] (float progress) {
//         RgbColor nextcolour = palette->next();

//         RgbColor updatedColor;
//         if (progress < 0.5) {
//           updatedColor = RgbColor::LinearBlend(originalcolor, nextcolour, progress * 2 );
//         } else {
//           updatedColor = RgbColor::LinearBlend(nextcolour, 0, (progress - 0.5) * 2 );
//         }
//         strip->SetPixelColor(pixel, updatedColor);
//       };
//       animator->StartAnimation(pixel, timeout, animUpdate);
//     });


//   }

//   break;
//   case RUN_EFFECT: {

//     blob->UpdateObject();
//     blob->StartAnimations();

//   }
//   break;

//   case POST_EFFECT: {
//     Serial.println("End: Blob");
//     if (blob) { delete blob; }
//     blob = nullptr;
//     if (palette) { delete palette; }
//     palette = nullptr;
//   }
//   break;
//   }

// }