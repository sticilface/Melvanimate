#pragma once

#include "NeoPixelBus.h"

//  
/* Features

	NeoGrbFeature
	NeoRgbFeature
	NeoBrgFeature


*/
	
/* Methods

	NeoEsp8266Uart800KbpsMethod
	
	


*/

//typedef NeoGrbwFeature MyPixelColorFeature;

typedef NeoGrbwFeature MyColorFeature; 

// GRB LEDs , UART method
typedef NeoPixelBus<MyColorFeature, NeoEsp8266Uart800KbpsMethod> MyPixelBus;

//GRB DMA Method
//typedef NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> MyPixelBus;



//typedef NeoPixelBus< NeoBrgFeature, NeoEsp8266Uart800KbpsMethod> MyPixelBus;



//  4 colour 

//typedef NeoPixelBus<NeoGrbwFeature, Neo800KbpsMethod> MyPixelBus;


inline RgbColor myPixelColor(const RgbColor& color) {
	return color; 
}

inline RgbColor myPixelColor(const RgbwColor& color) {
	return RgbColor(color.R, color.G, color.B); 
}
