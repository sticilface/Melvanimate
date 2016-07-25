#pragma once
#include <Arduino.h>


struct rtc_data_t {
	uint16_t size{0};
  bool on{false};
	uint8_t effect{0};
  uint8_t state{0};
  uint8_t preset{255};
	uint16_t data{sizeof(this) + 1};
};

//        bool rtcUserMemoryRead(uint32_t offset, uint32_t *data, size_t size);
//        bool rtcUserMemoryWrite(uint32_t offset, uint32_t *data, size_t size);

class RTC_manager {
public:
	RTC_manager(){}
	void save() {

    uint32_t size = sizeof(rtc_data_t);

    rtc_data_t data = _data;

    uint32_t * test = static_cast<uint32_t*>(static_cast<void*>( &data));

    bool res = ESP.rtcUserMemoryWrite(1, test, size );

    //Serial.printf("Addr of _data = %p, Addr of data = %p\n", &_data, &data);

    if (!res) {
      Serial.printf("[RTC_manager::save()] Failed to save data\n", size  );
    } else {
      Serial.printf("[RTC_manager::save()] DATA SAVED .on = %s, effect = %u, preset = %u\n", (data.on)? "on": "off",  data.effect, data.preset);
    }

    uint8_t XOR{0};

    Serial.print("SAVE: ");

    for (size_t i = 0; i < sizeof(rtc_data_t) ; i++) {
      XOR ^= *((uint8_t *) &data + i);
      Serial.printf(" %02X", *((uint8_t *) &data + i));
    }

    Serial.println();

    Serial.printf("effect = %u\n", data.effect);

    if (ESP.rtcUserMemoryWrite(0, (uint32_t*)&XOR, 1 ))
    {
      Serial.printf("Saved: CRC = %u\n", XOR);
    } else {
      Serial.printf("Failed to Save: CRC = %u\n", XOR);
    }


  }
  bool load() {
     rtc_data_t temp_data;
     bool res = ESP.rtcUserMemoryRead(1, (uint32_t*)&temp_data, sizeof(rtc_data_t)  );

     if (!res) {
       Serial.println("Failed to Load data");
     }

     uint8_t xorbyte{0};

     if (!ESP.rtcUserMemoryRead(0, (uint32_t*)&xorbyte, 1)) {
       Serial.println("Failed to load CRC byte");
     }
     Serial.print("LOAD: ");


     uint8_t XOR{0};
    for (size_t i = 0; i < sizeof(rtc_data_t) ; i++) {
      XOR ^= *((uint8_t *) &temp_data + i);
      Serial.printf(" %02X", *((uint8_t *) &temp_data + i));
    }
    Serial.println();

    if (XOR == xorbyte) {
      Serial.printf("CRC match %u\n", XOR);
      _data = temp_data;
      _loaded = true;
      return true;
    } else {
      Serial.printf("CRC Do Not Match byte1 = %u, XOR = %u\n", xorbyte, XOR);
      _loaded = false;
      return false;
    }

  }

  operator bool() const {
    return _loaded;
  }

rtc_data_t & get() {
  return _data;
  }


	enum rtc_ani_state_t : uint8_t {
		UNKNOWN = 0,
		NONE,
		PRESET,
    OFF_TOGGLE
	};

private:
  rtc_data_t _data;
  bool _loaded{false};

};
