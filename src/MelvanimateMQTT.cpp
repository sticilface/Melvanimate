


#include "MelvanimateMQTT.h"

MelvanimateMQTT::MelvanimateMQTT(Melvanimate * lights, IPAddress Addr, uint16_t Port) : _melvanimate(lights), _addr(Addr), _port(Port)
{
				using namespace std::placeholders;
				DebugMelvanimateMQTTf("[MelvanimateMQTT::MelvanimateMQTT] IP (%u.%u.%u.%u):%u\n", _addr[0], _addr[1], _addr[2], _addr[3], _port );

				 char * willtopic = nullptr;

        _mqttClient.onConnect( [this, willtopic](bool sessionPresent) {
								_disconnected = false;
                _mqttClient.publish( ( "esp/" + String(_melvanimate->deviceName() ) ).c_str(), 2, false, WiFi.localIP().toString().c_str() );
                publish( "status", "online", false);
								publish( _melvanimate->deviceName(), "name", false );
                publish( "IP", WiFi.localIP().toString().c_str(), false);
                _mqttClient.subscribe( ( String(_melvanimate->deviceName()) + "/+/set").c_str(), 2);
								DebugMelvanimateMQTTf("[MelvanimateMQTT::MelvanimateMQTT] MQTT Connected\n");

								if (willtopic) {
									free(willtopic);
								}

								if (_connectCB) {
									_connectCB();
								}


        });

        _mqttClient.onDisconnect( [this](AsyncMqttClientDisconnectReason reason) {
                DebugMelvanimateMQTTf("[MelvanimateMQTT::MelvanimateMQTT] MQTT Disconnected\n");
								_disconnected = true;
            //    _mqttClient.connect();

        });

        _mqttClient.onMessage(std::bind (&MelvanimateMQTT::_onMqttMessage, this, _1, _2, _3, _4, _5, _6)  );

				// _mqttClient.onPublish(onMqttPublish);

        _mqttClient.setServer(_addr, _port);
        //_mqttClient.setClientId(_melvanimate->deviceName()); //  disbale this so u end up with unique IDs... 
        _mqttClient.setKeepAlive(15);

				 willtopic = strdup ((String( _melvanimate->deviceName()) + "/status").c_str());

        _mqttClient.setWill( willtopic, 2, true, "Disconnected");

        //.setCredentials("username", "password")
        DebugMelvanimateMQTTf("[MelvanimateMQTT::MelvanimateMQTT] Connecting to MQTT...");



}


void MelvanimateMQTT::loop()
{
        // if (_client.connected()) {
        //  _client.loop();
        //  _sendASync(); //  handles actual send....
        // } else {
        //  _reconnect();
        // }

				if (_disconnected && millis() - _timeout > 30000) {
					DebugMelvanimateMQTTf("[MelvanimateMQTT::loop] Async - reconnecting to MQTT...\n");
					_mqttClient.connect();
					_timeout = millis();
				}

}

// void MelvanimateMQTT::_sendASync()
// {
// if (_firstmessage && millis() - _asyncTimeout > 1000) {
//  mqtt_message * handle = _firstmessage;
//  _firstmessage = handle->next();
//  DebugMelvanimateMQTTf("[MelvanimateMQTT::_sendASync] Sending msg [%s:%s]\n", handle->topic(), handle->msg() ) ;
//  handle->publish();
//  delete handle;
//  _asyncTimeout = millis();
// }
//}

void MelvanimateMQTT::sendPresets()
{

        DynamicJsonBuffer jsonBufferReply;
        JsonObject & reply = jsonBufferReply.createObject();
        if (_melvanimate) {
                _melvanimate->addAllpresets(reply);

                if (reply.containsKey("Presets")) {
                        JsonArray & presets = reply["Presets"];

                        for (JsonArray::iterator it = presets.begin(); it != presets.end(); ++it) {

                                JsonObject & current = *it;
                                uint8_t ID = current["ID"];
                                String topic = "preset/" + String(ID);
                                String msg = String(current["name"].asString()) + " [" + String(current["effect"].asString()) + "]";

                                publish(topic.c_str(), msg.c_str(), msg.length() );


                        }

                        // size_t length = presets.measureLength();
                        // char * data = new char[length + 2];
                        // if (data) {
                        //  memset(data, '\0', length + 2);
                        //  presets.printTo(data, length + 1);
                        //  publish( "presets", data, length + 1 );
                        //  delete[] data;
                        // }
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

                                        publish( it->key, it->value.asString(), false );

                                } else {

                                        size_t length = it->value.measureLength();
                                        char * data2 = new char[length + 2];

                                        if (data2) {
                                                memset(data2, '\0', length + 2);
                                                it->value.printTo(data2, length + 1);
                                                publish( it->key, (const char *)data2, length + 1, false );
                                                delete[] data2;
                                        }
                                }
                        }
                }
        }
//		_send_changed_flag = 0;
//	}
}


void MelvanimateMQTT::_onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
        _handle(topic, (byte*)payload, total);
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

												String topic_without_set = String(topic).substring( 0, strlen(topic) - 4 );
                        String shorttopic = topic_without_set.substring( topic_without_set.lastIndexOf("/")+1);

                        root[shorttopic.c_str()] = data;

												Serial.printf("command = %s, data = %s\n", shorttopic.c_str(), data);
                        _melvanimate->parse(root);
                        DynamicJsonBuffer jsonBufferReply;
                        JsonObject & reply = jsonBufferReply.createObject();

                        _melvanimate->populateJson(reply);

                        if (reply.containsKey("settings")) {

                                JsonObject & settings = reply["settings"];

                                String shorttopic = String(topic).substring( String(_melvanimate->deviceName()).length() + 1, strlen(topic) - 4 );


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
        publish (topic, payload, strlen(payload), retained);
}

bool MelvanimateMQTT::publish(const char * topic, const char * payload, size_t length, bool retained)
{


				String fulltopic = (String(_melvanimate->deviceName()) + "/" + String(topic)).c_str();
				_mqttClient.publish( fulltopic.c_str(), 2, retained,  payload, length);

        DebugMelvanimateMQTTf("[MelvanimateMQTT::publish] %s : %s \n", topic, payload);

        // if (topic && payload) {
        //
        //  uint8_t count = 1;
        //  if (!_firstmessage) {
        //
        //   //DebugMelvanimateMQTTf("[MelvanimateMQTT::publish] _firsthandle assigned.\n");
        //   _firstmessage = new mqtt_message(this, topic, payload, length, retained);
        //   if (_firstmessage) {
        //    DebugMelvanimateMQTTf("[MelvanimateMQTT::publish] assigning %u\n", count);
        //    return true;
        //   }
        //  } else {
        //   //DebugMelvanimateMQTTf("[MelvanimateMQTT::publish] _firsthandle already assigned.\n");
        //   count++;
        //   mqtt_message * handle = _firstmessage;
        //   mqtt_message * current = _firstmessage;
        //   while (current->next()) {
        //    count++;
        //    current = current->next();
        //   }
        //
        //   current->next( new mqtt_message(this, topic, payload, length, retained));
        //
        //  }
        //
        //  DebugMelvanimateMQTTf("[MelvanimateMQTT::publish] assigning %u\n", count);
        // }

}


// void MelvanimateMQTT::_reconnect()
// {

//  if (!_client.connected()) {
//
//   if (millis() - _reconnectTimer > ( 5000 * _reconnecttries)) {
//    DebugMelvanimateMQTTf("[MelvanimateMQTT::_reconnect] [%s] MQTT Connect Attempted...", _melvanimate->deviceName() );
//
// //    boolean connect(const char* id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
//
//    if (_client.connect(_melvanimate->deviceName(), (String(_melvanimate->deviceName()) + "/status").c_str(), 1, false, "offline"  )    ) {
//     DebugMelvanimateMQTTf("connected\n");
//     // Once connected, publish an announcement...
//     _reconnecttries = 0;
//     if (_melvanimate->deviceName()) {
//
//      _client.publish(  ( "esp/" + String(_melvanimate->deviceName() ) ).c_str() , WiFi.localIP().toString().c_str() , true);
//
//      publish( "status", "online", true);
//      publish( "IP" , WiFi.localIP().toString().c_str() , true);
//      // ... and resubscribe
//      _client.subscribe( ( String(_melvanimate->deviceName()) + "/+/set").c_str()) ;
//
//      sendPresets();
//
//     }
//     _reconnectTimer = 0;
//     return;
//    }
//    DebugMelvanimateMQTTf( "Failed\n");
//    _reconnectTimer = millis();
//    _reconnecttries++;
//    if (_reconnecttries > 300) {
//     _reconnecttries = 300;
//    }
//   }
//  }

//}

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


// bool MelvanimateMQTT::mqtt_message::publish()
// {
//  if (_host) {
//   DebugMelvanimateMQTTf("[MelvanimateMQTT::mqtt_message::publish] [%s] (%s) _retained = %s\n", String(_topic).c_str(), _msg , (_retained)? "true" : "false");
//   return  _host->mqttclient()->publish( (String(_host->pmelvanimate()->deviceName()) + "/" + String(_topic)).c_str() , _msg, _plength, _retained);
//  }
//  return false;
// }
