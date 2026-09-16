// OpenAI Codex, GPL-2.0-or-later. Persistence, bounds and actual encrypted UDP tests.
#include "identity.h"
#include "packet.h"
#include "transport.h"
#include <iostream>
#include <thread>
#include <set>
using namespace friends;
void tests(const std::string& dir){
 const Invite sample{"EZV-0123456789ABCDEF",std::string(64,'A'),std::string(64,'B')};
 auto original=gameInvite(sample),shell=original;shell.insert(shell.find('?'),"/");
 require(parseGameInvite(shell).key==sample.key&&parseGameInvite(original).key==sample.key,"Windows URL spelling rejected");
 for(const auto& bad:{shell+";quit",shell+"%00",std::string("ezquake-vulkan://join//?")+original.substr(original.find('?')+1),encode(sample)}){
  bool rejected=false;try{parseGameInvite(bad);}catch(...){rejected=true;}require(rejected,"Unsafe game URL accepted");
 }
 for(auto n:{1,999,1000,1001,1450,2900,8192}){
  std::string data(n,'\0');for(int i=0;i<n;++i)data[i]=char(i*37);
  auto pieces=fragment(n,data);Assembly a;std::string result;
  for(auto it=pieces.rbegin();it!=pieces.rend();++it){auto r=a.add(*it);if(!r.empty())result=r;}
  require(result==data,"Reordered binary datagram changed");
 }
 auto pieces=fragment(7,std::string(1450,'x'));Assembly a;
 require(a.add(pieces[0]).empty()&&a.add(pieces[0]).empty(),"Duplicate prematurely completed");
 require(a.add(pieces[1])==std::string(1450,'x'),"Duplicate disrupted reassembly");
 for(auto bad:{std::string(),std::string("QW"),pieces[0].substr(0,100),std::string(1011,'x')}){
  bool rejected=false;try{Assembly x;x.add(bad);}catch(...){rejected=true;}require(rejected,"Malformed packet accepted");
 }
 bool rejected=false;try{fragment(1,std::string(8193,'x'));}catch(...){rejected=true;}require(rejected,"Oversize accepted");
 std::string first,second;auto path=dir+"/identity";
 {StoredIdentity id;id.open(path);first=gameInvite(id.invitation);
  bool locked=false;try{StoredIdentity other;other.open(path);}catch(...){locked=true;}require(locked,"Identity has no exclusive lock");}
 {StoredIdentity id;id.open(path);require(gameInvite(id.invitation)==first,"Invitation changed on restart");id.rotate();second=gameInvite(id.invitation);require(second!=first,"Rotation unchanged");require(parseGameInvite(second).room==parseGameInvite(first).room,"Rotation changed room");}
 {StoredIdentity id;id.open(path);require(gameInvite(id.invitation)==second,"Rotation not persisted");}
 std::ifstream f(path,std::ios::binary);std::string raw((std::istreambuf_iterator<char>(f)),{});require(raw.find(parseGameInvite(second).key)==std::string::npos,"Plaintext secret on disk");f.close();
 {std::ofstream f(path,std::ios::binary|std::ios::trunc);f<<"corrupt";}
 rejected=false;try{StoredIdentity id;id.open(path);}catch(...){rejected=true;}require(rejected,"Corrupt identity silently replaced");
 std::cout<<"PASS: binary fragmentation, malformed bounds, persistence, locking, rotation, DPAPI, corruption\n";
}
int main(int argc,char** argv){
 try{
  require(argc>=3,"Arguments: --unit directory / --host directory / --join invitation-file");
  std::string mode=argv[1],path=argv[2];
  if(mode=="--unit"){tests(path);return 0;}
  bool host=mode=="--host";std::filesystem::create_directories(host?path:std::filesystem::path(path).parent_path().string());
  if(host)require(NF_Host((path+"/identity").c_str()),"Start host failed");
  else {std::ifstream f(path);std::string invite((std::istreambuf_iterator<char>(f)),{});require(NF_Join(invite.c_str()),"Start join failed");}
  auto end=std::chrono::steady_clock::now()+std::chrono::seconds(host?120:65),next=std::chrono::steady_clock::now();
  nf_status s{};std::string message;int sent=0,received=0;bool exported=false;std::set<uint64_t> guests;
  std::string payload(1450,'\0');for(int i=0;i<1450;++i)payload[i]=char(i*31);
  while(std::chrono::steady_clock::now()<end){
   NF_Status(&s);if(s.message!=message){message=s.message;std::cout<<message<<std::endl;}
   if(host){
    char invite[512];if(!exported&&NF_Invitation(invite,sizeof(invite))){std::ofstream(path+"/invite.txt")<<invite;exported=true;}
    for(auto command:{"close","open","rotate","stop"})if(std::filesystem::exists(path+"/"+command)){
     std::filesystem::remove(path+"/"+command);
     if(std::string(command)=="close")NF_Accept(0);else if(std::string(command)=="open")NF_Accept(1);
     else if(std::string(command)=="rotate"){NF_Rotate();exported=false;}else end=std::chrono::steady_clock::now();
    }
   }
   if(!host&&s.client_peer&&sent<100&&std::chrono::steady_clock::now()>=next){NF_Send(0,s.client_peer,payload.data(),int(payload.size()));++sent;next=std::chrono::steady_clock::now()+std::chrono::milliseconds(50);}
   char buffer[8192];uint64_t peer;int n;
   while((n=NF_Receive(host,&peer,buffer,sizeof(buffer)))>0){require(std::string(buffer,n)==payload,"Game packet changed");if(host){guests.insert(peer);NF_Send(1,peer,buffer,n);}else ++received;}
   if(!host&&received==100)break;
   std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  NF_Shutdown();std::cout<<"RESULT sent="<<sent<<" received="<<received<<" unique_guests="<<guests.size()<<std::endl;
  return host||received==100?0:2;
 }catch(const std::exception& e){NF_Shutdown();std::cerr<<e.what()<<std::endl;return 1;}
}
