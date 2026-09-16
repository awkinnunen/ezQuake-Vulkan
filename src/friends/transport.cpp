// OpenAI Codex, GPL-2.0-or-later. FTE broker + native ICE/DTLS game transport.
#include "transport.h"
#include "identity.h"
#include "packet.h"
#include "../../tools/friends-probe/broker.h"
#include <juice/juice.h>
#include <thread>
#include <deque>
#include <atomic>
#include <map>
#include <set>
#include <cstdio>
#include <climits>

namespace {
using namespace friends;
using Clock=std::chrono::steady_clock;
struct Datagram {bool server;uint64_t peer;std::string data;};
struct Command {int kind;std::string text;uint64_t value=0;};
struct Peer {
    uint16_t brokerId;uint64_t token;juice_agent_t* ice=nullptr;
    std::unique_ptr<Dtls> dtls;Assembly assembly;
    Queue<std::string> candidates,encrypted;
    std::atomic<int> state{JUICE_STATE_DISCONNECTED};std::atomic<bool> bad{false};
    HANDLE wake;bool description=false,offered=false,admitted=false,signalLost=false;
    uint32_t sequence=1;
    Clock::time_point started=Clock::now(),last=started,nextSend=started;
    Peer(uint16_t id,uint64_t number,HANDLE event):brokerId(id),token(number),wake(event){
        juice_config_t c{};c.stun_server_host="master.frag-net.com";c.stun_server_port=27950;c.user_ptr=this;
        c.cb_state_changed=[](juice_agent_t*,juice_state_t s,void* p){auto x=static_cast<Peer*>(p);x->state=s;SetEvent(x->wake);};
        c.cb_candidate=[](juice_agent_t*,const char* s,void* p){auto x=static_cast<Peer*>(p);try{if(strlen(s)>1024||!x->candidates.push(s))x->bad=true;}catch(...){x->bad=true;}SetEvent(x->wake);};
        c.cb_recv=[](juice_agent_t*,const char* s,size_t n,void* p){auto x=static_cast<Peer*>(p);try{if(!n||n>MaxDatagram||!x->encrypted.push(std::string(s,n)))x->bad=true;}catch(...){x->bad=true;}SetEvent(x->wake);};
        ice=juice_create(&c);require(ice!=nullptr,"ICE initialization failed");
    }
    ~Peer(){if(ice)juice_destroy(ice);}
    std::string sdp(const Identity& identity){
        char b[JUICE_MAX_SDP_STRING_LEN];require(!juice_get_local_description(ice,b,sizeof(b)),"ICE description failed");
        std::string fp;for(size_t n=0;n<64;n+=2){if(n)fp+=':';fp+=identity.fingerprint.substr(n,2);}
        return std::string(b)+"a=fingerprint:sha-256 "+fp+"\r\na=ezv-game:1\r\n";
    }
};
class Service {
    std::mutex mutex;
    std::deque<Command> commands;
    std::deque<Datagram> outgoing,incoming;
    nf_status status{};
    std::set<uint64_t> live;
    int nextRequest=1,activeRequest=0;
    std::string invitation;
    HANDLE wake=CreateEventW(nullptr,FALSE,FALSE,nullptr);
    std::atomic<bool> exiting{false};
    std::thread worker;
    std::unique_ptr<Broker> broker;
    std::unique_ptr<StoredIdentity> stored;
    std::unique_ptr<Identity> guestIdentity;
    Invite guestInvite;
    std::map<uint64_t,std::unique_ptr<Peer>> peers;
    bool host=false,active=false,announced=false,accepting=false,brokerAlive=false;
    uint64_t nextToken=1;
    Clock::time_point heartbeat,started;
    void note(const char* text){std::lock_guard<std::mutex> l(mutex);std::snprintf(status.message,sizeof(status.message),"%s",text);}
    void publish(){
        std::lock_guard<std::mutex> l(mutex);
        status.mode=active?(host?1:2):0;status.request=activeRequest;status.ready=brokerAlive&&announced;status.accepting=accepting;status.peers=0;status.client_peer=0;live.clear();
        for(auto& [id,p]:peers)if(p->admitted){live.insert(p->token);++status.peers;if(!host)status.client_peer=p->token;}
        for(auto it=incoming.begin();it!=incoming.end();){bool live=false;for(auto& item:peers)if(item.second->admitted&&item.second->token==it->peer)live=true;if(!live)it=incoming.erase(it);else ++it;}
        invitation=host&&stored?gameInvite(stored->invitation):"";
    }
    void reset(){
        for(auto& item:peers)goodbye(*item.second);
        peers.clear();broker.reset();stored.reset();guestIdentity.reset();guestInvite={};
        active=announced=accepting=brokerAlive=false;
        std::lock_guard<std::mutex> l(mutex);outgoing.clear();incoming.clear();live.clear();status={};status.request=activeRequest;invitation.clear();
    }
    void goodbye(Peer& p){try{if(p.admitted&&p.dtls&&p.dtls->ready()){p.dtls->write("EZVG1 BYE");std::string data;while(p.dtls->output(data))juice_send(p.ice,data.data(),data.size());}}catch(...){}}
    Identity& identity(){return host?stored->identity:*guestIdentity;}
    void sendOffer(Peer& p){broker->send(Offer,p.brokerId,p.sdp(identity()));p.offered=true;require(!juice_gather_candidates(p.ice),"ICE gather failed");}
    void execute(const Command& c){
        if(c.kind==1||c.kind==2){
            activeRequest=int(c.value);reset();host=c.kind==1;active=true;accepting=host;started=heartbeat=Clock::now();
            note("Preparing encrypted connection...");publish();
            std::string room;
            if(host){stored=std::make_unique<StoredIdentity>();stored->open(c.text);room=stored->invitation.room;}
            else {guestInvite=parseGameInvite(c.text);guestIdentity=std::make_unique<Identity>();room=guestInvite.room;}
            broker=std::make_unique<Broker>();broker->open(host,room);brokerAlive=true;
            note(host?"Registering your saved room...":"Waiting for the host...");
        }else if(c.kind==3){reset();note("Invitations are off.");}
        else if(c.kind==4&&active&&host){accepting=c.value!=0;note(accepting?"Invitations open.":"Invitations closed; current guests can stay.");}
        else if(c.kind==5&&host&&stored){stored->rotate();note("Invitation replaced. Old links cannot admit new guests.");}
        else if(c.kind==6){for(auto it=peers.begin();it!=peers.end();)if(it->second->token==c.value){goodbye(*it->second);it=peers.erase(it);}else ++it;}
        publish();
    }
    void signal(const Message& m){
        if(m.command==NameInUse)throw std::runtime_error("Saved room is already in use. Close the other host or retry later.");
        if(m.command==Greeting&&host){require(m.body=="FTE-Quake/"+stored->invitation.room,"Unexpected broker room");announced=true;note("Ready. Copy invitation to invite friends.");}
        if(m.command==NewPeer){
            if((host&&!accepting)||peers.size()>=15||(!host&&!peers.empty()))return;
            for(auto& item:peers)if(!item.second->signalLost&&item.second->brokerId==m.peer)return;
            require(m.body.find('\0')!=std::string::npos,"Malformed peer notification");
            require(nextToken!=0,"Peer identifier exhausted; restart game");
            auto p=std::make_unique<Peer>(m.peer,nextToken++,wake);
            if(!host)sendOffer(*p);auto token=p->token;peers.emplace(token,std::move(p));
        }
        auto it=std::find_if(peers.begin(),peers.end(),[&](auto& item){return !item.second->signalLost&&item.second->brokerId==m.peer;});if(it==peers.end())return;
        auto& p=*it->second;
        try{
            if(m.command==Offer){
                require(!p.description,"Duplicate connection offer");
                auto fp=fingerprintFromSDP(m.body);require(m.body.find("a=ezv-game:1\r\n")!=std::string::npos,"Peer uses incompatible game transport");
                if(!host)require(fp==guestInvite.fingerprint,"Host identity does not match invitation");
                require(!juice_set_remote_description(p.ice,m.body.c_str()),"Invalid ICE description");
                p.dtls=std::make_unique<Dtls>(host,identity(),fp);p.description=true;
                if(host)sendOffer(p);
            }else if(m.command==Candidate){
                require(p.description&&m.body.size()<1024&&m.body.find('\0')==std::string::npos,"Invalid ICE candidate");
                int e=juice_add_remote_candidate(p.ice,m.body.c_str());require(e==0||e==JUICE_ERR_IGNORED,"ICE candidate rejected");
            }else if(m.command==PeerLost){if(p.admitted)p.signalLost=true;else{peers.erase(it);if(!host)note("The host has disconnected.");}}
        }catch(const std::exception& e){peers.erase(it);note(e.what());}
    }
    void tickPeer(Peer& p){
        auto now=Clock::now();
        require(!p.bad&&p.state!=JUICE_STATE_FAILED,"Peer network connection failed");
        require(p.admitted||(now-p.started)<std::chrono::seconds(45),"Peer connection timed out");
        require(!p.admitted||(now-p.last)<std::chrono::seconds(45),"Peer stopped responding");
        std::string packet;
        while(p.candidates.pop(packet))if(brokerAlive&&!p.signalLost)broker->send(Candidate,p.brokerId,packet);
        if(!p.dtls||(p.state!=JUICE_STATE_CONNECTED&&p.state!=JUICE_STATE_COMPLETED))return;
        while(p.encrypted.pop(packet))p.dtls->input(packet);
        p.dtls->tick();
        if(p.dtls->ready()){
            for(int n=0;n<128&&!(packet=p.dtls->read()).empty();++n){
                if(host&&packet.rfind("EZVG1 AUTH ",0)==0){
                    auto auth="EZVG1 AUTH "+stored->invitation.key;
                    require(p.admitted||(accepting&&packet.size()==auth.size()&&!CRYPTO_memcmp(packet.data(),auth.data(),auth.size())),"Invitation invalid or closed");
                    p.admitted=true;p.dtls->write("EZVG1 OK");note("Guest admitted.");
                }else if(!host&&packet=="EZVG1 OK"){p.admitted=true;announced=true;note("Connected through an encrypted Friends link.");}
                else{
                    require(p.admitted,"Payload received before admission");
                    require(packet!="EZVG1 BYE","Peer left the Friends game");
                    if(packet=="EZVG1 PING")p.dtls->write("EZVG1 PONG");
                    else if(packet!="EZVG1 PONG"){
                        auto data=p.assembly.add(packet);
                        if(!data.empty()){std::lock_guard<std::mutex> l(mutex);if(incoming.size()<512)incoming.push_back({host,p.token,std::move(data)});}
                    }
                }
                p.last=now;
            }
            if(now>=p.nextSend){
                if(!host&&!p.admitted){p.dtls->write("EZVG1 AUTH "+guestInvite.key);p.nextSend=now+std::chrono::milliseconds(500);}
                else if(p.admitted){p.dtls->write("EZVG1 PING");p.nextSend=now+std::chrono::seconds(10);}
            }
        }
        while(p.dtls->output(packet))require(!juice_send(p.ice,packet.data(),packet.size()),"ICE send failed");
    }
    void loop(){
        juice_set_log_level(JUICE_LOG_LEVEL_NONE);
        while(!exiting){
            try{
                std::deque<Command> work;{std::lock_guard<std::mutex> l(mutex);work.swap(commands);}
                for(auto& c:work)execute(c);
                if(active){
                    if(brokerAlive){
                        try{
                            Message m;for(int n=0;n<64&&broker->poll(m);++n)signal(m);
                            if(host&&announced&&Clock::now()>=heartbeat){
                                broker->send(ServerInfo,65535,"\\hostname\\ezQuake Friends\\protocol\\28\\maxclients\\16\\needpass\\1");heartbeat=Clock::now()+std::chrono::seconds(30);
                            }
                        }catch(const std::exception& e){brokerAlive=false;accepting=false;note(e.what());broker.reset();}
                    }
                    if(!host&&!announced&&Clock::now()-started>std::chrono::seconds(60))throw std::runtime_error("Could not join. Host may be offline or these networks cannot connect.");
                    std::deque<Datagram> packets;{std::lock_guard<std::mutex> l(mutex);packets.swap(outgoing);}
                    for(auto& [id,p]:peers){
                        try{
                            for(auto& packet:packets)if(packet.server==host&&packet.peer==p->token&&p->admitted)
                                for(auto& piece:fragment(p->sequence++,packet.data))p->dtls->write(piece);
                            tickPeer(*p);
                        }catch(const std::exception& e){p->bad=true;note(e.what());}
                    }
                    for(auto it=peers.begin();it!=peers.end();)if(it->second->bad)it=peers.erase(it);else ++it;
                    publish();
                }
            }catch(const std::exception& e){reset();note(e.what());}
            WaitForSingleObject(wake,active?5:INFINITE);
        }
        reset();
    }
public:
    Service(){require(wake!=nullptr,"Friends worker event failed");worker=std::thread(&Service::loop,this);}
    ~Service(){exiting=true;SetEvent(wake);if(worker.joinable())worker.join();CloseHandle(wake);}
    int command(Command c){std::lock_guard<std::mutex> l(mutex);if(commands.size()>=16||nextRequest==INT_MAX)return 0;int result=1;if(c.kind==1||c.kind==2){result=nextRequest++;c.value=result;}commands.push_back(std::move(c));SetEvent(wake);return result;}
    int alive(uint64_t peer){std::lock_guard<std::mutex> l(mutex);return live.count(peer)!=0;}
    void get(nf_status* out){std::lock_guard<std::mutex> l(mutex);*out=status;}
    int invite(char* out,int size){std::lock_guard<std::mutex> l(mutex);if(!status.ready||invitation.empty()||size<=int(invitation.size()))return 0;memcpy(out,invitation.c_str(),invitation.size()+1);return 1;}
    void send(bool server,uint64_t peer,const void* data,int size){if(!peer||size<=0||size>int(GamePacketMax))return;std::lock_guard<std::mutex> l(mutex);if(outgoing.size()<512)outgoing.push_back({server,peer,std::string(static_cast<const char*>(data),size)});SetEvent(wake);}
    int receive(bool server,uint64_t* peer,void* data,int size){
        std::lock_guard<std::mutex> l(mutex);
        for(auto it=incoming.begin();it!=incoming.end();){
            if(it->server!=server){++it;continue;}
            if(!live.count(it->peer)){++it;continue;}
            if(it->data.size()>size_t(size)){it=incoming.erase(it);continue;}
            int n=int(it->data.size());*peer=it->peer;memcpy(data,it->data.data(),n);incoming.erase(it);return n;
        }return 0;
    }
};
std::unique_ptr<Service> service;
Service& get(){if(!service)service=std::make_unique<Service>();return *service;}
}
extern "C" {
int NF_Host(const char* path){try{return path&&get().command({1,path});}catch(...){return 0;}}
int NF_Join(const char* invite){try{parseGameInvite(invite?invite:"");return get().command({2,invite});}catch(...){return 0;}}
int NF_ValidateInvite(const char* s){try{parseGameInvite(s?s:"");return 1;}catch(...){return 0;}}
void NF_Stop(){if(service)service->command({3,{}});}
void NF_Shutdown(){service.reset();}
void NF_Accept(int enable){if(service)service->command({4,{},uint64_t(enable!=0)});}
void NF_Rotate(){if(service)service->command({5,{}});}
void NF_Drop(uint64_t peer){if(service)service->command({6,{},peer});}
int NF_Alive(uint64_t peer){return service?service->alive(peer):0;}
void NF_Status(nf_status* out){if(service)service->get(out);else *out={};}
int NF_Invitation(char* out,int size){return service?service->invite(out,size):0;}
int NF_Receive(int server,uint64_t* peer,void* data,int size){return service?service->receive(server!=0,peer,data,size):0;}
void NF_Send(int server,uint64_t peer,const void* data,int size){if(service)service->send(server!=0,peer,data,size);}
}
