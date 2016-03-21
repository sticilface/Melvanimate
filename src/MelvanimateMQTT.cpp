


#include "MelvanimateMQTT.h"

void MelvanimateMQTT::loop()
{
	if (_client.connected()) {
		_client.loop();
		_sendFullJson(); // done async to take it out of handler.
	} else {
		_reconnect();
	}

}

void MelvanimateMQTT::sendPresets()
{

	DynamicJsonBuffer jsonBufferReply;
	JsonObject & reply = jsonBufferReply.createObject();
	if (_melvanimate) {
		_melvanimate->addAllpresets(reply);
		size_t length = reply.measureLength();
		char * data = new char[length + 2];
		if (data) {
			memset(data, '\0', length + 2);
			reply.printTo(data, length + 1);
			publish( "presets", data, length + 1 );
			delete[] data;
		}

	}
}

void MelvanimateMQTT::_sendFullJson()
{
	if ( _send_flag &&  millis()  - _send_flag > 500 ) {
		DynamicJsonBuffer jsonBufferReply;
		JsonObject & reply = jsonBufferReply.createObject();
		if (_melvanimate) {

			_melvanimate->populateJson(reply);
			size_t length = reply.measureLength();
			char * data = new char[length + 2];
			if (data) {

				memset(data, '\0', length + 2);
				reply.printTo(data, length + 1);
				publish( "json", data, length + 1 );
				delete[] data;
			}
		}
		_send_flag = 0; 
	}
}


void MelvanimateMQTT::_handle(char* topic, byte* payload, unsigned int length)
{

	bool sendresponse = false;

	{
		DynamicJsonBuffer jsonBuffer;

		String Stopic = String(topic);
		char * data = new char[length + 1];
		memset(data, '\0', length + 1);
		memcpy( data, (char*)payload, length);
		//String message = String(data);

		DebugMelvanimateMQTTf("[MelvanimateMQTT::_handle] DEBUG: %s : %s\n", topic, data );


		if (Stopic.endsWith("/json/set")) {

			JsonObject & root = jsonBuffer.parseObject( (char*)payload, length);
			_melvanimate->parse(root);
			sendresponse = true;


			sendFullJson();


		} else if (Stopic.endsWith("/set")) {

			//  not found.. so send to melvanimate...
			JsonObject & root = jsonBuffer.createObject();

			String shorttopic = String(topic).substring( String(_melvanimate->deviceName()).length() + 1 , strlen(topic) - 4 );
			root[shorttopic.c_str()] = data;
			_melvanimate->parse(root);
			DynamicJsonBuffer jsonBufferReply;
			JsonObject & reply = jsonBufferReply.createObject();

			_melvanimate->populateJson(reply);

			if (reply.containsKey("settings")) {

				JsonObject & settings = reply["settings"];

				String shorttopic = String(topic).substring( String(_melvanimate->deviceName()).length() + 1 , strlen(topic) - 4 );


				for (JsonObject::iterator it = settings.begin(); it != settings.end(); ++it) {

					if (shorttopic == String(it->key)) {

						if (it->value.is<const char *>()) {
							publish( it->key, it->value.asString() );

						} else {

							size_t length = it->value.measureLength();
							char * data2 = new char[length + 2];

							if (data2) {
								memset(data, '\0', length + 2);
								it->value.printTo(data2, length + 1);
								publish( it->key, data2, length + 1 );
								delete[] data2;
							}
						}
					}
				}
			}
		}

		if (data) {
			delete[] data;
			data = nullptr;
		}

	}

}

bool MelvanimateMQTT::publish(const char * topic, const char * payload)
{

	return 	_client.publish( (String(_melvanimate->deviceName()) + "/" + topic).c_str() , payload );
}
bool MelvanimateMQTT::publish(const char * topic, const char * payload, size_t length)
{

	return 	_client.publish( (String(_melvanimate->deviceName()) + "/" + String(topic)).c_str() , payload, length );
}


void MelvanimateMQTT::_reconnect()
{

	if (!_client.connected()) {

		if (millis() - _reconnectTimer > 5000) {
			DebugMelvanimateMQTTf("[MelvanimateMQTT::_reconnect] MQTT Connect Attempted...");

//    boolean connect(const char* id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);

			if (_client.connect(_melvanimate->deviceName(), (String(_melvanimate->deviceName()) + "/status").c_str(), 1, false, "offline"  )    ) {
				DebugMelvanimateMQTTf("connected\n");
				// Once connected, publish an announcement...
				if (_melvanimate->deviceName()) {

					_client.publish(  ( "esp/" + String(_melvanimate->deviceName() ) ).c_str() , WiFi.localIP().toString().c_str() , true);

					publish( "status", "online", true);
					publish( "IP" , WiFi.localIP().toString().c_str() , true);
					// ... and resubscribe
					_client.subscribe( ( String(_melvanimate->deviceName()) + "/+/set").c_str()) ;

					sendPresets();

				}
				_reconnectTimer = 0;
				return;
			}
			_reconnectTimer = millis();
		}
	}

}

bool MelvanimateMQTT::addJson(JsonObject & root)
{

	JsonObject & mqttjson = root.createNestedObject("MQTT"); 

	mqttjson["enable"] = true; 

	JsonArray & ip = mqttjson.createNestedArray("ip");
	ip.add(_addr[0]);
	ip.add(_addr[1]);
	ip.add(_addr[2]);
	ip.add(_addr[3]);

	mqttjson["port"] = _port; 



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
