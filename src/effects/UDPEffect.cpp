#include "UDPEffect.h"

bool UDPEffect::Run()
{
	//   int packetSize;

//   switch (state) {

//   case PRE_EFFECT: {

//     lights.SetTimeout(0);

// //    if (millis() > 60000) Adalight_Flash();
//     Udp.beginMulticast(WiFi.localIP(), multicast_ip_addr, UDPlightPort);

//     break;
//   }
//   case RUN_EFFECT: {

//     if (!Udp) Udp.beginMulticast(WiFi.localIP(), multicast_ip_addr, UDPlightPort); // restart listening if it stops...

//     packetSize = Udp.parsePacket();

//     if  (Udp.available())  {
//       for (int i = 0; i < packetSize; i = i + 3) {
//         if (i > strip->PixelCount() * 3) break;         // Stops reading if LED count is reached.
//         stripBuffer[i + 1] = Udp.read();   // direct buffer is GRB,
//         stripBuffer[i]     = Udp.read();
//         stripBuffer[i + 2] = Udp.read();
//       }
//       Udp.flush();
//       strip->Dirty();
//       //strip->Show();

//       Show_pixels(true);
//       lights.timeoutvar = millis();

//     }

//     if (millis() - lights.timeoutvar > 5000)  {
//       strip->ClearTo(0, 0, 0);
//       lights.timeoutvar = millis();
//     }

//     break;
//   }
//   case POST_EFFECT: {
//     Udp.stop();
//     animator->FadeTo(250, 0);

//     break;
//   }

//   }
}

bool UDPEffect::Start() {

}


bool UDPEffect::Stop()
{
	if (_udp) {
		delete _udp;
		_udp = nullptr; 
	}

	if (_vars) {
		delete _vars;
		_vars = nullptr; 
	}
}