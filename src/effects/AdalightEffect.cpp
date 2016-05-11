#include "AdalightEffect.h"


#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include "mybus.h"


extern MyPixelBus * strip;
extern NeoPixelAnimator * animator;



bool AdalightEffect::Start()
{

	animator = new NeoPixelAnimator(1);

	//if (millis() > 30000) { 
	if (animator) {
		Adalight_Flash(); 
	}

	if (_Serial) {
		_Serial.flush();
		//delay(500);
		//Serial.end(); //  this seems to cause reboot
		_Serial.begin(serialspeed());
	}



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

	if (animator)
	{
		if (animator->IsAnimating()) {
			return false; 
		}
	}

	if (animator) {
		delete animator;
		animator = nullptr; 
	}

	if (!_vars) {
		Stop();
		return false;
	}

	switch (_vars->state) {

	case MODE_HEADER:

		_vars->effectbuf_position = 0; // reset the buffer position for DATA collection...

		if (_Serial.available()) { // if there is _Serial available... process it... could be 1  could be 100....

			for (int i = 0; i < _Serial.available(); i++) {  // go through every character in serial buffer looking for prefix...

				if (_Serial.read() == prefix[_vars->prefixcount]) { // if character is found... then look for next...
					_vars->prefixcount++;
				} else { _vars->prefixcount = 0; }  //  otherwise reset....  ////

				if (_vars->prefixcount == 3) {
					_vars->effect_timeout = millis(); // generates START TIME.....
					_vars->state = MODE_CHECKSUM;
					_vars->prefixcount = 0;
					break;
				} // end of if prefix == 3
			} // end of for loop going through serial....
		} else if (!_Serial.available() && (_vars->ada_sent + SEND_ADA_TIMEOUT) < millis()) {
			_Serial.print("Ada\n"); // Send "Magic Word" string to host
			_vars->ada_sent = millis();
		} // end of serial available....

		break;

	case MODE_CHECKSUM:

		if (_Serial.available() >= 3) {
			hi  = _Serial.read();
			lo  = _Serial.read();
			chk = _Serial.read();
			if (chk == (hi ^ lo ^ 0x55)) {
				_vars->state = MODE_DATA;
			} else {
				_vars->state = MODE_HEADER; // ELSE RESET.......
			}
		}

		if ((_vars->effect_timeout + 1000) < millis()) { _vars->state = MODE_HEADER; } // RESET IF BUFFER NOT FILLED WITHIN 1 SEC.

		break;

	case MODE_DATA:

		while (_Serial.available() && _vars->effectbuf_position < 3 * strip->PixelCount()) {  // was <=

			strip->Pixels()[_vars->effectbuf_position++] = _Serial.read();
		}

		if (_vars->effectbuf_position >= 3 * strip->PixelCount()) { // goto show when buffer has recieved enough data...
			_vars->state = MODE_SHOW;
			break;
		}

		if ((_vars->effect_timeout + 1000) < millis()) { _vars->state = MODE_HEADER; } // RESET IF BUFFER NOT FILLED WITHIN 1 SEC.

		break;

	case MODE_SHOW:

	{
		if (strip) {
			strip->Dirty();
			_vars->pixellatchtime = millis();
			strip->Show();
		}
		_vars->state = MODE_HEADER;
	}
	break;

	}
	return true;
}