


#include "MelvanimateMQTT.h"

void MelvanimateMQTT::loop()
{
	if (_client.connected()) {
		_client.loop();
		_sendASync(); //  handles actual send....
	} else {
		_reconnect();
	}

}

void MelvanimateMQTT::_sendASync()
{
	if (_firstmessage && millis() - _asyncTimeout > 1000) {
		mqtt_message * handle = _firstmessage;
		_firstmessage = handle->next();
		DebugMelvanimateMQTTf("[MelvanimateMQTT::_sendASync] Sending msg [%s:%s]\n", handle->topic(), handle->msg() ) ;
		handle->publish();
		delete handle;
		_asyncTimeout = millis();
	}
}

void MelvanimateMQTT::sendPresets()
{

	DynamicJsonBuffer jsonBufferReply;
	JsonObject & reply = jsonBufferReply.createObject();
	if (_melvanimate) {
		_melvanimate->addAllpresets(reply);

		if (reply.containsKey("Presets")) {
			JsonArray & presets = reply["Presets"];

			size_t length = presets.measureLength();
			char * data = new char[length + 2];
			if (data) {
				memset(data, '\0', length + 2);
				presets.printTo(data, length + 1);
				publish( "presets", data, length + 1 );
				delete[] data;
			}
		}
	}
}

void MelvanimateMQTT::sendJson(bool onlychanged)
{

	DynamicJsonBuffer jsonBufferReply;
	JsonObject & reply = jsonBufferReply.createObject();
	if (_melvanimate) {

		_melvanimate->populateJson(reply, onlychanged ); //  this fills the json with only changed data

		if (reply.containsKey("settings")) {

			JsonObject & settings = reply["settings"];

#ifdef DebugMelvanimateMQTT
			// Serial.println("Settings: ");
			// settings.prettyPrintTo(Serial);
			// Serial.println();
#endif

			for (JsonObject::iterator it = settings.begin(); it != settings.end(); ++it) {

				if (it->value.is<const char *>()) {
					publish( it->key, it->value.asString() );

				} else {

					size_t length = it->value.measureLength();
					char * data2 = new char[length + 2];

					if (data2) {
						memset(data2, '\0', length + 2);
						it->value.printTo(data2, length + 1);
						publish( it->key, data2, length + 1 );
						delete[] data2;
					}
				}
			}
		}
	}
//		_send_changed_flag = 0;
//	}
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

			sendJson(true);

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
								memset(data2, '\0', length + 2);
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

bool MelvanimateMQTT::publish(const char * topic, const char * payload, bool retained)
{
	publish (topic, (uint8_t*)payload, strlen(payload), retained);
}

bool MelvanimateMQTT::publish(const char * topic, uint8_t * payload, size_t length, bool retained)
{

	uint8_t count = 1;
	if (!_firstmessage) {

		//DebugMelvanimateMQTTf("[MelvanimateMQTT::publish] _firsthandle assigned.\n");
		_firstmessage = new mqtt_message(this, topic, payload, length, retained);
		if (_firstmessage) {
			DebugMelvanimateMQTTf("[MelvanimateMQTT::publish] assigning %u\n", count);
			return true;
		}
	} else {
		//DebugMelvanimateMQTTf("[MelvanimateMQTT::publish] _firsthandle already assigned.\n");
		count++;
		mqtt_message * handle = _firstmessage;
		mqtt_message * current = _firstmessage;
		while (current->next()) {
			count++;
			current = current->next();
		}

		current->next( new mqtt_message(this, topic, payload, length, retained));

	}

	DebugMelvanimateMQTTf("[MelvanimateMQTT::publish] assigning %u\n", count);

}


void MelvanimateMQTT::_reconnect()
{

	if (!_client.connected()) {

		if (millis() - _reconnectTimer > 5000) {
			DebugMelvanimateMQTTf("[MelvanimateMQTT::_reconnect] [%s] MQTT Connect Attempted...", _melvanimate->deviceName() );

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
			DebugMelvanimateMQTTf( "Failed\n");
			_reconnectTimer = millis();
		}
	}

}

bool MelvanimateMQTT::addJson(JsonObject & root)
{
	DebugMelvanimateMQTTf("[MelvanimateMQTT::addJson] called\n" );

	JsonObject & mqttjson = root.createNestedObject("MQTT");

	mqttjson["enabled"] = true;

	JsonArray & ip = mqttjson.createNestedArray("ip");
	ip.add(_addr[0]);
	ip.add(_addr[1]);
	ip.add(_addr[2]);
	ip.add(_addr[3]);

	mqttjson["port"] = _port;
}


bool MelvanimateMQTT::mqtt_message::publish()
{
	if (_host) {
		return 	_host->mqttclient()->publish( (String(_host->pmelvanimate()->deviceName()) + "/" + String(_topic)).c_str() , _msg, _plength, _retained);
	}
	return false;
}



