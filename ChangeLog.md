1.1

New Features:

- Moved to async web server.  Provides much faster responses. Less pauses in lights.
- Added locator, now all Melvanimates on the same network will find each other and display in the GUI.  Super easy to manage multiple instances.
- Beat detection via MSGEQ7, works over UDP to transmit to other Melvanimates.  Only one basic effect, more coming.
  Select via EQ in side menu.  Can acts as server or client for network stream. 
- EQ mode, again via MSGEQ7. requires 64 (8x8 matrix).
- Added support for 4 colour LEDs, MACROS found in myBuf.h to switch between 3-4 colours and UART and DMA methods.
- Confirmed support for Platformio with travis CI.  
- New effect ColorBlend.
- Upgraded ColorPicker (still to move to latest multiple instance method).
- Add function to serve SPIFFS files.  Needed if not using ESPManager.

Bugfix:
- nullptr exception in OffFn.
- Clear strip to black before changing LED number
- Fix some capitals in includes.
- DEBUG now done to ESP_DEBUG_PORT which needs to also be defined.
- Lots of others i've forgotten.
