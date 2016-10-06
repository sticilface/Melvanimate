/*

      A simple package to store arduinojson stuff...


*/


#pragma once

#include <ArduinoJson.h>
#include <FS.h>

class JSONpackage {

private:
        DynamicJsonBuffer _jsonBuffer;
        JsonVariant _root;
        std::unique_ptr<char[]> _data;
        bool _isArray {false};

public:
        JSONpackage(bool isArray = false) {
                if(isArray) {
                        _isArray = true;
                        _root = _jsonBuffer.createArray();
                } else {
                        _isArray = false;
                        _root = _jsonBuffer.createObject();
                }
        }
        ~JSONpackage() {
        }

        JsonVariant & getRoot() {
                return _root;
        }

        int parseSPIFS(const char * file, FS & fs = SPIFFS);
        int parse(char * data, int size);
        static void mergejson(JsonObject& dest, JsonObject& src);

};
