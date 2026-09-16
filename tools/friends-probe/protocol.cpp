// FRIENDS-002, OpenAI Codex, 2026-09-16. GPL-2.0-or-later.
#include "protocol.h"
#include <algorithm>
#include <iostream>

namespace friends {
void require(bool condition,const char* error) { if(!condition) throw std::runtime_error(error); }
bool hex(const std::string& s,size_t size) {
    return s.size()==size && std::all_of(s.begin(),s.end(),[](char c){return (c>='0'&&c<='9')||(c>='A'&&c<='F');});
}
std::string encode(const Invite& i) {
    require(i.room.size()==20 && i.room.substr(0,4)=="EZV-" && hex(i.room.substr(4),16),"Invalid probe room");
    require(hex(i.key,64) && hex(i.fingerprint,64),"Invalid invitation key or fingerprint");
    return "ezquake-vulkan://probe?v=1&service=fragnet&room="+i.room+"#key="+i.key+"&fp="+i.fingerprint;
}
Invite decode(const std::string& text) {
    const std::string prefix="ezquake-vulkan://probe?v=1&service=fragnet&room=";
    require(text.size()==prefix.size()+20+5+64+4+64,"Invalid invitation length");
    require(text.compare(0,prefix.size(),prefix)==0,"Unsupported invitation format");
    size_t p=prefix.size(); Invite i;
    i.room=text.substr(p,20); p+=20;
    require(text.substr(p,5)=="#key=","Missing invitation key");p+=5;
    i.key=text.substr(p,64);p+=64;
    require(text.substr(p,4)=="&fp=","Missing host fingerprint");p+=4;
    i.fingerprint=text.substr(p,64);
    require(encode(i)==text,"Noncanonical invitation");return i;
}
std::vector<char> pack(uint8_t command,uint16_t peer,const std::string& body) {
    require(body.size()<=MaxSignal-3,"Oversized signaling message");
    std::vector<char> out{char(command),char(peer&255),char(peer>>8)};
    out.insert(out.end(),body.begin(),body.end());return out;
}
Message unpack(const std::vector<char>& v) {
    require(!v.empty() && v.size()<=MaxSignal,"Invalid signaling size");
    // FTE emits NAMEINUSE without the usual two-byte peer field.
    if(uint8_t(v[0])==NameInUse) return {NameInUse,65535,{}};
    require(v.size()>=3,"Truncated signaling header");
    return {uint8_t(v[0]),uint16_t(uint8_t(v[1])|(uint16_t(uint8_t(v[2]))<<8)),std::string(v.begin()+3,v.end())};
}
std::string fingerprintFromSDP(const std::string& text) {
    require(text.size()<=MaxSignal && text.find('\0')==std::string::npos,"Invalid SDP");
    const std::string prefix="a=fingerprint:sha-256 ";
    auto p=text.find(prefix);
    require(p!=std::string::npos && (p==0||text[p-1]=='\n'),"Missing DTLS fingerprint");
    require(text.find(prefix,p+1)==std::string::npos,"Duplicate DTLS fingerprint");
    p+=prefix.size();auto e=text.find_first_of("\r\n",p);
    std::string s=text.substr(p,e-p),out;
    require(s.size()==95,"Invalid SDP fingerprint size");
    for(size_t n=0;n<s.size();++n) {
        if(n%3==2)require(s[n]==':',"Invalid fingerprint separator");
        else out+=s[n];
    }
    require(hex(out,64),"Invalid fingerprint bytes");return out;
}
int selfTest() {
    Invite i{"EZV-0123456789ABCDEF",std::string(64,'A'),std::string(64,'B')};
    auto s=encode(i);require(decode(s).key==i.key,"Round trip failed");
    int rejected=0;
    std::vector<std::string> invalid{s+"\n",s+";quit",s+"&v=2",s.substr(1),s+"%00",std::string(20000,'X')};
    auto bad=s;bad[bad.find("#key=")+5]='g';invalid.push_back(bad);
    bad=s;bad[bad.find("EZV-")]='x';invalid.push_back(bad);
    bad=s;bad.replace(bad.find("service=fragnet"),15,"service=evilnet");invalid.push_back(bad);
    for(const auto& v:invalid){try{decode(v);}catch(const std::exception&){++rejected;}}
    require(rejected==int(invalid.size()),"Malformed invitation accepted");
    const std::string binary("one\0two\0",8);
    auto m=unpack(pack(NewPeer,513,binary));require(m.peer==513&&m.body==binary,"Binary protocol round trip failed");
    require(unpack({char(NameInUse)}).command==NameInUse,"FTE room conflict rejected");
    rejected=0;
    for(auto v:std::vector<std::vector<char>>{{},{1},{1,2},std::vector<char>(MaxSignal+1,0)}){
        try{unpack(v);}catch(const std::exception&){++rejected;}
    }
    require(rejected==4,"Truncated/oversized frame accepted");
    std::string fp;for(int n=0;n<32;++n){if(n)fp+=':';fp+="AB";}
    std::string expected;for(int n=0;n<32;++n)expected+="AB";
    require(fingerprintFromSDP("v=0\r\na=fingerprint:sha-256 "+fp+"\r\n")==expected,"Fingerprint parsing failed");
    rejected=0;
    for(const auto& v:std::vector<std::string>{"v=0\n","a=fingerprint:sha-256 XX\n","a=fingerprint:sha-256 "+fp+"\na=fingerprint:sha-256 "+fp+"\n"}){
        try{fingerprintFromSDP(v);}catch(const std::exception&){++rejected;}
    }
    require(rejected==3,"Bad certificate identity accepted");
    Queue<int> q;for(int n=0;n<128;++n)require(q.push(n),"Premature queue overflow");
    require(!q.push(129),"Unbounded queue");
    std::cout<<"PASS: invitation validation, binary FTE frames, fingerprint parsing, bounded queues\n";
    return 0;
}
}
