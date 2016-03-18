


#include "MelvanimateMQTT.h"
#include <ArduinoJson.h>

void MelvanimateMQTT::loop()
{
	if (_client.connected()) {
		_client.loop();
	} else {
		_reconnect();
	}

}



void MelvanimateMQTT::_handle(char* topic, byte* payload, unsigned int length)
{


	if ( _melvanimate->deviceName() && !strcmp(topic, _melvanimate->deviceName() ) ) {
		DebugMelvanimateMQTTf("JSON topic recieved for %s\n", _melvanimate->deviceName() );
		DynamicJsonBuffer jsonBuffer;
		JsonObject & root = jsonBuffer.parseObject( (char*)payload, length);
		root.prettyPrintTo(Serial);
		DebugMelvanimateMQTTf("\n"); 
	}




	/*

	 	ON / OFF
		load save preset


	*/



//  this has to go last for the JSON to be passed to the current effect
// 	if (Current()) {
// 		if (Current()->parseJson(root)) {
// //      Serial.println("[handle] JSON Setting applied");
// 			code = 1;
// 		}
// 	}





}



void MelvanimateMQTT::_reconnect()
{

	if (!_client.connected()) {
		if (millis() - _reconnectTimer > 5000) {
		DebugMelvanimateMQTTf("[MelvanimateMQTT::_reconnect] ReConnect Attempted\n");

			if (_client.connect("ESP8266Client")) {
				DebugMelvanimateMQTTf("connected\n");
				// Once connected, publish an announcement...
				if (_melvanimate->deviceName()) {
					_client.publish(_melvanimate->deviceName(), "Ready");


					// ... and resubscribe
					_client.subscribe(_melvanimate->deviceName());
					_client.publish( (String(_melvanimate->deviceName()) + "/" + "modes").c_str(), "modes here" );

				}
				_reconnectTimer = 0; 
				return;
			}

			_reconnectTimer = millis();
		}
	}


//   // Loop until we're reconnected
//   while (!client.connected()) {
//     DebugMelvanimateMQTTf("Attempting MQTT connection...\n");
//     // Attempt to connect
//     if (client.connect("ESP8266Client")) {
//       DebugMelvanimateMQTTf("connected\n");
//       // Once connected, publish an announcement...
//       client.publish("outTopic", "hello world");
//       // ... and resubscribe
//       client.subscribe("inTopic");
//     } else {
//       DebugMelvanimateMQTTf("failed, rc=");
//       DebugMelvanimateMQTTf("%s", String(client.state()).c_str() );
//       DebugMelvanimateMQTTf(" try again in 5 seconds\n");
//       // Wait 5 seconds before retrying
//       delay(5000);
//     }
//   }
}