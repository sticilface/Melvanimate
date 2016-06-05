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



// GRB LEDs , UART method
//typedef NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> MyPixelBus;

//GRB DMA Method
//typedef NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> MyPixelBus;



//typedef NeoPixelBus< NeoBrgFeature, NeoEsp8266Uart800KbpsMethod> MyPixelBus;





typedef NeoPixelBus<NeoGrbwFeature, Neo800KbpsMethod> MyPixelBus;


inline RgbColor myPixelColor(const RgbColor& color) {
	return color; 
}

inline RgbColor myPixelColor(const RgbwColor& color) {
	return RgbColor(color.R, color.G, color.B); 
}
