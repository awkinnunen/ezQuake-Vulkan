// OpenAI Codex, GPL-2.0-or-later. Native Unix WebSocket adapter; unchanged FTE wire format.
#include "broker.h"
#include <ixwebsocket/IXWebSocket.h>
#include <condition_variable>
#include <chrono>
namespace friends {
struct Broker::Native {
    ix::WebSocket websocket;
    std::mutex mutex;
    std::condition_variable changed;
    bool opened=false,failed=false;
    Queue<std::vector<char>> messages;
};
Broker::Broker()=default;
Broker::~Broker(){close();}
void Broker::open(bool host,const std::string& room){
    require(!native,"Broker already open");
    require(room.size()==20&&room.substr(0,4)=="EZV-"&&hex(room.substr(4),16),"Invalid room");
    native=std::make_unique<Native>();auto n=native.get();
    n->websocket.setUrl("wss://master.frag-net.com:27950/FTE-Quake/"+room);
    n->websocket.addSubProtocol(host?"rtc_host":"rtc_client");
    n->websocket.disableAutomaticReconnection();
    n->websocket.setHandshakeTimeout(10);
    n->websocket.setPingInterval(30);
    n->websocket.setPerMessageDeflateOptions(ix::WebSocketPerMessageDeflateOptions(false));
    n->websocket.setOnMessageCallback([n](const ix::WebSocketMessagePtr& m){
        std::lock_guard<std::mutex> lock(n->mutex);
        if(m->type==ix::WebSocketMessageType::Open)n->opened=true;
        else if(m->type==ix::WebSocketMessageType::Error||m->type==ix::WebSocketMessageType::Close)n->failed=true;
        else if(m->type==ix::WebSocketMessageType::Message){
            if(!m->binary||m->str.empty()||m->str.size()>MaxSignal||!n->messages.push(std::vector<char>(m->str.begin(),m->str.end())))n->failed=true;
        }
        n->changed.notify_all();
    });
    n->websocket.start();
    std::unique_lock<std::mutex> lock(n->mutex);
    require(n->changed.wait_for(lock,std::chrono::seconds(12),[n]{return n->opened||n->failed;})&&n->opened&&!n->failed,"Broker TLS/WebSocket connection failed");
}
void Broker::send(uint8_t command,uint16_t peer,const std::string& body){
    require(native!=nullptr,"Broker is closed");auto data=pack(command,peer,body);
    require(native->websocket.sendBinary(std::string(data.begin(),data.end())).success,"Broker send failed");
}
bool Broker::poll(Message& msg){
    require(native!=nullptr,"Broker is closed");std::vector<char> data;
    if(native->messages.pop(data)){msg=unpack(data);return true;}
    std::lock_guard<std::mutex> lock(native->mutex);require(!native->failed,"Broker connection closed or invalid message");return false;
}
void Broker::close(){if(native){native->websocket.stop();native.reset();}}
}
