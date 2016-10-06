#include "JsonPackage.h"

#define MELVANA_MAX_BUFFER_SIZE 2000

int JSONpackage::parse(char * data, int size) {

        //using namespace ESPMAN;

        if (!data) {
                return -1;
        }

        if (_isArray) {
                _root = _jsonBuffer.parseArray(_data.get(),size);
        } else {
                _root = _jsonBuffer.parseObject(_data.get(),size);
        }

        if (!_root.success()) {
                return -1;
        }

        return 0;

}

int JSONpackage::parseSPIFS(const char * file, FS & fs) {

        //using namespace ESPMAN;
        File f = fs.open(file, "r");
        int totalBytes = f.size();
        if (!f) {
                return -1;
        }
        if (totalBytes > MELVANA_MAX_BUFFER_SIZE) {
                f.close();
                return -2;
        }

        _data = std::unique_ptr<char[]>(new char[totalBytes]);

        if (!_data) {
                f.close();
                return -3;
        }
        int position = 0;
        int bytesleft = totalBytes;
        while ((f.available() > -1) && (bytesleft > 0)) {
                // get available data size
                int sizeAvailable = f.available();

                if (sizeAvailable) {
                        int readBytes = sizeAvailable;

                        // read only the asked bytes
                        if (readBytes > bytesleft) {
                                readBytes = bytesleft;
                        }

                        // get new position in buffer
                        char * buf = &_data.get()[position];
                        // read data
                        int bytesread = f.readBytes(buf, readBytes);
                        if (readBytes && bytesread == 0) { break; } //  this fixes a corrupt file that has size but can't be read.
                        bytesleft -= bytesread;
                        position += bytesread;
                        // Serial.printf("totalbytes = %u, sizeAvailable = %u, readBytes = %u, bytesleft = %u, position = %u\n", totalBytes, sizeAvailable, readBytes, bytesleft, position);
                        // Serial.println("chunk = ");
                        // Serial.write(buf, bytesread);
                        // Serial.println();

                }
                // time for network streams
                delay(0);
        }
        f.close();

        if (bytesleft) {
          //Serial.printf("parseERROR: bytesleft = %u\n", bytesleft);
          return -5;
        }

        // Serial.println("BUFFER before parse:");
        // Serial.write(_data.get(), totalBytes);
        // Serial.println();

        if (_isArray) {
                _root = _jsonBuffer.parseArray(_data.get(),totalBytes);
        } else {
                _root = _jsonBuffer.parseObject(_data.get(),totalBytes);
        }
        if (!_root.success()) {
                return -4;
        }

        // Serial.println("BUFFER after parse:");
        // Serial.write(_data.get(), totalBytes);
        // Serial.println();
        //
        // Serial.println("_root =");
        // _root.prettyPrintTo(Serial);
        // Serial.println();

        return 0;

}

void JSONpackage::mergejson(JsonObject& dest, JsonObject& src) {
   for (auto kvp : src) {
     dest[kvp.key] = kvp.value;
   }
}
