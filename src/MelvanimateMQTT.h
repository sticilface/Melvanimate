
#pragma once

//#include <PubSubClient.h>
#include <AsyncMqttClient.h>

#include <ESP8266WiFi.h>
#include "Melvanimate.h"
#include "IPAddress.h"
#include <functional>
#include <ArduinoJson.h>

#define DebugMelvanimateMQTT

#if defined(DEBUG_ESP_PORT) && defined(DebugMelvanimateMQTT)
#define DebugMelvanimateMQTTf(...) DEBUG_ESP_PORT.printf(__VA_ARGS__)
#else
#define DebugMelvanimateMQTTf(...) {}
#endif

using namespace std::placeholders;

class Melvanimate;

class MelvanimateMQTT
{
public:

        MelvanimateMQTT(Melvanimate * lights, IPAddress Addr, uint16_t Port = 1883);

        ~MelvanimateMQTT() {
          _mqttClient.disconnect();
        }

        void loop();

        operator bool()
        {
                return _mqttClient.connected();
        }

        //bool publish(const char * topic, const char * payload);
        bool publish(const char * topic, const char * payload, size_t length, bool retained = false );
        bool publish(const char * topic, const char * payload, bool retained = false );

        //void sendFullJson() { _send_flag = millis(); }
        void sendJson(bool onlychanged); //  { _send_changed_flag = millis();  _onlychanged = onlychanged; }
        void sendPresets();

        bool addJson(JsonObject & root);
        bool parseJson(JsonObject & root) {
        };

        IPAddress getIP() {
          return _addr;
        }

        uint16_t getPort() {
          return _port;
        }
        //PubSubClient * mqttclient() { return &_client; }
        // Melvanimate * pmelvanimate() {
        //         return _melvanimate;
        // }

private:


//        void _reconnect();
        void _handle(char* topic, byte* payload, unsigned int length);
				void _onMqttMessage(char* topic, char* payload, uint8_t qos, size_t len, size_t index, size_t total);

        //WiFiClient _espClient;
        //PubSubClient _client;
        uint32_t _reconnectTimer {0};
        Melvanimate * _melvanimate {nullptr};
        AsyncMqttClient _mqttClient;
        bool _disconnected{false};
        uint32_t _timeout{0};
        
        //uint32_t _send_flag{0};
        //uint32_t _send_changed_flag{0};
        IPAddress _addr;
        uint16_t _port;

        //mqtt_message * _firstmessage{nullptr};
        //uint32_t _asyncTimeout{0};
        bool _onlychanged {nullptr};
        uint16_t _reconnecttries {0};

};
