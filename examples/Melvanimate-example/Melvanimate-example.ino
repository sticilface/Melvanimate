/*-------------------------------------------------------------------------------------------------------

              ESP8266 & Arduino IDE
              Animation software to control WS2812 - several requirements.
              Software requires jquery + jquerymobile in SPIFFS, along with index.htm
              Use SPIFFS Data upload to flash files to SPIFFS.
              Connect WS2812 to PIN 2 of ESP8266.

  Sticilface - Beerware licence
--------------------------------------------------------------------------------------------------------*/

//#include <GDBStub.h>


#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
//#include <ESP8266HTTPUpdateServer.h>
//#include <ESP8266WebServer.h>
//#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>

#include <ArduinoJson.h>
#include <NeoPixelBus.h>
#include <pubsubclient.h>
#include <Adafruit_GFX.h>

//#include <ESPmanager.h>
//#include <FSBrowser.h>
#include <Melvanimate.h>
#include "SimpleTimer/_SimpleTimer.h"


#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "AsyncWebServerResponseImpl.h"

//#include <print.h>
//AsyncResponseStream *stream;
AsyncWebServerRequest *client;

String _global_page = "";
// template <class T> void sendJsontoHTTPnew(const T & root, AsyncWebServerRequest *request){
//   *stream = new AsyncResponseStream("text/json", root.measureLength());
//   request->send(stream);
// }

// #include <cont.h>
// #include <stddef.h>
// #include <ets_sys.h>


//AsyncResponseStream *stream = nullptr;

//#include <cbuf.h>
extern "C" void system_set_os_print(uint8 onoff);
extern "C" void ets_install_putc1(void* routine);

//Use the internal hardware buffer
static void _u0_putc(char c)
{
  while (((U0S >> USTXC) & 0x7F) == 0x7F);
  U0F = c;
}

#include "misc.h"

// template<size_t CAPACITY>
// class AsyncPrinter: public Print {
// private:
//     AsyncClient *_client;
//     cbuf *_buffer;
// public:
// AsyncPrinter():_client(NULL),_buffer(NULL){}
// AsyncPrinter(AsyncClient *client):_client(client){
//       client->onPoll([](void *obj, AsyncClient* c){
//         AsyncPrinter *p = (AsyncPrinter*)obj;
//         p->sendBuffer();
//       }, this);
//       client->onDisconnect([](void *obj, AsyncClient* c){
//         AsyncPrinter *p = (AsyncPrinter*)obj;
//         p->_on_close();
//         c->free();
// delete c;
//       }, this);
//       _buffer = new cbuf(CAPACITY);
//     }
// ~AsyncPrinter(){
// _on_close();
//     }
// size_t write(uint8_t data){
// if(!connected())
// return 0;
//       _buffer->write(data);
// if(_buffer->getSize() == CAPACITY)
// sendBuffer();
// return 1;
//     }
// size_t write(const uint8_t *data, size_t len){
// if(!connected())
// return 0;
// size_t space = _buffer->room();
// size_t toWrite = 0;
// if(space < len){
//         toWrite = len - space;
//         _buffer->write((const char*)data, toWrite);
// sendBuffer();
//       }
//       _buffer->write((const char*)(data+toWrite), len - toWrite);
// return len;
//     }
// bool connected(){
// return (_client != NULL && _client->connected());
//     }
// void close(){
// if(_client != NULL)
//         _client->close();
//     }
// void sendBuffer(){
// size_t available = _buffer->getSize();
// if(available == 0)
// return;
// while(!_client->canSend())
// delay(0);
// char *out = new char[available];
//       _buffer->read(out, available);
//       _buffer->flush();
//       _client->write(out, available);
// delete out;
//     }
// void _on_close(){
// if(_client != NULL){
//         _client->onPoll(NULL, NULL);
//         _client->onDisconnect(NULL, NULL);
//       }
//       _client = NULL;
//       _buffer->flush();
// delete _buffer;
//     }
// };

//    void send(Stream &stream, String contentType, size_t len);

// template <class T> void sendJsontoHTTP( const T & root, AsyncWebServerRequest *request)
// {

//      size_t jsonlength = root.measureLength();
//     // _HTTP.setContentLength(jsonlength);

//      AsyncWebServerResponse response;
//      response.addHeader("Content-Length", String(jsonlength));
//      response.addHeader("Content-Type", "text/json");
//      request->send(&response);
//      AsyncPrinter<1440> proxy(request->client());
//      root.printTo(proxy);
//      proxy.sendBuffer();
//      //proxy.stop();

// }

// size_t sendJson(uint8_t *buf,  size_t maxlen){
//   //fill buf with as much as you can up to maxlen and return the amount written
//   return json.read(buf, maxlen);

//   uint8_t * buf = new uint8_t(1024);

// }

// void respond(AsyncWebServerRequest *req){
//   req->send("text/json", lengthOfJson, sendJson);//<<lengthOfJson is ContentLength
// }


AsyncWebServer HTTP(80);
//FSBrowser fsbrowser(HTTP);
//ESPmanager settings(HTTP, SPIFFS, "Melvanimate-square", "SKY", "wellcometrust");

//ESPmanager settings(HTTP, SPIFFS, "Melvanimate-square", "SONET_1", "tachi123");

//ESPmanager settings(HTTP, SPIFFS, "Melvanimate", "VodafoneMobileWiFi-CDD1C0", "WCZ8J89175");

//ESPmanager settings(HTTP, SPIFFS, "Melvanimate", "MobileWiFi-743e", "wellcometrust");

const char * http_username = "andrew";
const char * http_password = "test";

struct XY_t {
  int x;
  int y;
} XY;

//  MQTT

IPAddress mqtt_server_ip(192, 168, 1, 1);

WiFiClient mqtt_wclient;
PubSubClient mqtt(mqtt_wclient, mqtt_server_ip);

//  This initialises everything.

Melvanimate lights;

uint32_t save_flag = 0;
bool modechange = false;


void setup()
{

  Serial.begin(115200);
  ets_install_putc1((void *) &_u0_putc);
  system_set_os_print(1);

  Serial.println("");
  //Serial.setDebugOutput(true);

  SPIFFS.begin();

  lights.begin();

// settings.begin();
// fsbrowser.begin();


  //HTTP.on("/crash", HTTP_ANY, crashfunc);

  // request->on("/stack", HTTP_ANY, []() {
  //   cont_ stackvars;
  //   if (cont_get_free_stack(&stackvars)){

  //   }

  // });
  //HTTP.serveStatic("/espman", SPIFFS, "/espman", "max-age=86400");


  HTTP.on("/data.esp", HTTP_ANY, handle_data);

  HTTP.on("/debug", HTTP_GET, [](AsyncWebServerRequest * request) {
    static bool debugstate = false;
    debugstate = !debugstate;
    Serial.setDebugOutput(debugstate);
    //request->setContentLength(0);
    request->send(200); // sends OK if were just receiving data...
  });

  HTTP.on("/json", HTTP_ANY, [](AsyncWebServerRequest * request) {

    Serial.println();
    Serial.printf("New Json request: heap = %u\n", ESP.getFreeHeap()); 

    JsonObject * out = nullptr; 

    AsyncJsonResponse * responce = new AsyncJsonResponse(&out); 
    DynamicJsonBuffer buffer = responce->getBuffer(); 

     //out = &buffer.createObject(); 

    JsonObject& test = responce->createObject(); 

    //(*out)["1"] = "blah";
     test["2"] = "blahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["3"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["4"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["5"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["6"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["7"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["8"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["9"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["10"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["11"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["12"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["13"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["14"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["15"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["16"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["17"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["18"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["19"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["20"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["21"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";
     test["22"] = "blahblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[bblahrerqjvnrjnvqerjkvnqerjkvnq;rjvnqrkjvnqerjvbqer[jvbqer[jvbqer[vjbqrv[b";


     responce->dump();

     request->send(responce); 

      //uint8_t _data[100];
      //ChunkPrint dest( _data , 0, test.measureLength() ); 
      //test.printTo( dest ) ;


  });


  HTTP.on("/command", HTTP_ANY, [](AsyncWebServerRequest * request) {
    if (request->hasArg("save")) {
      lights.newSave(request->arg("save").toInt());
      Serial.printf("[request->on/command] done, heap: %u\n", ESP.getFreeHeap());
      //request->setContentLength(0);
      request->send(200); // sends OK if were just receiving data...
    }

    if (request->hasArg("load")) {
      lights.newLoad(request->arg("load").toInt());
      Serial.printf("[load] done, heap: %u\n", ESP.getFreeHeap());
      Serial.printf("[load] current preset = %u\n", lights.Current()->getPreset());
      //request->setContentLength(0);
      request->send(200); // sends OK if were just receiving data...
    }

    if (request->hasArg("print")) {
      File f = SPIFFS.open(PRESETS_FILE, "r");
      Serial.println("SETTINGS_FILE");

      do {
        char buf[250];
        uint8_t number = (f.size() - f.position() > 250) ? 250 : f.size() - f.position();
        f.readBytes(buf, number);
        Serial.write(buf, number);
      } while (f.position() < f.size());

      Serial.println("---");

      //    request->setContentLength(0);
      request->send(200); // sends OK if were just receiving data...
    }

    if (request->hasArg("remove")) {
      lights.removePreset(request->arg("remove").toInt());
//     request->setContentLength(0);
      request->send(200); // sends OK if were just receiving data...
    }

    if (request->hasArg("list")) {

      Serial.printf("[list] _numberofpresets = %u\n", lights._numberofpresets);

      for (uint8_t i = 0; i < lights._numberofpresets; i++) {

        char * text = lights._preset_names[i];


        Serial.printf("[%u] %u (%s)\n", i, lights._presets[i], text) ;

      }
    }

  });

  //void serveStatic(const char* uri, fs::FS & fs, const char* path, const char* cache_header = NULL );

  //HTTP.serveStatic("/jqColorPicker.min.js", SPIFFS, "/jqColorPicker.min.js", "max-age=86400");



// -------------------------------------------------------- //

  lights.Add("Off", new SwitchEffect(offFn));                              // working
  lights.Add("SimpleColor", new GeneralEffect(SimpleColorFn));              // working

  lights.Add("Adalight", new AdalightEffect(AdaLightFn));                    // working - need to test

  // lights.Add("UDP", new SwitchEffect(UDPFn));                              // working
  // lights.Add("DMX", new SwitchEffect(DMXfn));                              // need to test - requires custom libs included
  lights.Add("Marquee", new MarqueeEffect(MarqueeFn));                      // works. need to add direction....
  // lights.Add("RainbowCycle", new SwitchEffect(RainbowCycleFn));
  // lights.Add("Rainbow", new SwitchEffect(RainbowFn));
  // lights.Add("BobblySquares", new SwitchEffect(BobblySquaresFn));

// experimental and in testing

  // lights.Add("TIMINGfunc", new SwitchEffect(TimingFn));
  // lights.Add("generic", new Effect(SimpleFn));
  // lights.Add("complex", new ComplexEffect(ComplexFn));
  // lights.Add("oldsnakes", new SwitchEffect(SnakesFn));
  // lights.Add("Object", new SwitchEffect(ObjectFn));


  //timer.setTimeout(5000, []() { lights.Start("Marquee");} ) ;

  // timer.setInterval(1000, []() {
  //   Debugf("HEAP: %u\n", ESP.getFreeHeap());
  // });

  //Adalight_Flash();


  //timer.setTimeout(1000, []() { lights.Start("BobblySquares");} ) ;


  //timer.setTimeout(2000, []() { lights.Start("Off");} ) ;

  lights.Start("Off");

  // timer.setInterval(500, []() {
  //   text2Fn("Wellcome To the Jungle");
  // });

  Serial.print("HEAP: ");
  Serial.println(ESP.getFreeHeap());

  Serial.println(F("Melvanimate Ready"));

  // lights.Current()->GeneralEffect::*pmf()

  //bool (GeneralEffect::*fptr) (uint8_t) = &GeneralEffect::setBrightness;


//  (*fptr)(100);

  // Melvtrix & matrix =  *lights.matrix();

  // ShapeUpdateCallback ShapeCallback = [](uint16_t pixel, int16_t x, int16_t y) {
  //   Serial.printf("%3u ", pixel);
  // };


  // matrix.setShapeFn( ShapeCallback );


  // for (int x = 0; x < matrix.width(); x++) {
  //   for (int y = 0; y < matrix.height(); y++) {
  //     matrix.drawPixel(x, y);
  //   }
  //   Serial.println();
  // }

  // lights.palette().getModeString();


  // do {
  //   Serial.println("remove presets");
  // } while (SPIFFS.remove(PRESETS_FILE) );

  // do {
  //   Serial.println("remove presets");
  // } while (SPIFFS.remove("/MelvanaSettings.txt") );



  HTTP.addHandler(new SPIFFSEditor(http_username, http_password));

  HTTP.serveStatic("/", SPIFFS, "/");


  HTTP.onNotFound([](AsyncWebServerRequest * request) {
    os_printf("NOT_FOUND: ");
    if (request->method() == HTTP_GET)
      os_printf("GET");
    else if (request->method() == HTTP_POST)
      os_printf("POST");
    else if (request->method() == HTTP_DELETE)
      os_printf("DELETE");
    else if (request->method() == HTTP_PUT)
      os_printf("PUT");
    else if (request->method() == HTTP_PATCH)
      os_printf("PATCH");
    else if (request->method() == HTTP_HEAD)
      os_printf("HEAD");
    else if (request->method() == HTTP_OPTIONS)
      os_printf("OPTIONS");
    else
      os_printf("UNKNOWN");
    os_printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if (request->contentLength()) {
      os_printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      os_printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for (i = 0; i < headers; i++) {
      AsyncWebHeader* h = request->getHeader(i);
      os_printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for (i = 0; i < params; i++) {
      AsyncWebParameter* p = request->getParam(i);
      if (p->isFile()) {
        os_printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if (p->isPost()) {
        os_printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        os_printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });

  HTTP.onFileUpload([](AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index)
      os_printf("UploadStart: %s\n", filename.c_str());
    os_printf("%s", (const char*)data);
    if (final)
      os_printf("UploadEnd: %s (%u)\n", filename.c_str(), index + len);
  });
  HTTP.onRequestBody([](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if (!index)
      os_printf("BodyStart: %u\n", total);
    os_printf("%s", (const char*)data);
    if (index + len == total)
      os_printf("BodyEnd: %u\n", total);
  });


  HTTP.begin();

  // Start OTA server.
  //ArduinoOTA.setHostname("nodemcu");

  ArduinoOTA.begin();


}

void loop()
{
  ArduinoOTA.handle();

  uint32_t _tick = millis();
  uint32_t _arrays[10] = {0};
  uint8_t poss = 0;
  _arrays[0] = millis();

  // request->handleClient();

// settings.handle();

  lights.Loop();

  Show_pixels(false); // bool is show pixels override...

  timer.run();

  if (save_flag) {
    if (millis() - save_flag > 100) {
      save_flag = 0;
      lights.save(modechange); //  will only save if actually required.
    }
  }


  if (_global_page.length() > 0 && client) {
    send_data(_global_page);
    //client = nullptr;
    _global_page = "";

  }

  // if (millis() - _tick > 1000 ) {
  //   Serial.printf("Loop >1S %u\n", millis() - _tick);
  // }



}

void Show_pixels(bool override)
{
  static uint32_t tick = 0;
  if (override) { tick = 0; }
  if ( millis() - tick < 30) { return; }
  if (lights.animations()) {
    if ( animator->IsAnimating() ) { animator->UpdateAnimations(100); }
  }
  strip->Show();
  tick = millis();
}









void testshapegenerater()
{
  uint16_t pixels[4] ;
  Palette localP(WHEEL, 8 * 4);
  static uint8_t position = 0;
  static uint8_t counter = 0;
  RgbColor next = Palette::wheel(counter);

  Melvtrix& matrix = *lights.matrix();

  matrix.setShapeFn( [&localP, &next] (uint16_t pixel, int16_t x, int16_t y) {
    FadeToAndBack(pixel, next, 400);
  });

  matrix.drawRect(0 + position, 0 + position, 8 -  2 * position, 8 - 2 * position, 0);

  int16_t circle[20] { -1};
  int16_t line[20] { -1};


  position++;
  position %= 4;
  counter += 10;
}

void FadeToAndBack(uint16_t pixel, RgbColor color, uint16_t time)
{
  RgbColor originalcolor = strip->GetPixelColor(pixel);
  AnimUpdateCallback animUpdate = [pixel, originalcolor, color] (float progress) {
    RgbColor updatedColor;
    if (progress < 0.5) {
      updatedColor = RgbColor::LinearBlend(originalcolor, color, progress * 2 );
    } else {
      updatedColor = RgbColor::LinearBlend(color, 0, (progress - 0.5) * 2 );
    }
    strip->SetPixelColor(pixel, updatedColor);
  };
  StartAnimation(pixel, time , animUpdate);
}

void OnOff(uint16_t pixel, RgbColor color, uint16_t time)
{
  AnimUpdateCallback animUpdate = [pixel, color] (float progress) {
    if (progress < 1.0) {
      strip->SetPixelColor(pixel, color);
    } else {
      strip->SetPixelColor(pixel, RgbColor(0));
    }

  };
  StartAnimation(pixel, time , animUpdate);
}


void print_args(AsyncWebServerRequest *request)
{
  Serial.printf("[args] %u\n", request->args());

  for (uint8_t i = 0; i < request->args(); i++) {
    Serial.print("[ARG:");
    Serial.print(i);
    Serial.print("] ");
    Serial.print(request->argName(i));
    Serial.print(" = ");
    Serial.println(request->arg(i));
  }


  int params = request->params();
  for (int i = 0; i < params; i++) {
    AsyncWebParameter* p = request->getParam(i);
    if (p->isFile()) {
      Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
    } else if (p->isPost()) {
      Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    } else {
      Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
  }


}

//  this is required as some
bool check_duplicate_req(AsyncWebServerRequest *request)
{
  static uint32_t last_time = 0;
  static char last_request[16] = {0};
  if (request->hasArg("data")) return false;

  MD5Builder md5;
  md5.begin();

  for (uint8_t args = 0; args < request->args(); args++) {
    String req = request->argName(args) + request->arg(args);
    md5.add(req);
  }

  md5.calculate();
  bool match = false;
  //Serial.printf("[MD5] %s\n", md5.toString().c_str());
  char this_request[16] = {0};
  md5.getChars(this_request);

  if (memcmp(last_request, this_request, 16) == 0) {
    match = true;
    //Serial.println("request ignored: duplicate");
  }

  memcpy(last_request, this_request, 16);

  bool time_elapsed = (millis() - last_time > 10000) ? true : false;
  last_time = millis();

  return match & !time_elapsed;

}

void handle_data(AsyncWebServerRequest *request)

{
  uint32_t start_time = millis();
  String page = "homepage";
  //  this fires back an OK, but ignores the request if all the args are the same.  uses MD5.
  if (check_duplicate_req(request)) { request->send(200); return; }

  Serial.println();
  print_args(request);

  if (request->hasArg("mode")) {
    Serial.println("[handle] HAS MODE PARAM");
  } else {
    Serial.println("[handle] FAIL NO MODE PARAM");

  }



  if (request->hasArg("json")) {

    DynamicJsonBuffer jsonBufferplain;
    JsonObject& root = jsonBufferplain.parseObject(request->arg("plain").c_str());
    if (root.success()) {

      if (lights.Current()) {
        if (lights.Current()->args(root)) {
          Serial.println("[handle] JSON (via Plain) Setting applied");
        }
      }

    }
  }

  if (request->hasArg("enable")) {
    if (request->arg("enable").equalsIgnoreCase("on")) {
      lights.Start();
    } else if (request->arg("enable").equalsIgnoreCase("off")) {
      lights.Start("Off");
    }
  }

  if (request->hasArg("mode")) {
    modechange = lights.Start(request->arg("mode"));
    if (request->arg("mode") != "Off") { lights.SetToggle(request->arg("mode").c_str()); }
  }


  if (request->hasArg("preset")) {
    uint8_t preset = request->arg("preset").toInt();
    if (lights.newLoad(preset)) {
      //  try to switch current effect to preset...
      Serial.printf("[handle] Loaded preset %u\n", preset);
    }

  }


  DynamicJsonBuffer jsonBuffer;

  JsonObject & root = jsonBuffer.createObject();

  for (uint8_t i = 0; i < request->args(); i++) {
    root[request->argName(i)] = request->arg(i);
  }

  if (lights.Current()) {
    if (lights.Current()->args(root)) {
      Serial.println("[handle] JSON Setting applied");
    }
  }

  if (request->hasArg("nopixels") && request->arg("nopixels").length() != 0) {
    lights.setPixels(request->arg("nopixels").toInt());
    page = "layout";

  }

  if (request->hasArg("palette")) {
    lights.palette().mode(request->arg("palette").c_str());
    page = "layout";

  }



// matrixmode stuff
// #define NEO_MATRIX_TOP         0x00 // Pixel 0 is at top of matrix
// #define NEO_MATRIX_BOTTOM      0x01 // Pixel 0 is at bottom of matrix
// #define NEO_MATRIX_LEFT        0x00 // Pixel 0 is at left of matrix
// #define NEO_MATRIX_RIGHT       0x02 // Pixel 0 is at right of matrix
// #define NEO_MATRIX_CORNER      0x03 // Bitmask for pixel 0 matrix corner
// #define NEO_MATRIX_ROWS        0x00 // Matrix is row major (horizontal)
// #define NEO_MATRIX_COLUMNS     0x04 // Matrix is column major (vertical)
// #define NEO_MATRIX_AXIS        0x04 // Bitmask for row/column layout
// #define NEO_MATRIX_PROGRESSIVE 0x00 // Same pixel order across each line
// #define NEO_MATRIX_ZIGZAG      0x08 // Pixel order reverses between lines
// #define NEO_MATRIX_SEQUENCE    0x08 // Bitmask for pixel line order

// #define NEO_TILE_TOP           0x00 // First tile is at top of matrix
// #define NEO_TILE_BOTTOM        0x10 // First tile is at bottom of matrix
// #define NEO_TILE_LEFT          0x00 // First tile is at left of matrix
// #define NEO_TILE_RIGHT         0x20 // First tile is at right of matrix
// #define NEO_TILE_CORNER        0x30 // Bitmask for first tile corner
// #define NEO_TILE_ROWS          0x00 // Tiles ordered in rows
// #define NEO_TILE_COLUMNS       0x40 // Tiles ordered in columns
// #define NEO_TILE_AXIS          0x40 // Bitmask for tile H/V orientation
// #define NEO_TILE_PROGRESSIVE   0x00 // Same tile order across each line
// #define NEO_TILE_ZIGZAG        0x80 // Tile order reverses between lines
// #define NEO_TILE_SEQUENCE      0x80 // Bitmask for tile line order

  if (request->hasArg("grid_x") && request->hasArg("grid_y")) {
    lights.grid(request->arg("grid_x").toInt(), request->arg("grid_y").toInt() );
    page = "layout";
  }

  if (request->hasArg("matrixmode")) {
    page = "layout";
    uint8_t matrixvar = 0;
    if (request->arg("matrixmode") == "singlematrix") { lights.multiplematrix = false; }
    if (request->arg("firstpixel") == "topleft") { matrixvar += NEO_MATRIX_TOP + NEO_MATRIX_LEFT; }
    if (request->arg("firstpixel") == "topright") { matrixvar += NEO_MATRIX_TOP + NEO_MATRIX_RIGHT; }
    if (request->arg("firstpixel") == "bottomleft") { matrixvar += NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT; }
    if (request->arg("firstpixel") == "bottomright") { matrixvar += NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT; }

    if (request->arg("axis") == "rowmajor") { matrixvar += NEO_MATRIX_ROWS; }
    if (request->arg("axis") == "columnmajor") { matrixvar += NEO_MATRIX_COLUMNS ; }

    if (request->arg("sequence") == "progressive") { matrixvar += NEO_MATRIX_PROGRESSIVE ; }
    if (request->arg("sequence") == "zigzag") { matrixvar += NEO_MATRIX_ZIGZAG ; }

    if (request->arg("matrixmode") == "multiplematrix") {
      lights.multiplematrix = true;
      if (request->arg("multimatrixtile") == "topleft") { matrixvar += NEO_TILE_TOP + NEO_TILE_LEFT; }
      if (request->arg("multimatrixtile") == "topright") { matrixvar += NEO_TILE_TOP + NEO_TILE_RIGHT; }
      if (request->arg("multimatrixtile") == "bottomleft") { matrixvar += NEO_TILE_BOTTOM + NEO_TILE_LEFT; }
      if (request->arg("multimatrixtile") == "bottomright") { matrixvar += NEO_TILE_BOTTOM + NEO_TILE_RIGHT; }
      if (request->arg("multimatrixaxis") == "rowmajor") { matrixvar += NEO_TILE_ROWS ; }
      if (request->arg("multimatrixaxis") == "columnmajor") { matrixvar += NEO_TILE_COLUMNS ; }
      if (request->arg("multimatrixseq") == "progressive") { matrixvar += NEO_TILE_PROGRESSIVE ; }
      if (request->arg("multimatrixseq") == "zigzag") { matrixvar += NEO_TILE_ZIGZAG ; }
    }

    Debugf("NEW Matrix params: %u\n", matrixvar);
    lights.setmatrix(matrixvar);
  }


  // if (request->hasArg("serialspeed")) {
  //   if (lights.Current()) {
  //     lights.Current()->setSerialspeed(request->arg("serialspeed").toInt());
  //   }
  // }

  if (request->hasArg("flashfirst")) {
    page = "layout";
    lights.Start("Off");
    lights.Stop();
    strip->ClearTo(0);
    AnimUpdateCallback animUpdate = [] (float progress) {
      strip->SetPixelColor(0, Palette::wheel( (uint8_t)(progress * 255) ));
      if (progress == 1.0) { strip->SetPixelColor(0, 0); }
    };
    StartAnimation(0, 5000 , animUpdate);



  }

  if (request->hasArg("revealorder")) {
    page = "layout";
    lights.Start("Off");
    lights.Stop();
    strip->ClearTo(0);
    // ToDo
    float ratio = 1.0 / strip->PixelCount();

    for (uint16_t pixel = 0; pixel < strip->PixelCount() ; pixel++) {
      AnimUpdateCallback animUpdate = [ratio, pixel] (float progress) {
        if ( (uint8_t)(progress * 100) == (uint8_t)(pixel * ratio * 100)) {
          strip->SetPixelColor(pixel, Palette::wheel( (uint8_t)(ratio * 255)));
          strip->SetPixelColor( (pixel > 2) ? pixel - 2 : 0 , 0 );

        }
        if (progress == 1.0) { lights.Start("Off"); }
      };
      StartAnimation(pixel, 5000 , animUpdate);
    }


  }

  if (request->hasArg("palette-random")) {
    lights.palette().randommode(request->arg("palette-random").c_str());
    page = "palette";

  }


  if (request->hasArg("palette-spread")) {
    lights.palette().range(request->arg("palette-spread").toFloat());
    page = "palette";
  }

  if (request->hasArg("palette-delay")) {
    lights.palette().delay(request->arg("palette-delay").toInt());
    page = "palette";

  }


  if (request->hasArg("data")) {
    //send_data(request, request->arg("data")); // sends JSON data for whatever page is currently being viewed
    _global_page = request->arg("data");
  }

  if (request->hasArg("enabletimer")) {
    page = "timer";
    if (request->arg("enabletimer") == "on") {

      if (request->hasArg("timer") && request->hasArg("timercommand")) {

        String effect =  (request->hasArg("timeroption")) ? request->arg("timeroption") : String();

        if (lights.setTimer(request->arg("timer").toInt(), request->arg("timercommand"), effect )) {
          Serial.println("[handle] Timer command accepted");
        }
      }
    } else if (request->arg("enabletimer") == "off") {
      lights.setTimer(0, "off");
    }

  }

  //request->setContentLength(0);
  //request->send(200); // sends OK if were just receiving data...
  //send_data(request, page);
  if (_global_page.length() == 0) _global_page = page;

  //int len = send_data(page, true);

  if (client == NULL) {
    Serial.println("[handle] client = request");
    client = request;
  } else {
    Serial.println("[handle] client already.... chaining....");
//   client->send(500);
//    client= nullptr;
    AsyncWebServerRequest *c = client;
    while (c->next != NULL) c = c->next;
    c->next = request;
  }

  // if(client == NULL){
  //   client = request;
  // } else {
  //   AsyncWebServerRequest *c = client;
  //   while(c->next != NULL) c = c->next;
  //   c->next = request;
  // }


  save_flag = millis();
  Serial.printf("[handle] time %u: [Heap] %u\n", millis() - start_time, ESP.getFreeHeap());
  return;

}






int send_data(String page)
{
  Serial.println("[send_data] hit");

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  /*
        Home page
  */

  if (page == "homepage" || page == "all") {
    JsonArray& modes = root.createNestedArray("modes");
    //Serial.printf("Total effects: %u\n", lights.total());
    for (uint8_t i = 0; i < lights.total(); i++) {
      modes.add(lights.getName(i));
    }
    // creates settings node for web page
    JsonObject& settings = root.createNestedObject("settings");
    // adds minimum current effect name, if there if addJson returns false.
    if (lights.Current()) {
      settings["currentpreset"] = lights.Current()->getPreset();
      if (!lights.Current()->addJson(settings)) {
        settings["effect"] = lights.Current()->name();
      }


      if (lights._numberofpresets) {
        JsonObject& currentpresets = root.createNestedObject("currentpresets");
        for (uint8_t i = 0; i < lights._numberofpresets; i++ ) {
          currentpresets[ String(lights._presets[i])] = lights._preset_names[i];
        }
      }
    }


    root["palette"] = String(lights.palette().getModeString());

  }
  /*
        Layout Page

  [ARG:3] matrixmode = singlematrix
  [ARG:4] firstpixel = topleft
  [ARG:5] axis = rowmajor
  [ARG:6] sequence = progressive
  [ARG:7] multimatrixtile = topleft
  [ARG:8] multimatrixaxis = rowmajor
  [ARG:9] multimatrixseq = progressive
  */
  if (page == "layout" || page == "all") {
    root["pixels"] = lights.getPixels();
    root["grid_x"] = lights.getX();
    root["grid_y"] = lights.getY();
    root["multiplematrix"] = lights.multiplematrix;

    root["matrixconfig"] = lights.getmatrix();

    uint8_t matrixconfig = lights.getmatrix();
    bool bottom = (matrixconfig & NEO_MATRIX_BOTTOM) ;
    bool right = (matrixconfig & NEO_MATRIX_RIGHT) ;

// single matrix
    if (!bottom && !right) { root["firstpixel"] = "topleft"; }
    if (!bottom && right) { root["firstpixel"] = "topright"; }
    if (bottom && !right) { root["firstpixel"] = "bottomleft"; }
    if (bottom && right ) { root["firstpixel"] = "bottomright"; }

    if ((matrixconfig & NEO_MATRIX_AXIS) == NEO_MATRIX_ROWS) {
      root["axis"] = "rowmajor";
    } else {
      root["axis"] = "columnmajor";
    }

    if ((matrixconfig & NEO_MATRIX_SEQUENCE) == NEO_MATRIX_PROGRESSIVE) {
      root["sequence"] = "progressive";
    } else {
      root["sequence"] = "zigzag";
    }


// Tiles

    bottom = (matrixconfig & NEO_TILE_BOTTOM) ;
    right = (matrixconfig & NEO_TILE_RIGHT) ;

    if (!bottom && !right) { root["multimatrixtile"] = "topleft"; }
    if (!bottom && right) { root["multimatrixtile"] = "topright"; }
    if (bottom && !right) { root["multimatrixtile"] = "bottomleft"; }
    if (bottom && right ) { root["multimatrixtile"] = "bottomright"; }

    if ((matrixconfig & NEO_TILE_AXIS) == NEO_TILE_ROWS) {
      root["multimatrixaxis"] = "rowmajor";
    } else {
      root["multimatrixaxis"] = "columnmajor";
    }


    if ((matrixconfig & NEO_TILE_SEQUENCE) == NEO_TILE_PROGRESSIVE) {
      root["multimatrixseq"] = "progressive";
    } else {
      root["multimatrixseq"] = "zigzag";
    }


  }

  /*
        palette page
  */


  if (page == "palette" || page == "all") {

    if (page != "all") root["palette"] = String(lights.palette().getModeString()); // ignore if already sent
    root["paletterandom"] = String(lights.palette().randommodeAsString());
    root["palettespread"] = String(lights.palette().range());
    root["palettedelay"] = String(lights.palette().delay());

  }

  if (page == "timer" || page == "all") {

    JsonObject& timerobj = root.createNestedObject("timer");
    timerobj["running"] = lights.isTimerRunning();
    if (lights.isTimerRunning()) {
      JsonArray& remaining = timerobj.createNestedArray("remaining");
      int minutes = timer.getTimeLeft(lights.getTimer()) / ( 1000 * 60) ;
      int seconds = timer.getTimeLeft(lights.getTimer()) / 1000 ;
      seconds %= 60;
      remaining.add(minutes);
      remaining.add(seconds);
    }

  }


//  ESPmanager::sendJsontoHTTP(root, HTTP);
// sendJsontoHTTPnew(root,request);


  // File f = SPIFFS.open("/senddata.txt", "w");
  // if (f) {
  //   Serial.println("[send_data] Json file written");
  //   root.printTo(f);
  //   f.close();
  //   client->send(SPIFFS, "/senddata.txt", "text/json");
  //   //SPIFFS.remove("/senddata.txt");
  // }


  while (client != NULL) {
    Serial.println("[loop] Send Json");
    AsyncResponseStream *stream = client->beginResponseStream("text/json", root.measureLength());
    root.printTo(*stream);
 //   char * data[100];

 //   root.printTo( ChunkPrint(data[0], 0, 100 ) ) ;

    client = client->next;
  }




//        void send(FS &fs, String path, String contentType=String(), bool download=false);




}

void StartAnimation( uint16_t pixel, uint16_t time, AnimUpdateCallback animUpdate)
{
  if (lights.animations()) {
    animator->StartAnimation(pixel, time, animUpdate);
  }

}

void FadeTo(RgbColor color)
{
  uint32_t current_brightness = 0;
  uint32_t target_brightness = color.CalculateBrightness();
  uint32_t brightness = 0;

  for (uint16_t i = 0; i < strip->PixelCount(); i++) {
    current_brightness += strip->GetPixelColor(i).CalculateBrightness();
  }
  current_brightness /= strip->PixelCount();

  if (current_brightness > target_brightness) {
    brightness = current_brightness;
  } else {
    brightness = target_brightness;
  }

  //int32_t difference = abs(brightness - color.CalculateBrightness() );

  Serial.printf("[FadeTo] current brightness %u, target brightness %u, Brightness Diff = %u, time %ums\n", current_brightness, target_brightness, brightness, brightness * 8);

  FadeTo(brightness * 8, color);
}

void FadeTo( uint16_t time, RgbColor color)
{
  if (lights.animations()) {
    animator->FadeTo(time, color);
  } else {
    strip->ClearTo(color);
  }


}


// bool crashfunc(AsyncWebServerrequest *request)
// {

//   NeoPixelBus * voidpointer;

//   voidpointer->Show();

// }







