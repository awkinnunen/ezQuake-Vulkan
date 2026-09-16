// OpenAI Codex, GPL-2.0-or-later. Datagram fragmentation, no reliability/ordering layer.
#pragma once
#include "../../tools/friends-probe/protocol.h"
#include <chrono>
#include <map>
#include <algorithm>
namespace friends {
constexpr size_t GamePacketMax=8192, FragmentPayload=1000;
inline std::vector<std::string> fragment(uint32_t sequence,const std::string& data) {
    require(!data.empty()&&data.size()<=GamePacketMax,"Invalid game datagram size");
    std::vector<std::string> out;size_t count=(data.size()+FragmentPayload-1)/FragmentPayload;
    for(size_t n=0;n<count;++n){
        std::string p="QW";for(int i=0;i<4;++i)p+=char(sequence>>(8*i));
        p+=char(n);p+=char(count);p+=char(data.size()&255);p+=char(data.size()>>8);
        p+=data.substr(n*FragmentPayload,FragmentPayload);out.push_back(std::move(p));
    }return out;
}
class Assembly {
    struct Parts {std::string data;uint16_t mask=0;uint8_t count=0;std::chrono::steady_clock::time_point started;};
    std::map<uint32_t,Parts> pending;
public:
    std::string add(const std::string& p) {
        require(p.size()>=11&&p.size()<=1010&&p.substr(0,2)=="QW","Malformed game fragment");
        uint32_t id=0;for(int i=0;i<4;++i)id|=uint32_t(uint8_t(p[2+i]))<<(8*i);
        size_t n=uint8_t(p[6]),count=uint8_t(p[7]),total=uint8_t(p[8])|(size_t(uint8_t(p[9]))<<8);
        require(total>0&&total<=GamePacketMax&&count==(total+FragmentPayload-1)/FragmentPayload&&n<count,"Invalid fragment bounds");
        require(p.size()-10==std::min(FragmentPayload,total-n*FragmentPayload),"Truncated game fragment");
        auto now=std::chrono::steady_clock::now();
        for(auto it=pending.begin();it!=pending.end();)if(now-it->second.started>std::chrono::seconds(2))it=pending.erase(it);else ++it;
        if(!pending.count(id)){
            if(pending.size()>=8)pending.erase(pending.begin());
            Parts s;s.data.resize(total);s.count=uint8_t(count);s.started=now;pending.emplace(id,std::move(s));
        }
        auto& s=pending.at(id);require(s.data.size()==total&&s.count==count,"Conflicting game fragments");
        const uint16_t bit=uint16_t(1u<<n);
        if(s.mask&bit){require(s.data.compare(n*FragmentPayload,p.size()-10,p,10,p.size()-10)==0,"Conflicting duplicate fragment");return {};}
        s.data.replace(n*FragmentPayload,p.size()-10,p,10,p.size()-10);s.mask|=bit;
        if(s.mask==uint16_t((1u<<count)-1)){auto data=std::move(s.data);pending.erase(id);return data;}return {};
    }
};
}
