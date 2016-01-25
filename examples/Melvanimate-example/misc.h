
// WEB HANDLER IMPLEMENTATION
class SPIFFSEditor: public AsyncWebHandler
{
private:
  String _username;
  String _password;
  bool _uploadAuthenticated;
public:
  SPIFFSEditor(String username = String(), String password = String()): _username(username), _password(password), _uploadAuthenticated(false) {}
  bool canHandle(AsyncWebServerRequest *request)
  {
    if (request->method() == HTTP_GET && request->url() == "/edit" && (SPIFFS.exists("/edit.htm") || SPIFFS.exists("/edit.htm.gz")))
      return true;
    else if (request->method() == HTTP_GET && request->url() == "/list")
      return true;
    else if (request->method() == HTTP_GET && (request->url().endsWith("/") || SPIFFS.exists(request->url()) || (!request->hasParam("download") && SPIFFS.exists(request->url() + ".gz"))))
      return true;
    else if (request->method() == HTTP_POST && request->url() == "/edit")
      return true;
    else if (request->method() == HTTP_DELETE && request->url() == "/edit")
      return true;
    else if (request->method() == HTTP_PUT && request->url() == "/edit")
      return true;
    return false;
  }

  void handleRequest(AsyncWebServerRequest *request)
  {
    if (_username.length() && (request->method() != HTTP_GET || request->url() == "/edit" || request->url() == "/list") && !request->authenticate(_username.c_str(), _password.c_str()))
      return request->requestAuthentication();

    if (request->method() == HTTP_GET && request->url() == "/edit") {
      request->send(SPIFFS, "/edit.htm");
    } else if (request->method() == HTTP_GET && request->url() == "/list") {
      if (request->hasParam("dir")) {
        String path = request->getParam("dir")->value();
        Dir dir = SPIFFS.openDir(path);
        path = String();
        String output = "[";
        while (dir.next()) {
          File entry = dir.openFile("r");
          if (output != "[") output += ',';
          bool isDir = false;
          output += "{\"type\":\"";
          output += (isDir) ? "dir" : "file";
          output += "\",\"name\":\"";
          output += String(entry.name()).substring(1);
          output += "\"}";
          entry.close();
        }
        output += "]";
        request->send(200, "text/json", output);
        output = String();
      } else
        request->send(400);
    } else if (request->method() == HTTP_GET) {
      String path = request->url();
      if (path.endsWith("/"))
        path += "index.htm";
      request->send(SPIFFS, path, String(), request->hasParam("download"));
    } else if (request->method() == HTTP_DELETE) {
      if (request->hasParam("path", true)) {
        ESP.wdtDisable(); SPIFFS.remove(request->getParam("path", true)->value()); ESP.wdtEnable(10);
        request->send(200, "", "DELETE: " + request->getParam("path", true)->value());
      } else
        request->send(404);
    } else if (request->method() == HTTP_POST) {
      if (request->hasParam("data", true, true) && SPIFFS.exists(request->getParam("data", true, true)->value()))
        request->send(200, "", "UPLOADED: " + request->getParam("data", true, true)->value());
      else
        request->send(500);
    } else if (request->method() == HTTP_PUT) {
      if (request->hasParam("path", true)) {
        String filename = request->getParam("path", true)->value();
        if (SPIFFS.exists(filename)) {
          request->send(200);
        } else {
          File f = SPIFFS.open(filename, "w");
          if (f) {
            f.write(0x00);
            f.close();
            request->send(200, "", "CREATE: " + filename);
          } else {
            request->send(500);
          }
        }
      } else
        request->send(400);
    }
  }

  void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
  {
    if (!index) {
      if (!_username.length() || request->authenticate(_username.c_str(), _password.c_str()))
        _uploadAuthenticated = true;
      request->_tempFile = SPIFFS.open(filename, "w");
    }
    if (_uploadAuthenticated && request->_tempFile && len) {
      ESP.wdtDisable(); request->_tempFile.write(data, len); ESP.wdtEnable(10);
    }
    if (_uploadAuthenticated && final)
      if (request->_tempFile) request->_tempFile.close();
  }
};





/*

My async web handler stuff.....

*/


  class ChunkPrint : public Print
  {
  public:
    ChunkPrint(uint8_t* destination, size_t from, size_t to)
      : _destination(destination), _to_skip(from), _to_write(to - from), _pos{0} {
        os_printf("[ChunkPrint] ChunkPrint initialised [%u]->[%u]\n", from, to);
      }

     size_t write(uint8_t c)  
    {
      // os_printf("(%u)%u,",_pos++,c); 
      // return 1;

      if (_to_skip > 0) {
        _to_skip--;
      } else if (_to_write > 0) {
        _to_write--;
        _destination[_pos++] = c;
        return 1;
      }
      return 0;
    }

  private:
    uint8_t* _destination;
    size_t _to_skip;
    size_t _to_write;
    size_t _pos; 
  };
  //  takes print and write chunks of it to buffer...


/*
 * File Response
 * */

class AsyncJsonResponse: public AsyncAbstractResponse
{
private:

  DynamicJsonBuffer _jsonBuffer;
  JsonObject ** _out;
 // JsonVariant* _root;
  JsonObject* _root; 
  //size_t _position;

  // class ChunkPrint : public Print
  // {
  // public:
  //   ChunkPrint(Print& destination, size_t from, size_t to)
  //     : _destination(destination), _to_skip(from), _to_write(to - from) {}

  //   virtual size_t write(uint8_t c)
  //   {
  //     if (_to_skip > 0) {
  //       _to_skip--;
  //     } else if (_to_write > 0) {
  //       _to_write--;
  //       return _destination.write(c);
  //     }
  //     return 0;
  //   }

  // private:
  //   Print& _destination;
  //   size_t _to_skip;
  //   size_t _to_write;
  // };

public:

  AsyncJsonResponse( JsonObject ** out );
  ~AsyncJsonResponse();

  DynamicJsonBuffer & getBuffer() { return _jsonBuffer; }
  
  bool _sourceValid(){ return 1; }

  void dump();

  //bool _sourceValid() { return 0; }
  size_t _fillBuffer(uint8_t *buf, size_t len);
  //void end() { delete this; }
  JsonObject& createObject() { 
    _root = &_jsonBuffer.createObject(); 
    return *_root; 
  }
};


AsyncJsonResponse::~AsyncJsonResponse()
{
  os_printf("[~AsyncJsonResponse]\n"); 
}

void AsyncJsonResponse::dump()
{
  if (_root) {
  //(_root)->prettyPrintTo(Serial);
  } else {
    os_printf("[dump] nullptr");
  } 
  os_printf("\n"); 
  _contentLength = (_root)->measureLength(); 
}


AsyncJsonResponse::AsyncJsonResponse(JsonObject ** out)
{
  _code = 200;
  _contentType = "text/json";
  _out = out;
  // _path = path;
  // if(!download && !fs.exists(_path) && fs.exists(_path+".gz")){
  //   _path = _path+".gz";
  //   addHeader("Content-Encoding", "gzip");
  // }

  // if(download)
  //   _contentType = "application/octet-stream";
  // else
  //   _setContentType(path);
  // _content = fs.open(_path, "r");
  // _contentLength = _content.size();
}

size_t AsyncJsonResponse::_fillBuffer(uint8_t *data, size_t len)
{
  os_printf("[_fillBuffer] len = %u, heap = %u\n", len, ESP.getFreeHeap() );

  ChunkPrint dest(data, _sentLength, _sentLength + len ); 
  os_printf("[_fillBuffer] chunk created, start [%u] -> end [%u]\n", _sentLength, _sentLength + len);
  //JsonObject& test = **_out; 

  _root->printTo( dest ) ;

  // os_printf("[_fillBuffer] data =");

  // for (size_t i = 0; i < len; i ++) {
  //   os_printf("%u",data[i]);
  // }
  // os_printf("  - END\n"); 


  return len;

}



