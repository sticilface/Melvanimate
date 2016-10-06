#pragma once

#include <NeoPixelBus.h>

//
/* Features

	NeoGrbFeature
	NeoRgbFeature
	NeoBrgFeature


*/

//#define WS2812_UART_METHOD

/*****************************************************************


******************************************************************/


#ifdef WS2812_FOUR_COLOR
		typedef NeoGrbwFeature MyColorFeature; //  FOUR COLOUR LEDS
#else
		typedef NeoGrbFeature MyColorFeature; //  THREE COLOUR LEDS
#endif


/*****************************************************************


******************************************************************/

#if defined(WS2812_UART_METHOD)
		typedef NeoEsp8266Uart800KbpsMethod MyWS2812OutputMethod; //  UART  method GPIO 2
		#pragma message("WS2812_UART_METHOD")
#elif defined(WS2812_UART_ASYNC_METHOD)
		typedef NeoEsp8266AsyncUart800KbpsMethod MyWS2812OutputMethod;
		#pragma message("WS2812_UART_ASYNC_METHOD")
#else
		typedef Neo800KbpsMethod MyWS2812OutputMethod; //  Standard DMA method GPIO 3 / RX
#endif


//GRB DMA Method
typedef NeoPixelBus<MyColorFeature, MyWS2812OutputMethod> MyPixelBus;


inline RgbColor myPixelColor(const RgbColor& color) {
	return color;
}

inline RgbColor myPixelColor(const RgbwColor& color) {
	return RgbColor(color.R, color.G, color.B);
}
