// OpenAI Codex, GPL-2.0-or-later. Native Unix Starter; no Python/runtime install required.
#include <curl/curl.h>
#include <jansson.h>
#include <unzip.h>
#include <openssl/sha.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <set>
#include <array>
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <sys/stat.h>
#include <sys/wait.h>
#include <spawn.h>
#include <unistd.h>
extern char **environ;
namespace fs=std::filesystem;
static void need(bool ok,const std::string& message){if(!ok)throw std::runtime_error(message);}
static std::string field(json_t* j,const char* key){auto s=json_string_value(json_object_get(j,key));need(s!=nullptr,"Invalid download manifest");return s;}
static std::string sha(const fs::path& p){
    std::ifstream f(p,std::ios::binary);need(bool(f),"Cannot read "+p.string());SHA256_CTX ctx;SHA256_Init(&ctx);
    std::array<char,65536> bytes;while(f.read(bytes.data(),bytes.size())||f.gcount())SHA256_Update(&ctx,bytes.data(),size_t(f.gcount()));need(f.eof(),"Read failed");
    unsigned char digest[32];SHA256_Final(digest,&ctx);std::string result;for(auto b:digest){result+="0123456789abcdef"[b>>4];result+="0123456789abcdef"[b&15];}return result;
}
static fs::path safe(const fs::path& root,const std::string& n){
    need(!n.empty()&&n.front()!='/'&&n.find('\\')==n.npos&&n.find(':')==n.npos&&n.find('\0')==n.npos,"Unsafe archive path");
    fs::path relative(n);for(const auto& part:relative)need(part!=".."&&part!=".","Archive path traversal");
    for(unsigned char c:n)need(c>=32&&c!=127,"Control character in archive path");return root/relative;
}
static void extract(const fs::path& archive,const fs::path& stage,const std::string& kind){
    unzFile z=unzOpen64(archive.c_str());need(z!=nullptr,"Invalid ZIP");
    try{
        std::set<std::string> seen;uint64_t total=0;int code=unzGoToFirstFile(z);
        for(;code==UNZ_OK;code=unzGoToNextFile(z)){
            unz_file_info64 info{};char name[4096];need(unzGetCurrentFileInfo64(z,&info,name,sizeof(name),nullptr,0,nullptr,0)==UNZ_OK&&info.size_filename<sizeof(name),"Invalid ZIP entry");
            std::string n(name,info.size_filename);safe(stage,n);need(seen.insert(n).second,"Duplicate ZIP path");
            total+=info.uncompressed_size;need(total<=4ull*1024*1024*1024,"Archive too large");
            auto type=(info.external_fa>>16)&S_IFMT;need(type==0||type==S_IFREG||type==S_IFDIR,"ZIP links/special files are not allowed");
        }
        need(code==UNZ_END_OF_LIST_OF_FILE,"Invalid ZIP directory");
        for(code=unzGoToFirstFile(z);code==UNZ_OK;code=unzGoToNextFile(z)){
            unz_file_info64 info{};char name[4096];unzGetCurrentFileInfo64(z,&info,name,sizeof(name),nullptr,0,nullptr,0);std::string n(name,info.size_filename);
            if(n.back()=='/')continue;
            if(kind=="shareware"){
                std::string lower=n;std::transform(lower.begin(),lower.end(),lower.begin(),[](unsigned char c){return char(std::tolower(c));});
                if(lower=="id1/pak0.pak")n="id1/pak0.pak";
                else{
                    if(fs::path(lower).extension()!=".txt")continue;
                    n="licenses/quake-shareware/"+fs::path(n).filename().string();
                }
            }else if(kind=="gpl"&&n=="ezquake.exe")continue;
            auto target=safe(stage,n);
            static const std::set<std::string> overlays={"yellow","orange","purple","green","pink","blue","white","cyan","base","red"};
            bool overlay=false;for(auto& color:overlays)if(n=="qw/skins/player_"+color+".png"&&kind=="nongpl")overlay=true;
            need(!fs::exists(target)||overlay,"Upstream package collision: "+n);fs::create_directories(target.parent_path());
            need(unzOpenCurrentFile(z)==UNZ_OK,"ZIP entry open failed");std::ofstream out(target,std::ios::binary|std::ios::trunc);
            char bytes[65536];int count;while((count=unzReadCurrentFile(z,bytes,sizeof(bytes)))>0)out.write(bytes,count);
            int closed=unzCloseCurrentFile(z);out.close();need(count==0&&closed==UNZ_OK&&bool(out),"ZIP extraction/CRC failed");
        }
        unzClose(z);
    }catch(...){unzClose(z);throw;}
}
struct Download{std::ofstream file;uint64_t written=0,limit;};
static size_t receive(char* data,size_t size,size_t count,void* context){auto& d=*static_cast<Download*>(context);size_t bytes=size*count;if(bytes>d.limit-d.written)return 0;d.file.write(data,bytes);d.written+=bytes;return d.file?bytes:0;}
static fs::path download(json_t* p,const fs::path& cache){
    auto name=field(p,"name"),hash=field(p,"sha256"),url=field(p,"url");
    need(name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_.")==name.npos,"Invalid download name");
    need(hash.size()==64&&hash.find_first_not_of("0123456789abcdef")==hash.npos,"Invalid checksum");
    need(url=="https://github.com/nQuake/distfiles/releases/download/snapshot/"+name,"Unexpected download origin");
    auto length=json_integer_value(json_object_get(p,"bytes"));need(length>0&&length<512*1024*1024,"Invalid download size");
    fs::create_directories(cache);auto file=cache/(hash.substr(0,12)+"-"+name);
    if(!fs::exists(file)){
        auto part=file;part+="."+std::to_string(getpid())+".part";need(!fs::exists(part),"Partial download exists");
        std::cout<<"Downloading "<<name<<"..."<<std::endl;Download d{std::ofstream(part,std::ios::binary),0,uint64_t(length)};need(bool(d.file),"Cannot create download");
        CURL* curl=curl_easy_init();need(curl!=nullptr,"Cannot initialize download");
        curl_easy_setopt(curl,CURLOPT_URL,url.c_str());curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,1L);curl_easy_setopt(curl,CURLOPT_MAXREDIRS,8L);
        curl_easy_setopt(curl,CURLOPT_PROTOCOLS_STR,"https");curl_easy_setopt(curl,CURLOPT_REDIR_PROTOCOLS_STR,"https");curl_easy_setopt(curl,CURLOPT_FAILONERROR,1L);
        curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT,30L);curl_easy_setopt(curl,CURLOPT_TIMEOUT,600L);curl_easy_setopt(curl,CURLOPT_USERAGENT,"ezQuake-Vulkan-Unix-Starter/0.3.0");
        curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,receive);curl_easy_setopt(curl,CURLOPT_WRITEDATA,&d);auto result=curl_easy_perform(curl);curl_easy_cleanup(curl);d.file.close();
        need(result==CURLE_OK,"Download failed; rerun setup to retry");need(d.written==uint64_t(length)&&sha(part)==hash,"Download checksum/size mismatch; upstream snapshot may have changed");fs::rename(part,file);
    }
    need(fs::is_regular_file(fs::symlink_status(file))&&fs::file_size(file)==uint64_t(length)&&sha(file)==hash,"Cached download verification failed");return file;
}
static std::set<std::string> pak(const fs::path& file){
    std::ifstream f(file,std::ios::binary);char magic[4];f.read(magic,4);need(f&&memcmp(magic,"PACK",4)==0,"Invalid Quake PAK");
    auto number=[&](){unsigned char b[4];f.read(reinterpret_cast<char*>(b),4);need(bool(f),"Truncated PAK");return uint32_t(b[0])|(uint32_t(b[1])<<8)|(uint32_t(b[2])<<16)|(uint32_t(b[3])<<24);};
    uint64_t size=fs::file_size(file),offset=number(),length=number();need(offset>=12&&length%64==0&&length<=4*1024*1024&&offset+length<=size,"Invalid PAK directory");f.seekg(offset);std::set<std::string> names;
    for(uint64_t i=0;i<length/64;++i){char n[57]={};f.read(n,56);uint64_t start=number(),bytes=number();need(start+bytes<=size,"Invalid PAK entry");names.insert(n);}return names;
}
static void copyTree(const fs::path& source,const fs::path& target){fs::copy(source,target,fs::copy_options::recursive|fs::copy_options::copy_symlinks);}
static int run(const fs::path& exe,const std::vector<std::string>& args){std::string executable=exe.string();std::vector<char*> argv{executable.data()};for(auto& x:args)argv.push_back(const_cast<char*>(x.c_str()));argv.push_back(nullptr);pid_t pid;int error=posix_spawn(&pid,exe.c_str(),nullptr,nullptr,argv.data(),environ);if(error)return error;int status;while(waitpid(pid,&status,0)<0)if(errno!=EINTR)return -1;return WIFEXITED(status)?WEXITSTATUS(status):-1;}
int main(int argc,char** argv){try{
    umask(077);need(curl_global_init(CURL_GLOBAL_DEFAULT)==CURLE_OK,"TLS initialization failed");
    fs::path package,destination,cache,quake;bool yes=false,links=true;
    for(int i=1;i<argc;++i){std::string arg=argv[i];if(arg=="--yes")yes=true;else if(arg=="--no-links")links=false;else{need(i+1<argc,"Missing argument value");std::string value=argv[++i];if(arg=="--package")package=value;else if(arg=="--destination")destination=value;else if(arg=="--cache")cache=value;else if(arg=="--quake")quake=value;else throw std::runtime_error("Unknown setup option");}}
    need(!package.empty(),"--package is required");package=fs::canonical(package);const char* h=getenv("HOME");need(h&&*h,"HOME is missing");fs::path home=h;
#ifdef __APPLE__
    if(destination.empty())destination=home/"Library/Application Support/ezQuake-Vulkan";
    if(cache.empty())cache=home/"Library/Caches/ezQuake-Vulkan/downloads";
#else
    if(destination.empty())destination=(getenv("XDG_DATA_HOME")?fs::path(getenv("XDG_DATA_HOME")):home/".local/share")/"ezquake-vulkan";
    if(cache.empty())cache=(getenv("XDG_CACHE_HOME")?fs::path(getenv("XDG_CACHE_HOME")):home/".cache")/"ezquake-vulkan/downloads";
#endif
    destination=fs::absolute(destination);cache=fs::absolute(cache);
    std::cout<<"ezQuake Vulkan Starter (unofficial nQuake-based distribution)\nDownloads about 112 MB of verified nQuake data. Commercial Quake assets are not included.\nInstall to: "<<destination<<"\nExisting installations and autoexec.cfg are preserved.\n";
    if(!yes){std::cout<<"Continue? [Y/n] "<<std::flush;std::string answer;std::getline(std::cin,answer);if(!answer.empty()&&answer!="y"&&answer!="Y")return 1;}
    need(!fs::exists(fs::symlink_status(destination)),"Destination exists. Choose a new directory with --destination; nothing was replaced.");
    fs::create_directories(destination.parent_path());std::string pattern=(destination.parent_path()/".ezv-install-XXXXXX").string();std::vector<char> name(pattern.begin(),pattern.end());name.push_back(0);need(mkdtemp(name.data())!=nullptr,"Cannot create installation stage");fs::path stage=name.data();
    std::cout<<"Work directory: "<<stage<<std::endl;
    json_error_t error;json_t* lock=json_load_file((package/"downloads.lock.json").c_str(),0,&error);need(lock!=nullptr,"Cannot read download manifest");json_t* packages=json_object_get(lock,"packages");need(json_is_array(packages),"Invalid manifest");
    size_t i;json_t* p;unsigned downloaded=0;json_array_foreach(packages,i,p){auto kind=field(p,"kind");if(kind=="engine")continue;need(kind=="shareware"||kind=="gpl"||kind=="nongpl"||kind=="data","Unknown package kind");extract(download(p,cache),stage,kind);++downloaded;}json_decref(lock);need(downloaded==4,"Incomplete package set");
    need(pak(stage/"id1/pak0.pak").count("progs.dat"),"Shareware game logic missing");
    if(!quake.empty()){
        if(fs::is_directory(quake/"id1"))quake/="id1";std::vector<fs::path> paks;std::set<std::string> entries;
        for(auto& e:fs::directory_iterator(quake)){auto n=e.path().filename().string();if(n.size()>=8&&n.substr(0,3)=="pak"&&n[3]>='1'&&n[3]<='9'&&e.path().extension()==".pak"){auto names=pak(e.path());entries.insert(names.begin(),names.end());paks.push_back(e.path());}}
        need(entries.count("gfx/pop.lmp")&&entries.count("maps/e2m1.bsp"),"No complete registered classic Quake data found");
        for(auto& f:paks){auto to=stage/"id1"/f.filename();fs::copy_file(f,to);need(sha(f)==sha(to),"PAK copy verification failed");}
        auto maps=stage/"id1/gpl_maps.pk3";if(fs::exists(maps))fs::rename(maps,maps.string()+".disabled");
    }
    // Copy only absent profile files. Upstream autoexec.cfg is never edited.
    for(auto& e:fs::recursive_directory_iterator(package/"profiles"))if(e.is_regular_file()){auto to=stage/fs::relative(e.path(),package/"profiles");if(!fs::exists(to)){fs::create_directories(to.parent_path());fs::copy_file(e.path(),to);}}
    auto preset=stage/"ezquake/configs/preset.cfg";need(!fs::exists(preset),"Unexpected upstream preset.cfg");fs::create_directories(preset.parent_path());
    std::ofstream f(preset);f<<"// Starter first-run defaults; nQuake consumes this preset once.\ncfg_save_onquit 1\n";
    std::ifstream graphics(package/"profiles/ezquake/presets/graphics/builtin/Balanced.cfg"),keys(package/"profiles/qw/ezv-wasd.cfg");need(graphics&&keys,"Missing package defaults");f<<graphics.rdbuf()<<'\n'<<keys.rdbuf()<<"\nexec ezv-crosshairs.cfg\ncl_onload menu\n";f.close();need(bool(f),"Cannot write preset");
    copyTree(package/"engine",stage/"engine");fs::copy_file(package/"Start.command",stage/"Start.command");fs::permissions(stage/"Start.command",fs::perms::owner_all);
    fs::copy_file(package/"README.txt",stage/"START-HERE.txt");fs::copy_file(package/"manifest.json",stage/"starter-install.json");
    need(!fs::exists(fs::symlink_status(destination)),"Destination appeared during installation");fs::rename(stage,destination);
#ifdef __APPLE__
    auto exe=destination/"engine/ezQuake.app/Contents/MacOS/ezQuake";
    fs::create_directories(home/"Applications");auto shortcut=home/"Applications/ezQuake Vulkan.app";if(!fs::exists(fs::symlink_status(shortcut)))fs::create_directory_symlink(destination/"engine/ezQuake.app",shortcut);
#else
    auto exe=destination/"engine/ezquake";
#endif
    if(links&&run(exe,{"-friends-register",destination.string()})!=0)std::cout<<"Installed, but URL registration needs attention: use Friends > Invitation links.\n";
    std::cout<<"Installed: "<<destination<<"\nRun Start.command. Your Friends link stays valid until you rotate it.\n";return 0;
}catch(const std::exception& e){std::cerr<<"Setup failed: "<<e.what()<<"\nExisting installations were not overwritten.\n";return 1;}}
