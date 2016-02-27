#include "AdalightEffect.h"


#include "NeopixelBus.h"

extern NeoPixelBus * strip;
extern NeoPixelAnimator * animator;



bool AdalightEffect::Start()
{

	if (_Serial) {
		_Serial.flush();
		delay(500);
		//Serial.end();
	}

	uint32_t speed = 115200;

	speed = serialspeed();

	{
		_Serial.begin(speed);
	}

	Serial.printf("Init: Adalight [%u]\n", speed);

	if (millis() > 30000) { Adalight_Flash(); }


}

bool AdalightEffect::Stop()
{
	if (_vars) {
		delete _vars; 
		_vars = nullptr; 
	}
	_Serial.begin(_defaultSpeed);
}


void  AdalightEffect::Adalight_Flash()
{


	for (int pixel = 0; pixel < strip->PixelCount(); pixel++) {

		RgbColor originalcolor = strip->GetPixelColor(pixel);

		AnimUpdateCallback animUpdate = [pixel, originalcolor] (float progress) {
			RgbColor updatedColor;

			if (progress < 0.25) {
				updatedColor = RgbColor::LinearBlend(originalcolor, RgbColor(100, 0, 0), progress * 4 );
			} else if (progress < 0.5) {
				updatedColor = RgbColor::LinearBlend(RgbColor(100, 0, 0), RgbColor(0, 100, 0) , (progress - 0.25) * 4 );
			} else if (progress < 0.75) {
				updatedColor = RgbColor::LinearBlend(RgbColor(0, 100, 0), RgbColor(0, 0, 100), (progress - 0.5) * 4 );
			} else {
				updatedColor = RgbColor::LinearBlend(RgbColor(0, 0, 100), RgbColor(0, 0, 0), (progress - 0.75) * 4 );
			}
			strip->SetPixelColor(pixel, updatedColor);
		};

		if (animator) {
			animator->StartAnimation(pixel, 2000, animUpdate);
		}


	}

}

bool AdalightEffect::Run()
{
	uint8_t prefix[] = {'A', 'd', 'a'}, hi, lo, chk, i;

	if (!_vars)
	{
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
		} else if (!_Serial.available() && (_vars->ada_sent + 5000) < millis()) {
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

		//  this bit is what might... be causing the flashing... as it extends past memory stuctures....
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
			strip->Dirty(); // MUST USE if you're using the direct buffer copy...
			_vars->pixellatchtime = millis();
			strip->Show();
		}
		//Show_pixels(true);
		_vars->state = MODE_HEADER;
	}
	break;

	}
	return true; 
}