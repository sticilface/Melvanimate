

#include "UDP_broadcast.h"

#include "WiFiUdp.h"
#include "lwip/opt.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
#include "lwip/igmp.h"
#include "lwip/mem.h"
#include "include/UdpContext.h"

#define UDP_PING_TIMEOUT 30000
#define UDP_TASK_TIMEOUT 5000
#define UDP_STALE_TIMEOUT 60000


static const IPAddress MELVANIMATE_MULTICAST_ADDR(224, 0, 0, 251);

void UDP_broadcast::begin(const char * host, uint16_t port) {

        _host = host;
        _port = port;

        _gotIPHandler = WiFi.onStationModeGotIP([this](const WiFiEventStationModeGotIP& event){
                _restart();
                DebugUDPf("[UDP_broadcast::_gotIPHandler] Finished.");
        });

        _disconnectedHandler = WiFi.onStationModeDisconnected([this](const WiFiEventStationModeDisconnected& event) {
                _restart();
                DebugUDPf("[UDP_broadcast::_disconnectedHandler] Finished.");
        });

        _listen();
        DebugUDPf("[UDP_broadcast::begin] Finished.");
}

void UDP_broadcast::loop() {
return;
        // if (millis() - _lastmessage > 10000) {
        //         _sendRequest(PONG);
        //         _lastmessage = millis();
        // }
#ifdef UDP_TEST_SENDER
      _test_sender();
#endif

        uint32_t earliest_last_PING = (**devices.begin()).lastseen;

        if (millis() - _checkTimeOut > UDP_TASK_TIMEOUT) {
                // purge stale entries
                for (UDPList::iterator it=devices.begin(); it!=devices.end(); ++it) {

                        UDP_item & item = **it;

                        if (item.lastseen < earliest_last_PING) {
                                earliest_last_PING = item.lastseen;
                        }

                        if (millis() - item.lastseen > UDP_STALE_TIMEOUT) {

                                //  std::delete me from the list.....
                                DebugUDPf("[UDP_broadcast::loop] Removed %s (%u.%u.%u.%u) not seen for %u\n", item.name.get(), item.IP[0],item.IP[1],item.IP[2],item.IP[3],item.lastseen );
                                //DebugUDPf("     NEED TO ADD DELETE \n");
                                it = devices.erase(it);
                        }
                }

                // get earliest last PING...

                if (earliest_last_PING > UDP_PING_TIMEOUT) {
                        DebugUDPf("[UDP_broadcast::loop] Sending PING...........");
                        _sendRequest(PING);
                }

                //only run the loop every so many seconds...
                _checkTimeOut = millis();
        }

//  send replies with delay
        if (_sendPong && _sendPong < millis()) {
                _sendRequest(PONG);
                _sendPong = 0;
        }


}

void UDP_broadcast::_restart() {

        if (_conn) {
                _conn->unref();
                _conn = nullptr;
        }

        _listen();
        DebugUDPf("[UDP_broadcast::_restart] Finished.");

}

void UDP_broadcast::setHost(const char * host ) {
        _host = host;
}

bool UDP_broadcast::_listen() {
        DebugUDPf("[UDP_broadcast::_listen] Called.");

        if (!_conn) {
                uint32_t ourIp = _getOurIp();
                if(ourIp == 0) {
                        return false;
                }

                ip_addr_t ifaddr;
                ifaddr.addr = ourIp;
                ip_addr_t multicast_addr;
                multicast_addr.addr = (uint32_t) MELVANIMATE_MULTICAST_ADDR;

                if (igmp_joingroup(&ifaddr, &multicast_addr)!= ERR_OK) {
                        return false;
                }

                _conn = new UdpContext;
                _conn->ref();

                if (!_conn->listen(*IP_ADDR_ANY, _port)) {
                        return false;
                }
                _conn->setMulticastInterface(ifaddr);
                _conn->setMulticastTTL(1);
                _conn->onRx(std::bind(&UDP_broadcast::_update, this));
                _conn->connect(multicast_addr, _port);
        }
        DebugUDPf("[UDP_broadcast::_listen] Finsished.");

        return true;

}

uint32_t UDP_broadcast::_getOurIp(){
        IPAddress IP {0,0,0,0};
        WiFiMode mode = WiFi.getMode();
        if(mode == WIFI_STA || mode ==  WIFI_AP_STA) {
                IP = WiFi.localIP();
                return (uint32_t)IP;
        } else if (mode == WIFI_AP) {
                IP = WiFi.softAPIP();
                return (uint32_t)IP;
        } else {
                return 0;
        }
}

void UDP_broadcast::_update() {

        if (!_conn || !_conn->next()) {
                return;
        }
        _parsePacket();
}

void UDP_broadcast::_parsePacket() {

        IPAddress IP;

        size_t size = _conn->getSize();

        UDP_REQUEST_TYPE method = static_cast<UDP_REQUEST_TYPE>(_conn->read());

        char tmp[2];
        tmp[0] = _conn->read();
        tmp[1] = _conn->read();

        uint16_t port = ((uint16_t)tmp[1] << 8) | tmp[0];

        for (uint8_t i = 0; i < 4; i++)  {
                IP[i] = _conn->read();
        }
        uint8_t host_len = _conn->read();

        //char * buf = new char[host_len+1];

        std::unique_ptr<char[]> buf(new char[host_len+1]);

        _conn->read(     buf.get(),host_len);

        buf[host_len] = '\0';

        _conn->flush();

        DebugUDPf("UDP PACKET RECIEVED %s (%u.%u.%u.%u:%u) %s\n", (method == PING) ? "PING" : "PONG",IP[0], IP[1], IP[2], IP[3], port, buf.get());

        _addToList(IP, std::move(buf) );

        //delete[] buf;

        if (method == PING) {
                _sendPong = millis() + random(0,50);
        }
}

void UDP_broadcast::_sendRequest(UDP_REQUEST_TYPE method) {

        if (!_conn) {
                return;
        }

        IPAddress IP = WiFi.localIP();

        const char ip[4] = { IP[0],IP[1],IP[2],IP[3] };
        _conn->flush();
        _conn->append(reinterpret_cast<const char*>(&method), 1);
        _conn->append(reinterpret_cast<const char*>(&_port), 2);
        _conn->append( ip, 4);

        if (_host) {
                //DebugUDPf("[UDP_broadcast::_sendRequest] host = %s\n", _host);
                uint8_t host_len = strlen(_host);
                _conn->append(reinterpret_cast<const char*>(&host_len), 1);
                _conn->append(_host, strlen(_host) + 1);
        } else {
                //DebugUDPf("[UDP_broadcast::_sendRequest] No Host\n");
                _conn->append("No Host", 7);
        }


        _conn->send();

}

void UDP_broadcast::_addToList(IPAddress IP, std::unique_ptr<char[]>(ID) ) {

        if (!ID) {
                return;
        }

        if (strlen(ID.get()) > 33 ) {
                return;
        }

        for (UDPList::iterator it=devices.begin(); it!=devices.end(); ++it) {
                //UDP_item & item = *(*it).get();
                UDP_item & item = **it;

                if (IP == item.IP) {
                        if ( strcmp(ID.get(), item.name.get() )  )  {
                                DebugUDPf("[UDP_broadcast::_addToList] name different reassigning %s->%s", item.name.get(), ID.get());
                                std::unique_ptr<char[]> p( new char[strlen(ID.get()) + 1]  );
                                item.name = std::move(p);
                                strcpy(  item.name.get(), ID.get() );
                        }
                        item.lastseen = millis(); //  last seen time set whenever packet recieved....
                        return;
                }
        }

        devices.push_back( std::unique_ptr<UDP_item> (new UDP_item(IP, ID.get() ) )  );

}

uint8_t UDP_broadcast::count() {
        return devices.size();
}

void UDP_broadcast::addJson(JsonArray & root) {

        for (UDPList::iterator it=devices.begin(); it!=devices.end(); ++it) {
                UDP_item & udp_item = **it;

                JsonObject & item = root.createNestedObject();

                JsonArray & IP = item.createNestedArray("IP");
                IP.add(udp_item.IP[0]);
                IP.add(udp_item.IP[1]);
                IP.add(udp_item.IP[2]);
                IP.add(udp_item.IP[3]);

                item["name"] = udp_item.name.get();

        }


}


#ifdef UDP_TEST_SENDER

void UDP_broadcast::_test_sender() {
  static uint32_t timeout = 0;
  static uint16_t number = 0;
  UDP_REQUEST_TYPE method = PING;

  if (millis() - timeout > 5000) {

    timeout = millis();

    if (!_conn) {
            return;
    }

    char buf[32] = {'\0'};

    char * end = strcpy(buf, "Test Sender ");

    snprintf( end, 5, "%u", number++);

    const char ip[4] = { 192, 168, 1, (uint8_t)random (1,255) };

    _conn->flush();
    _conn->append(reinterpret_cast<const char*>(&method), 1);
    _conn->append(reinterpret_cast<const char*>(&_port), 2);
    _conn->append( ip, 4);

    if (_host) {
            //DebugUDPf("[UDP_broadcast::_sendRequest] host = %s\n", _host);
            uint8_t host_len = strlen(_host);
            _conn->append(reinterpret_cast<const char*>(&host_len), 1);
            _conn->append(_host, strlen(_host) + 1);
    } else {
            //DebugUDPf("[UDP_broadcast::_sendRequest] No Host\n");
            _conn->append("No Host", 7);
    }


    _conn->send();

  }


}

#endif
