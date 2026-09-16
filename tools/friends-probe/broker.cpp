// FRIENDS-002, OpenAI Codex, 2026-09-16. GPL-2.0-or-later.
// FTE signaling interoperability; Windows validates broker TLS certificates.
#include "broker.h"
#include <iostream>

namespace friends {
static void checked(BOOL ok,const char* context) {
    if(!ok)throw std::runtime_error(std::string(context)+" (Windows error "+std::to_string(GetLastError())+")");
}
Broker::~Broker(){close();}
void Broker::open(bool host,const std::string& room) {
    require(!session,"Broker already open");
    require(room.size()==20&&room.substr(0,4)=="EZV-"&&hex(room.substr(4),16),"Invalid room");
    session=WinHttpOpen(L"ezQuake-Vulkan-friends-probe/1",WINHTTP_ACCESS_TYPE_NO_PROXY,nullptr,nullptr,0);
    checked(session!=nullptr,"Create WinHTTP session");
    checked(WinHttpSetTimeouts(session,7000,7000,7000,10000),"Set broker timeouts");
    connection=WinHttpConnect(session,L"master.frag-net.com",27950,0);
    checked(connection!=nullptr,"Connect broker");
    std::wstring path=L"/FTE-Quake/"+std::wstring(room.begin(),room.end());
    HINTERNET request=WinHttpOpenRequest(connection,L"GET",path.c_str(),nullptr,nullptr,nullptr,WINHTTP_FLAG_SECURE);
    checked(request!=nullptr,"Create broker request");
    try {
        checked(WinHttpSetOption(request,WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,nullptr,0),"Enable WebSocket");
        const wchar_t* headers=host?L"Sec-WebSocket-Protocol: rtc_host\r\n":L"Sec-WebSocket-Protocol: rtc_client\r\n";
        checked(WinHttpSendRequest(request,headers,DWORD(-1),nullptr,0,0,0),"Send broker request");
        checked(WinHttpReceiveResponse(request,nullptr),"Receive broker response");
        DWORD code=0,size=sizeof(code);
        checked(WinHttpQueryHeaders(request,WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER,nullptr,&code,&size,nullptr),"Read broker status");
        require(code==101,"Broker rejected WebSocket upgrade");
        socket=WinHttpWebSocketCompleteUpgrade(request,0);
        checked(socket!=nullptr,"Complete broker WebSocket upgrade");
        // Do not depend on a machine-specific registry keepalive setting overnight.
        DWORD keepalive=30000;
        checked(WinHttpSetOption(socket,WINHTTP_OPTION_WEB_SOCKET_KEEPALIVE_INTERVAL,&keepalive,sizeof(keepalive)),"Set WebSocket keepalive");
    } catch(...) {WinHttpCloseHandle(request);throw;}
    WinHttpCloseHandle(request);
    receiver=std::thread(&Broker::receive,this);
}
void Broker::receive() {
    std::vector<char> frame; char buffer[4096];
    while(!stopping) {
        DWORD n=0;WINHTTP_WEB_SOCKET_BUFFER_TYPE type;
        DWORD e=WinHttpWebSocketReceive(socket,buffer,sizeof(buffer),&n,&type);
        if(e){if(!stopping)failure=e;break;}
        if(type==WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE){if(!stopping)failure=ERROR_CONNECTION_ABORTED;break;}
        if((type!=WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE&&type!=WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE)||frame.size()+n>MaxSignal){failure=ERROR_INVALID_DATA;break;}
        frame.insert(frame.end(),buffer,buffer+n);
        if(type==WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE){
            if(!messages.push(std::move(frame))){failure=ERROR_BUFFER_OVERFLOW;break;}
            frame.clear();
        }
    }
}
void Broker::send(uint8_t command,uint16_t peer,const std::string& body) {
    auto bytes=pack(command,peer,body);
    DWORD e=WinHttpWebSocketSend(socket,WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,bytes.data(),DWORD(bytes.size()));
    require(e==NO_ERROR,"Broker send failed");
}
bool Broker::poll(Message& msg) {
    std::vector<char> data;
    if(messages.pop(data)){msg=unpack(data);return true;}
    if(failure)throw std::runtime_error("Broker receive failed (Windows error "+std::to_string(failure.load())+")");
    return false;
}
void Broker::close() {
    stopping=true;
    // Close cancels a pending synchronous receive; don't touch its storage until joined.
    if(socket)WinHttpCloseHandle(socket);
    if(receiver.joinable())receiver.join();
    socket=nullptr;
    if(connection)WinHttpCloseHandle(connection);
    if(session)WinHttpCloseHandle(session);
    connection=session=nullptr;
}
}
