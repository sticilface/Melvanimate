1.1

#New Features:

- compatability with ESP8266 arduino 2.3.0 [Arduindo-ESP8266](https://github.com/esp8266/Arduino/releases/tag/2.3.0)
- Moved to async web server.  Provides much faster responses. Less pauses in lights.
- If ESPManager is used, it now requires the async branch.
- Moved to asyncMQTT: not perfect, need to refactor commands really. [asyncMQTT](https://github.com/marvinroger/async-mqtt-client)
- Added locator, now all Melvanimates on the same network will find each other and display in the GUI.  Super easy to manage multiple instances.
- Web Server Events supported.  Pass into melvanimate using   lights.setEventsServer( AsyncEventSource & )
- Beat detection via MSGEQ7, works over UDP to transmit to other Melvanimates.  Only one basic effect, more coming.
  Select via EQ in side menu.  Can acts as server or client for network stream.
- EQ mode, again via MSGEQ7. requires 64 (8x8 matrix).
- Added support for 4 colour LEDs, MACROS found in myBuf.h to switch between 3-4 colours and UART and DMA methods.
- Confirmed support for Platformio with travis CI.  
- New effect ColorBlend.
- Upgraded ColorPicker (still to move to latest multiple instance method).
- Add function to serve SPIFFS files.  Needed if not using ESPManager.
- `begin(const char *)` function now takes name of device, for listenser service.
- Added serverEvent based notifications to index.htm.  OTA and Upgrade events from ESPmanager will appear on GUI.
- App-cache support.  Apps will work offline, giving last list of online devices.  Add it to your homescreen...
  App-cache is automatically refreshed when a new binary with changed `__DATE__ __TIME__`

#Bugfix:

- nullptr exception in OffFn.
- Clear strip to black before changing LED number
- Fix some capitals in includes.
- DEBUG now done with ESP_DEBUG_PORT which needs to also be defined.
- Lots of others i've forgotten.
- Fixed Shapes, back off when no space to 200ms, dramatic improvement in idle CPU time, and effect speed.
- White effect not works for 3 + 4 Color LEDs.
- Fix Palette info request bug.

#Trivial improvements
- std::placeholder usings moved to functions.
- removed some unused code.
