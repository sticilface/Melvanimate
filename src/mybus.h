#pragma once

#include <NeoPixelBus.h>

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

#ifdef WS2812_FOUR_COLOR
		typedef NeoGrbwFeature MyColorFeature; //  FOUR COLOUR LEDS
#else
		typedef NeoGrbFeature MyColorFeature; //  THREE COLOUR LEDS
#endif

#ifdef WS2812_UART_METHOD
		typedef NeoEsp8266Uart800KbpsMethod MyWS2812OutputMethod; //  UART  method GPIO 2
#else
		typedef Neo800KbpsMethod MyWS2812OutputMethod; //  Standard DMA method GPIO 3 / RX
#endif

//typedef Neo800KbpsMethod MyWS2812OutputMethod; //  Standard DMA method GPIO 3 / RX
//typedef NeoEsp8266Uart800KbpsMethod MyWS2812OutputMethod; //  UART  method GPIO 2



// GRB LEDs , UART method
//typedef NeoPixelBus<MyColorFeature, NeoEsp8266Uart800KbpsMethod> MyPixelBus;

//GRB DMA Method
typedef NeoPixelBus<MyColorFeature, MyWS2812OutputMethod> MyPixelBus;



//typedef NeoPixelBus< NeoBrgFeature, NeoEsp8266Uart800KbpsMethod> MyPixelBus;



//  4 colour

//typedef NeoPixelBus<NeoGrbwFeature, Neo800KbpsMethod> MyPixelBus;


inline RgbColor myPixelColor(const RgbColor& color) {
	return color;
}

inline RgbColor myPixelColor(const RgbwColor& color) {
	return RgbColor(color.R, color.G, color.B);
}
