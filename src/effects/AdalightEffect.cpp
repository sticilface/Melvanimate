#include "AdalightEffect.h"


#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include "mybus.h"


extern MyPixelBus * strip;
extern NeoPixelAnimator * animator;

#define DEBUG_ADALIGHT(...) {}

bool AdalightEffect::Start()
{

	if (_Serial) {
		_Serial.flush();
	}

		_Serial.begin(serialspeed());

	animator = new NeoPixelAnimator(1);

	//if (millis() > 30000) {
	if (animator) {
		Adalight_Flash();
	}

_vars->state = MODE_HEADER;

}

bool AdalightEffect::Stop()
{

	if (strip) {
		strip->ClearTo(0);
	}

	if (_vars) {
		delete _vars;
		_vars = nullptr;
	}

	if (animator) {
		delete animator;
		animator = nullptr;
	}

	_Serial.begin(_defaultSpeed);
	_Serial.println();
}





bool AdalightEffect::Run()
{
  uint8_t prefix[] = {'A', 'd', 'a'}, hi, lo, chk, i;

  uint32_t count = _Serial.available();

  switch (_vars->state) {

  case MODE_HEADER:

    _vars->effectbuf_position = 0; // reset the buffer position for DATA collection...

    if (count) { // if there is _Serial available... process it... could be 1  could be 100....
      // _Serial.print("+");

      for (int i = 0; i < count; i++) {  // go through every character in serial buffer looking for prefix...
          delay(0);
        DEBUG_ADALIGHT("+");
        int readbyte = _Serial.read();
        DEBUG_ADALIGHT(".");

        if (readbyte  == prefix[_vars->prefixcount]) { // if character is found... then look for next...
          _vars->prefixcount++;
        } else { _vars->prefixcount = 0;  continue; }  //  otherwise reset....  ////

        if (_vars->prefixcount >= 3) {
          _vars->effect_timeout = millis(); // generates START TIME.....
          _vars->state = MODE_CHECKSUM;
          _vars->prefixcount = 0;
          DEBUG_ADALIGHT("header ");
          break;
        } // end of if prefix == 3
      } // end of for loop going through serial....



    } else if (!count && ( millis() - _vars->ada_sent > SEND_ADA_TIMEOUT ) ) {
      _Serial.print("Ada\n"); // Send "Magic Word" string to host
      _vars->ada_sent = millis();
      _vars->prefixcount = 0;
    } // end of serial available....

    break;

  case MODE_CHECKSUM:

    DEBUG_ADALIGHT(" ck");

    if (count >= 3) {
     // Serial.print("checksum..");
      hi  = _Serial.read();
      lo  = _Serial.read();
      chk = _Serial.read();
      if (chk == (hi ^ lo ^ 0x55)) {
       DEBUG_ADALIGHT(" pass ");
        _vars->state = MODE_DATA;
        break;
      } else {
        DEBUG_ADALIGHT(" failed ");
        _vars->state = MODE_HEADER; // ELSE RESET.......
        break;
      }
    }

    if (  millis() -  _vars->effect_timeout > 1000) { _vars->state = MODE_HEADER; } // RESET IF BUFFER NOT FILLED WITHIN 1 SEC.

    break;

  case MODE_DATA:



    if ( count ) {


     DEBUG_ADALIGHT(" data ");

      for (int i = 0; i < count; i++) {

          int byteread = _Serial.read();


         if (_vars->effectbuf_position < (  (3 * strip->PixelCount() ) - 1 ) ) {

        if (byteread > -1) {
          strip->Pixels()[_vars->effectbuf_position++] = byteread;
        }

        delay(0);
      } else {
        _vars->state = MODE_SHOW;
        break; //  buffer is full....
      }

    }

    }

    if (_vars->effectbuf_position >= 3 * strip->PixelCount() ) { // goto show when buffer has recieved enough data...
     // Serial.println("Show");
      _vars->state = MODE_SHOW;
      break;
    }

    if ( millis() - _vars->effect_timeout > 100 ) {
      _vars->state = MODE_SHOW;
      DEBUG_ADALIGHT("show: timeout");
    } // SHOW IF BUFFER NOT FILLED WITHIN 100ms.

    break;

  case MODE_SHOW:

  {
    DEBUG_ADALIGHT("show");

    if (strip) {
      strip->Dirty();
      _vars->pixellatchtime = millis();
      strip->Show();

    }

    _vars->state = MODE_HEADER;

  break;
  }

  }
  return true;
}
