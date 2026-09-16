// FRIENDS-002, OpenAI Codex, 2026-09-16. GPL-2.0-or-later.
// Transport feasibility harness, not a Quake server or production invite implementation.
#include "broker.h"
#include "dtls.h"
#include <juice/juice.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

using namespace friends;
using Clock=std::chrono::steady_clock;
static std::atomic<bool> cancelled{false};
static BOOL WINAPI cancel(DWORD type) {
    if(type==CTRL_C_EVENT||type==CTRL_BREAK_EVENT){cancelled=true;return TRUE;}return FALSE;
}
struct Agent {
    juice_agent_t* ice=nullptr;
    Queue<std::string> candidates,received;
    std::atomic<int> state{JUICE_STATE_DISCONNECTED};
    std::atomic<bool> gathered{false},badPacket{false};
    ~Agent(){if(ice)juice_destroy(ice);}
    void create() {
        juice_config_t c{};c.stun_server_host="master.frag-net.com";c.stun_server_port=27950;c.user_ptr=this;
        c.cb_state_changed=[](juice_agent_t*,juice_state_t state,void* p){static_cast<Agent*>(p)->state=state;};
        c.cb_candidate=[](juice_agent_t*,const char* s,void* p){auto a=static_cast<Agent*>(p);if(strlen(s)>1024)a->badPacket=true;else a->candidates.push(s);};
        c.cb_gathering_done=[](juice_agent_t*,void* p){static_cast<Agent*>(p)->gathered=true;};
        c.cb_recv=[](juice_agent_t*,const char* s,size_t n,void* p){auto a=static_cast<Agent*>(p);if(n>MaxDatagram)a->badPacket=true;else a->received.push(std::string(s,n));};
        ice=juice_create(&c);require(ice!=nullptr,"ICE agent creation failed");
    }
    std::string sdp(const Identity& id) {
        char buffer[JUICE_MAX_SDP_STRING_LEN];require(juice_get_local_description(ice,buffer,sizeof(buffer))==0,"Local ICE description failed");
        std::string fp;for(size_t n=0;n<64;n+=2){if(n)fp+=':';fp+=id.fingerprint.substr(n,2);}
        // Raw SDP (FTE's native ICE signaling form), not JSON/SCTP WebRTC.
        return std::string(buffer)+"a=fingerprint:sha-256 "+fp+"\r\na=ezv-probe:1\r\n";
    }
};
struct Report {
    bool host=false,broker=false,ice=false,dtls=false,admitted=false,pass=false;
    int sent=0,received=0,relayOffers=0;double meanRtt=0,maxRtt=0;
    std::string route="unknown",failure="none";
    void save(const std::string& path) const {
        std::ofstream f(path,std::ios::binary|std::ios::trunc);require(bool(f),"Cannot save report");
        f<<"{\n  \"format\": 1,\n  \"probe\": \"FRIENDS-002\",\n  \"role\": \""<<(host?"host":"join")<<"\",\n"
         <<"  \"brokerConnected\": "<<(broker?"true":"false")<<",\n  \"iceConnected\": "<<(ice?"true":"false")
         <<",\n  \"dtlsPinned\": "<<(dtls?"true":"false")<<",\n  \"admitted\": "<<(admitted?"true":"false")
         <<",\n  \"pass\": "<<(pass?"true":"false")<<",\n  \"sent\": "<<sent<<",\n  \"received\": "<<received
         <<",\n  \"meanRttMs\": "<<meanRtt<<",\n  \"maxRttMs\": "<<maxRtt<<",\n  \"relayOffers\": "<<relayOffers
         <<",\n  \"route\": \""<<route<<"\",\n  \"failureStage\": \""<<failure<<"\"\n}\n";
        require(bool(f),"Report write failed");
    }
};
static void clipboard(const std::string& value) {
    require(OpenClipboard(nullptr)!=0,"Cannot open clipboard");
    HGLOBAL h=GlobalAlloc(GMEM_MOVEABLE,value.size()+1);
    if(!h){CloseClipboard();throw std::runtime_error("Clipboard allocation failed");}
    void* p=GlobalLock(h);
    if(!p){GlobalFree(h);CloseClipboard();throw std::runtime_error("Clipboard lock failed");}
    memcpy(p,value.c_str(),value.size()+1);GlobalUnlock(h);
    if(!EmptyClipboard()||!SetClipboardData(CF_TEXT,h)){GlobalFree(h);CloseClipboard();throw std::runtime_error("Clipboard write failed");}
    CloseClipboard();
}
static std::string readInvite(const std::string& path) {
    std::ifstream f(path,std::ios::binary);require(bool(f),"Cannot read invitation file");
    char buffer[1025];f.read(buffer,sizeof(buffer));require(f.gcount()<1025,"Invitation file too large");
    std::string s(buffer,size_t(f.gcount()));
    while(!s.empty()&&(s.back()=='\r'||s.back()=='\n'))s.pop_back();
    return s;
}
static int cryptoTest() {
    Identity a,b;Dtls server(true,a,b.fingerprint),client(false,b,a.fingerprint);
    auto transfer=[](Dtls& from,Dtls& to){std::string p;while(from.output(p))to.input(p);};
    for(int n=0;n<500&&(!server.ready()||!client.ready());++n){client.tick();transfer(client,server);server.tick();transfer(server,client);}
    require(server.ready()&&client.ready(),"Local DTLS handshake failed");
    std::string payload(1100,'Q');payload[40]='\0';client.write(payload);transfer(client,server);
    require(server.read()==payload,"DTLS binary datagram boundary failed");
    server.write("reply");transfer(server,client);require(client.read()=="reply","DTLS reply failed");
    bool rejected=false;try{client.write(std::string(1101,'x'));}catch(...){rejected=true;}require(rejected,"Oversized app datagram allowed");
    Dtls wrongServer(true,a,b.fingerprint),wrongClient(false,b,std::string(64,'0'));rejected=false;
    try {for(int n=0;n<500;++n){wrongClient.tick();transfer(wrongClient,wrongServer);wrongServer.tick();transfer(wrongServer,wrongClient);}}
    catch(const std::exception&){rejected=true;}
    require(rejected,"Wrong invitation fingerprint accepted");
    // An overnight host must keep its invitation identity: trust is the explicit
    // fingerprint pin, not a CA validity window on this ephemeral certificate.
    Identity overnight;
    require(X509_gmtime_adj(X509_getm_notBefore(overnight.cert.get()),-172800)&&
        X509_gmtime_adj(X509_getm_notAfter(overnight.cert.get()),-86400),"Certificate age fixture failed");
    require(X509_sign(overnight.cert.get(),overnight.key.get(),EVP_sha256())>0,"Certificate age signing failed");
    overnight.fingerprint=certFingerprint(overnight.cert.get());
    Dtls agedServer(true,overnight,b.fingerprint),agedClient(false,b,overnight.fingerprint);
    for(int n=0;n<500&&(!agedServer.ready()||!agedClient.ready());++n){
        agedClient.tick();transfer(agedClient,agedServer);agedServer.tick();transfer(agedServer,agedClient);
    }
    require(agedServer.ready()&&agedClient.ready(),"Pinned overnight identity rejected");
    std::cout<<"PASS: pinned identity remains usable beyond the certificate's one-day CA validity window\n";
    std::cout<<"PASS: DTLS mutual pinning, binary payload, 1100-byte limit, wrong host rejection\n";return 0;
}
static void run(bool host,Invite& invite,const std::string& outFile,bool copy,int seconds,Report& report) {
    Identity identity;if(host){invite={"EZV-"+randomHex(8),randomHex(32),identity.fingerprint};}
    Broker broker;report.failure="broker";broker.open(host,invite.room);report.broker=true;
    std::cout<<"Broker TLS/WebSocket connected. This is a transport probe, not a game.\n";
    Agent agent;std::unique_ptr<Dtls> dtls;uint16_t peer=65535;
    bool havePeer=false,sentOffer=false,receivedOffer=false,announced=false;
    auto start=Clock::now(),nextSend=start,completed=start,nextHeartbeat=start,peerStarted=start;
    bool done=false;std::set<int> replies;std::vector<Clock::time_point> pingTimes(100);
    std::vector<std::string> pings;for(int n=0;n<100;++n)pings.push_back("EZVPING "+std::to_string(n)+" "+randomHex(8)+std::string(n%3==0?1000:20,'P'));
    double rttSum=0;
    while(!cancelled && (seconds==0 || std::chrono::duration<double>(Clock::now()-start).count()<seconds)) {
        // Unlimited hosting means unlimited waiting for a guest, not a stuck handshake.
        require(seconds!=0 || !havePeer || std::chrono::duration<double>(Clock::now()-peerStarted).count()<180,
            "Peer test timed out after connection started");
        Message m;
        while(broker.poll(m)) {
            if(m.command==NameInUse)throw std::runtime_error("Random room collision; restart this probe");
            if(m.command==Greeting&&host&&!announced) {
                require(m.body=="FTE-Quake/"+invite.room,"Broker assigned unexpected room");
                std::ofstream f(outFile,std::ios::binary|std::ios::trunc);require(bool(f),"Cannot write invitation file");
                f<<encode(invite)<<"\n";f.close();require(bool(f),"Invitation write failed");
                if(copy)clipboard(encode(invite));
                std::cout<<"Room ready. Invitation saved to "<<outFile<<(copy?" and copied to clipboard":"")<<".\n";
                announced=true;report.failure="waiting-peer";
            }
            if(m.command==NewPeer) {
                // The prototype admits one test peer only; multiple peers are an engine milestone.
                if(havePeer){continue;}
                auto zero=m.body.find('\0');require(zero!=std::string::npos,"Malformed peer notification");
                std::string relay=m.body.substr(zero+1);
                if(!relay.empty()&&relay[0]){++report.relayOffers;std::cout<<"Existing broker offered relay metadata; this probe does not allocate it.\n";}
                peer=m.peer;havePeer=true;peerStarted=Clock::now();agent.create();
                if(!host){broker.send(Offer,peer,agent.sdp(identity));sentOffer=true;require(juice_gather_candidates(agent.ice)==0,"ICE gathering failed");}
                std::cout<<"Peer discovered. Testing direct ICE route...\n";report.failure="ice";
            }
            if(havePeer&&m.peer==peer&&m.command==Offer) {
                require(!receivedOffer,"Duplicate SDP offer");
                report.failure="identity";
                auto fp=fingerprintFromSDP(m.body);
                require(m.body.find("a=ezv-probe:1\r\n")!=std::string::npos,"Peer is not this probe version");
                if(!host)require(fp==invite.fingerprint,"SDP host fingerprint differs from invitation");
                require(juice_set_remote_description(agent.ice,m.body.c_str())==0,"Remote ICE description rejected");
                dtls=std::make_unique<Dtls>(host,identity,fp);receivedOffer=true;report.failure="ice";
                if(host){broker.send(Offer,peer,agent.sdp(identity));sentOffer=true;require(juice_gather_candidates(agent.ice)==0,"ICE gathering failed");}
            }
            if(havePeer&&m.peer==peer&&m.command==Candidate){
                require(receivedOffer,"Candidate arrived before description");
                require(m.body.size()<1024&&m.body.find('\0')==std::string::npos,"Invalid candidate size");
                int e=juice_add_remote_candidate(agent.ice,m.body.c_str());require(e==0||e==JUICE_ERR_IGNORED,"Candidate rejected");
            }
            if(havePeer&&m.peer==peer&&m.command==PeerLost&&!done)throw std::runtime_error("Peer left before probe completion");
        }
        if(host&&announced&&Clock::now()>=nextHeartbeat){
            broker.send(ServerInfo,65535,"\\hostname\\ezVulkan transport test\\protocol\\28\\maxclients\\0\\clients\\0\\needpass\\1");
            nextHeartbeat=Clock::now()+std::chrono::seconds(30);
        }
        require(!agent.badPacket,"Oversized network packet");
        std::string packet;
        while(agent.candidates.pop(packet)){require(sentOffer,"Internal candidate ordering error");broker.send(Candidate,peer,packet);}
        auto state=agent.state.load();require(state!=JUICE_STATE_FAILED,"ICE could not find a direct route");
        if(dtls&&(state==JUICE_STATE_CONNECTED||state==JUICE_STATE_COMPLETED)){
            if(!report.ice){
                report.ice=true;report.failure="dtls";
                char local[512]{},remote[512]{};
                require(juice_get_selected_candidates(agent.ice,local,sizeof(local),remote,sizeof(remote))==0,"Cannot inspect selected route");
                report.route=(std::string(local).find("typ relay")!=std::string::npos||std::string(remote).find("typ relay")!=std::string::npos)?"relayed":"direct";
                std::cout<<"ICE connected: "<<report.route<<". Authenticating DTLS...\n";
            }
            while(agent.received.pop(packet))dtls->input(packet);
            dtls->tick();
            if(dtls->ready()) {
                if(!report.dtls){report.dtls=true;report.failure="admission";std::cout<<"DTLS certificate verified.\n";}
                while(!(packet=dtls->read()).empty()) {
                    if(host) {
                        const std::string auth="EZVAUTH1 "+invite.key;
                        if(packet.size()==auth.size()&&CRYPTO_memcmp(packet.data(),auth.data(),auth.size())==0){report.admitted=true;report.failure="datagrams";dtls->write("EZVREADY1");}
                        else if(report.admitted&&packet.rfind("EZVPING ",0)==0){
                            std::istringstream input(packet.substr(8));int n=-1;input>>n;require(n>=0&&n<100,"Invalid ping index");
                            if(replies.insert(n).second)++report.received;
                            dtls->write(packet);++report.sent;
                            if(report.received==100&&!done){done=true;completed=Clock::now();}
                        }
                        else throw std::runtime_error("Invalid invitation key or unauthenticated payload");
                    }else if(packet=="EZVREADY1"){report.admitted=true;report.failure="datagrams";}
                    else if(report.admitted){
                        for(int n=0;n<report.sent;++n)if(packet==pings[n]&&replies.insert(n).second){
                            double ms=std::chrono::duration<double,std::milli>(Clock::now()-pingTimes[n]).count();
                            rttSum+=ms;report.maxRtt=std::max(report.maxRtt,ms);++report.received;report.meanRtt=rttSum/report.received;
                            if(report.received==100&&!done){done=true;completed=Clock::now();}break;
                        }
                    }
                }
                auto now=Clock::now();
                if(!host&&now>=nextSend&&!done){
                    if(!report.admitted){dtls->write("EZVAUTH1 "+invite.key);nextSend=now+std::chrono::milliseconds(500);}
                    else if(report.sent<100){pingTimes[report.sent]=now;dtls->write(pings[report.sent]);++report.sent;nextSend=now+std::chrono::milliseconds(25);}
                }
            }
            while(dtls->output(packet))require(juice_send(agent.ice,packet.data(),packet.size())==0,"ICE datagram send failed");
        }
        if(done&&std::chrono::duration<double>(Clock::now()-completed).count()>1)break;
        std::this_thread::sleep_for(std::chrono::milliseconds(havePeer?5:100));
    }
    report.meanRtt=report.received&&!host?rttSum/report.received:0;
    require(!cancelled,"Probe cancelled");
    require(report.received==100,"Probe timed out or lost datagrams; inspect result stage and counts");
    report.pass=true;report.failure="none";
    std::cout<<"PASS: 100 protected datagrams verified; no cross-network claim unless these were different networks.\n";
}
int main(int argc,char** argv) {
    std::cout.setf(std::ios::unitbuf);SetConsoleCtrlHandler(cancel,TRUE);
    Report report;std::string reportFile="result.json";
    try {
        if(argc==2&&std::string(argv[1])=="--self-test"){selfTest();return cryptoTest();}
        bool host=false,join=false,copy=false;int seconds=-1;std::string input,outFile="invitation.txt";
        for(int i=1;i<argc;++i){std::string a=argv[i];
            if(a=="--host")host=true;
            else if(a=="--copy-invite")copy=true;
            else if(a=="--join-file"&&i+1<argc){join=true;input=argv[++i];}
            else if(a=="--invite-out"&&i+1<argc)outFile=argv[++i];
            else if(a=="--report"&&i+1<argc)reportFile=argv[++i];
            else if(a=="--seconds"&&i+1<argc){std::string value=argv[++i];size_t used=0;seconds=std::stoi(value,&used);require(used==value.size()&&(seconds==0||(seconds>=10&&seconds<=600)),"Timeout must be 0 (unlimited wait) or 10..600 seconds");}
            else throw std::runtime_error("Usage: FriendsProbe --host [--copy-invite] | --join-file invitation.txt [--report result.json] [--seconds 0|10..600]");
        }
        require(host!=join,"Choose host or join");report.host=host;
        if(seconds==-1)seconds=host?0:180;
        if(seconds==0)std::cout<<"Waiting without a time limit. Keep this computer awake and online; Ctrl+C stops the probe.\n";
        Invite invite;if(join)invite=decode(readInvite(input));
        juice_set_log_level(JUICE_LOG_LEVEL_NONE); // Library diagnostics may contain IPs/ICE credentials.
        run(host,invite,outFile,copy,seconds,report);report.save(reportFile);return 0;
    } catch(const std::exception& e) {
        std::cerr<<"FAIL: "<<e.what()<<"\n";
        try{report.save(reportFile);}catch(const std::exception& w){std::cerr<<w.what()<<"\n";}
        return 1;
    }
}
