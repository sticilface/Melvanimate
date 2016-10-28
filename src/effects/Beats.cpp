#include "Beats.h"
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include "Melvanimate.h" // required for the MAX_NUMBER_OF_ANIMATIONS definition

#include "Palette.h"

extern MyPixelBus * strip;
extern NeoPixelAnimator * animator;

using namespace helperfunc;

bool Beats::InitVars()
{
	addVar(new Variable<uint8_t>("brightness", 100));

	addVar(new Variable<uint8_t>("BeatMode", 1));

	//addVar(new Variable<uint8_t>("filter", 80));
	//addVar(new Variable<float>("beatsratio", 2.1));
	//addVar(new Variable<uint8_t>("beatstimeout", 100));
	addVar(new Variable<EQ*>(1000, 3000)); //  EQ(samples, sampletime) //  sets the defaults for EQ beat detection...  Start with no params for just graphic equaliser settings.
	addVar(new Variable<Palette*>("Palette", Palette::OFF));

	if (palette()) {

		palette()->mode(Palette::WHEEL);
		palette()->randommode(Palette::TOTAL_RANDOM);

	}

	_EQ = getVar<EQ*>("EQ");


}


bool Beats::Start()
{
	using namespace std::placeholders;

	if (strip) {
		strip->ClearTo(0);
	}

	if (animator) {
		delete animator;
		animator = nullptr;
	}

	if (animator) {
		delete animator;
		animator = nullptr;
	}

	_pixels =  ( strip->PixelCount() < MAX_NUMBER_OF_ANIMATIONS ) ? strip->PixelCount() : MAX_NUMBER_OF_ANIMATIONS;

	animator = new NeoPixelAnimator( _pixels  );

	if (_EQ && palette()) {

		switch (mode()) {

		case 0:
			_EQ->SetBeatCallback(  std::bind(&Beats::effect1, this, _1 ) );
			break;
		case 1:
			_EQ->SetBeatCallback(  std::bind(&Beats::effect2, this, _1 ) );
			break;
		case 2:
			_EQ->SetBeatCallback(  std::bind(&Beats::bassEffect, this, _1 ) );
			break;
		case 3:
			_EQ->SetBeatCallback(  std::bind(&Beats::snakeEffectCb, this, _1 ) );
			break;
		case 4:
			_EQ->SetBeatCallback(  std::bind(&Beats::effect4, this, _1 ) );
			break;
		case 5:
			_EQ->SetBeatCallback(  std::bind(&Beats::effect4, this, _1 ) );
			break;
		default :
			_EQ->SetBeatCallback(  std::bind(&Beats::stripEffect, this, _1 ) );
			break;
		}

		_currentColor = palette()->next();
		_colortimeout = millis();


		_animUpdate1 = [this](const AnimationParam & aniparam) {

			//float progress = aniparam.progress;
			float progress = NeoEase::ExponentialIn(aniparam.progress);

			RgbColor updatedColor = RgbColor::LinearBlend(dim( _currentColor , brightness() ), 0,  progress) ;

			strip->SetPixelColor(  aniparam.index  , updatedColor);
		};

		_animUpdate2 = [this](const AnimationParam & aniparam) {
			float progress = NeoEase::ExponentialOut(aniparam.progress);
			float test = sin(progress * 3.141);
			RgbColor updatedColor = RgbColor::LinearBlend(0,  dim( _currentColor , brightness() ),  test) ;
			strip->SetPixelColor(  aniparam.index  , updatedColor);
		};


	}


}

bool Beats::Run()
{


	if (_EQ) {
		_EQ->loop();
	}


	switch (mode()) {
	case 1:

		break;
	case 2: { //  bass effect
		if (millis() - _beattimeout > 2000) {
			_selectedchannel = 2; 
		}

	}

	break;
	case 3:
		snakeEffectRun();
		break;


	default:

		break;

	}


// static uint32_t test = 0;

// 	if (millis() - test > 1000) {

// 			for (uint16_t i = 0; i < _pixels; i++) {


// 				animator->StartAnimation(i, 900 ,  _animUpdate2);

// 			}

// 			test = millis();


// 	}

	// if (millis() - _tick > 30) {

	// 	for (int i = 0; i < 7; i++) {
	// 		int LEDs = map(_EQ->data[i], 80, 1023, 0, 8);
	// 		for (int j = 0; j < 8; j++) {
	// 			strip->SetPixelColor( (i * 8) + j, RgbColor(0));
	// 		}
	// 		for (int j = 0; j < LEDs; j ++) {
	// 			strip->SetPixelColor( (i * 8) + j, dim( RgbColor(50, 0, 0), brightness()) );
	// 		}
	// 	}

	// 	_tick = millis();
	// }

}

bool Beats::Stop()
{

	if (strip) {
		strip->ClearTo(0);
	}

	if (animator) {
		delete animator;
		animator = nullptr;
	}
}


void Beats::effect1(EQParam params)
{

	//if (params.channel == ) {
	//	Serial.printf("[Packet rec %u: Packet sent %u] channel: %u, avg: %u level: %u, bpm: %u\n", _EQ->seq(), params.seq_no , params.channel, params.average, params.level, params.bpm);
	//}

	uint8_t channel = params.channel;
	uint8_t size = 8 - channel;
	uint8_t level = params.level;

	bool pixel_busy = false;
	uint16_t pre_pixel = 0;
	uint16_t count = 0;

	do {
		pre_pixel = random(0, _pixels - size);
		count++;
		for (uint16_t i = pre_pixel; i < pre_pixel + size; i++) {
			if (animator->IsAnimationActive(pre_pixel)) {
				pixel_busy = true;
				break;
			}
		}
	} while (pixel_busy && count < 50);

	//  pixel block not busy
	if (!pixel_busy) {

		uint16_t pixel = pre_pixel;
		RgbColor targetColor = dim( palette()->next() , brightness() );


		for (uint16_t i = pixel; i < pixel + size; i++) {

			AnimUpdateCallback animUpdate = [ = ](const AnimationParam & aniparam) {

				float progress = aniparam.progress;

				RgbColor updatedColor = RgbColor::LinearBlend(targetColor, 0,  progress) ;

				strip->SetPixelColor(  i  , updatedColor);
			};

			animator->StartAnimation(i, level * (size / 3),  animUpdate);

		}
	}


}


void Beats::bassEffect(EQParam params)
{

	if (params.channel == 1) {
		_selectedchannel = 1; //  switches badk to bass if it comes. 
	}

	if (params.channel == _selectedchannel || params.channel == 0 ) {

		if (millis() - _beattimeout > 300) {
				_beattimeout = millis();
		_currentColor =  palette()->next() ;

		for (uint16_t i = 0; i < _pixels; i++) {

			animator->StartAnimation(i, params.level * 6 ,  _animUpdate2);

		}	
		}


	}






}


void Beats::snakeEffectCb(EQParam params)
{

	if (params.channel == 0 || params.channel == 1 ) {

		_direction = _direction * -1;
		_currentColor =  palette()->next() ;

	}
}


void Beats::snakeEffectRun()
{
	if (millis() - _beattimeout > _speed) {

		animator->StartAnimation(_position, _speed * _tail ,  _animUpdate1);
		_position =  _position + _direction;

		if ( _direction > 0 && _position + 1 == strip->PixelCount()) {
			_direction =  -1;
			_currentColor =  palette()->next() ;
		} else if (_direction < 0 && _position - 1 == -1) {
			_direction =  1;
			_currentColor =  palette()->next() ;
		}

		_beattimeout = millis();
	}
}


void Beats::effect2(EQParam params)
{

	if (params.channel == 5 ||  millis() - _colortimeout > 10000) {
		_currentColor =  palette()->next() ;
		_colortimeout = millis();
	}


	if (params.channel == 0 || params.channel == 1 || params.channel == 2) {
		_beattimeout = millis();

		uint8_t level = params.level;
		RgbColor targetColor = dim( _currentColor , brightness() );

		for (uint16_t i = 0; i < _pixels; i++) {

			animator->StartAnimation(i, params.level * 3 ,  _animUpdate2);

		}

	}

	if (millis() - _beattimeout > 3000) {

		_currentColor =  palette()->next() ;
		_colortimeout = millis();

		if (params.channel == 5 || params.channel == 6 || params.channel == 7) {
			_beattimeout = millis();

			uint8_t level = params.level;
			RgbColor targetColor = dim( _currentColor , brightness() );

			for (uint16_t i = 0; i < _pixels; i++) {

				animator->StartAnimation(i, level * 3 ,  _animUpdate2);

			}

		}


	}

}





void Beats::effect4(EQParam params)
{
	//  return if the channel is not the selected one

	//if ()
	if (params.channel == 5 ||  millis() - _colortimeout > 10000) {
		_currentColor =  palette()->next() ;
		_colortimeout = millis();
	}


	if (params.channel == 0 || params.channel == 1 || params.channel == 2) {
		_beattimeout = millis();

		uint8_t level = params.level;
		RgbColor targetColor = dim( _currentColor , brightness() );

		for (uint16_t i = 0; i < _pixels; i++) {

			animator->StartAnimation(i, params.level * 3 ,  _animUpdate2);

		}

	}

	if (millis() - _beattimeout > 3000) {

		_currentColor =  palette()->next() ;
		_colortimeout = millis();

		if (params.channel == 5 || params.channel == 6 || params.channel == 7) {
			_beattimeout = millis();

			uint8_t level = params.level;
			RgbColor targetColor = dim( _currentColor , brightness() );

			for (uint16_t i = 0; i < _pixels; i++) {


				animator->StartAnimation(i, level * 3 ,  _animUpdate2);

			}

		}


	}

}


void Beats::stripEffect(EQParam params)
{




}


