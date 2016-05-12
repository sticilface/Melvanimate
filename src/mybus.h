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
typedef NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> MyPixelBus;

//GRB DMA Method
//typedef NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> MyPixelBus;



//typedef NeoPixelBus< NeoBrgFeature, NeoEsp8266Uart800KbpsMethod> MyPixelBus;
