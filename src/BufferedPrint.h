#pragma once

#include <ESP8266WiFi.h>

//#include <ESP8266WebServer.h>
//#include <ESP8266HTTPClient.h>
#include <Print.h>


// template<size_t CAPACITY>
// class BufferedPrint : public Print
// {
// public:
// 	BufferedPrint(ESP8266WebServer & HTTP) : _size(0)
// 	{
// 		_client = HTTP.client();
// 	}

// 	BufferedPrint(WiFiClient & client) : _client(client), _size(0)
// 	{
// 	}

// 	~BufferedPrint()
// 	{
// 		_client.stop();
// 	}

// 	virtual size_t write(uint8_t c)
// 	{
// 		_buffer[_size++] = c;

// 		if (_size + 1 == CAPACITY) {
// 			flush();
// 		}
		
// 		return 1;
// 	}

// 	void flush()
// 	{
// 		_client.write( (const char *)_buffer, _size);
// 		_size = 0;
// 	}

// 	void stop()
// 	{
// 		_client.stop();
// 	}

// private:
// 	WiFiClient _client;
// 	size_t _size;
// 	char _buffer[CAPACITY];
// };



