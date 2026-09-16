// OpenAI Codex, GPL-2.0-or-later. Validated URLs over private Unix sockets.
#include "platform.h"
#include "transport.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <spawn.h>
extern char **environ;
#include <fcntl.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <limits.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
extern "C" int FriendsMac_Register(int enable);
extern "C" int FriendsMac_Registered(void);
#endif
namespace {
int inbox=-1,guard=-1;
std::string inboxPath;
std::string canonical(const char* value){char path[PATH_MAX];return value&&realpath(value,path)?path:"";}
std::string executable(){
    if(const char* app=getenv("APPIMAGE"))return canonical(app);
    char path[PATH_MAX];
#ifdef __APPLE__
    uint32_t n=sizeof(path);if(_NSGetExecutablePath(path,&n))return "";return canonical(path);
#else
    ssize_t n=readlink("/proc/self/exe",path,sizeof(path)-1);if(n<=0)return "";path[n]=0;return path;
#endif
}
std::string home(){const char* h=getenv("HOME");return h?h:"";}
std::string config(){
#ifdef __APPLE__
    return home()+"/Library/Application Support/ezQuake-Vulkan";
#else
    const char* x=getenv("XDG_CONFIG_HOME");return (x&&*x?std::string(x):home()+"/.config")+"/ezquake-vulkan";
#endif
}
std::string registrationFile(){return config()+"/link-installation";}
bool receipt(const char* base){std::ifstream f(registrationFile());std::string exe,dir;return bool(std::getline(f,exe)&&std::getline(f,dir))&&exe==executable()&&dir==canonical(base);}
std::string socketPath(const char* base){
    auto path=canonical(base);if(path.empty())return "";
    std::string directory="/tmp/ezquake-vulkan-"+std::to_string(getuid());
    if(mkdir(directory.c_str(),0700)&&errno!=EEXIST)return "";
    struct stat st{};if(lstat(directory.c_str(),&st)||!S_ISDIR(st.st_mode)||st.st_uid!=getuid()||(st.st_mode&077))return "";
    unsigned char digest[SHA256_DIGEST_LENGTH];SHA256(reinterpret_cast<const unsigned char*>(path.data()),path.size(),digest);
    const char* hex="0123456789abcdef";directory+='/';for(int i=0;i<16;++i){directory+=hex[digest[i]>>4];directory+=hex[digest[i]&15];}
    return directory;
}
bool address(const std::string& path,sockaddr_un& addr){if(path.empty()||path.size()>=sizeof(addr.sun_path))return false;addr={};addr.sun_family=AF_UNIX;memcpy(addr.sun_path,path.c_str(),path.size()+1);return true;}
#ifndef __APPLE__
std::string desktopPath(){const char* x=getenv("XDG_DATA_HOME");return (x&&*x?std::string(x):home()+"/.local/share")+"/applications/ezquake-vulkan.desktop";}
std::string desktopQuote(const std::string& s){std::string q="\"";for(char c:s){if(c=='%' )q+='%';if(c=='"'||c=='\\'||c=='`'||c=='$')q+='\\';if(c=='\n'||c=='\r')return "";q+=c;}return q+'"';}
int run(const std::vector<std::string>& args,std::string* output=nullptr){
    int pipes[2];if(pipe(pipes))return -1;
    std::vector<char*> argv;for(auto& arg:args)argv.push_back(const_cast<char*>(arg.c_str()));argv.push_back(nullptr);
    posix_spawn_file_actions_t actions;posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions,pipes[1],STDOUT_FILENO);
    posix_spawn_file_actions_addclose(&actions,pipes[0]);
    posix_spawn_file_actions_addclose(&actions,pipes[1]);
    posix_spawn_file_actions_addopen(&actions,STDERR_FILENO,"/dev/null",O_WRONLY,0);
    pid_t child;int error=posix_spawnp(&child,argv[0],&actions,nullptr,argv.data(),environ);
    posix_spawn_file_actions_destroy(&actions);close(pipes[1]);
    if(error){close(pipes[0]);return -1;}
    char bytes[256];ssize_t n;while((n=read(pipes[0],bytes,sizeof(bytes)))>0)if(output&&output->size()<4096)output->append(bytes,size_t(n));close(pipes[0]);
    int status=0;while(waitpid(child,&status,0)<0)if(errno!=EINTR)return -1;return WIFEXITED(status)?WEXITSTATUS(status):-1;
}
#endif
}
extern "C" {
const char* FriendsPlatform_DefaultBase(void){
    static std::string base;
    std::ifstream f(registrationFile());std::string exe,dir;
    if(std::getline(f,exe)&&std::getline(f,dir)&&exe==executable()&&!canonical(dir.c_str()).empty())base=dir;
#ifdef __APPLE__
    else base=home()+"/Library/Application Support/ezQuake-Vulkan";
#else
    else{const char* x=getenv("XDG_DATA_HOME");base=(x&&*x?std::string(x):home()+"/.local/share")+"/ezquake-vulkan";}
#endif
    return base.c_str();
}
int FriendsPlatform_Registered(const char* base){
    if(!receipt(base))return 0;
#ifdef __APPLE__
    return FriendsMac_Registered();
#else
    std::error_code error;return std::filesystem::is_regular_file(desktopPath(),error);
#endif
}
int FriendsPlatform_Register(const char* base,int enable){try{
    auto dir=canonical(base),exe=executable();if(dir.empty()||exe.empty()||dir.find('\n')!=std::string::npos)return 0;
    if(!enable&&!receipt(base))return 0;
#ifdef __APPLE__
    if(!FriendsMac_Register(enable))return 0;
#else
    auto desktop=desktopPath();
    if(enable){
        std::filesystem::create_directories(std::filesystem::path(desktop).parent_path());
        auto launch=getenv("APPIMAGE")?canonical(getenv("APPIMAGE")):exe;
        std::ofstream f(desktop);f<<"[Desktop Entry]\nType=Application\nName=ezQuake Vulkan\nExec="<<desktopQuote(launch)<<" -nohome -basedir "<<desktopQuote(dir)<<" +set vid_renderer 2 -friends-invite %u\nTerminal=false\nNoDisplay=true\nMimeType=x-scheme-handler/ezquake-vulkan;\n";f.close();if(!f)return 0;
        if(run({"xdg-mime","default","ezquake-vulkan.desktop","x-scheme-handler/ezquake-vulkan"}))return 0;
    }else unlink(desktop.c_str());
#endif
    std::filesystem::create_directories(config());
    if(enable){std::ofstream f(registrationFile());f<<exe<<'\n'<<dir<<'\n';return bool(f);}
    unlink(registrationFile().c_str());return 1;
}catch(...){return 0;}}
int FriendsPlatform_Forward(const char* base,const char* link){
    if(!NF_ValidateInvite(link))return 0;sockaddr_un addr{};if(!address(socketPath(base),addr))return 0;
    int fd=socket(AF_UNIX,SOCK_DGRAM,0);if(fd<0)return 0;fcntl(fd,F_SETFL,O_NONBLOCK);
    auto size=strlen(link)+1;ssize_t n=sendto(fd,link,size,0,reinterpret_cast<sockaddr*>(&addr),sizeof(addr));close(fd);return n==ssize_t(size);
}
void FriendsPlatform_Init(const char* base){
    auto path=socketPath(base);sockaddr_un addr{};if(!address(path,addr))return;
    guard=open((path+".lock").c_str(),O_RDWR|O_CREAT|O_NOFOLLOW|O_CLOEXEC,0600);
    if(guard<0||flock(guard,LOCK_EX|LOCK_NB)){if(guard>=0)close(guard);guard=-1;return;}
    inbox=socket(AF_UNIX,SOCK_DGRAM,0);if(inbox<0){FriendsPlatform_Close();return;}
    fcntl(inbox,F_SETFL,O_NONBLOCK);fcntl(inbox,F_SETFD,FD_CLOEXEC);unlink(path.c_str());
    if(bind(inbox,reinterpret_cast<sockaddr*>(&addr),sizeof(addr))){FriendsPlatform_Close();return;}
    chmod(path.c_str(),0600);inboxPath=path;
}
void FriendsPlatform_Poll(void){
    if(inbox<0)return;char data[513];ssize_t n=recv(inbox,data,sizeof(data),0);
    if(n>1&&n<ssize_t(sizeof(data))&&data[n-1]==0&&strlen(data)==size_t(n-1))Friends_ReceiveInvitation(data);
}
void FriendsPlatform_Close(void){if(inbox>=0)close(inbox);inbox=-1;if(!inboxPath.empty())unlink(inboxPath.c_str());inboxPath.clear();if(guard>=0)close(guard);guard=-1;}
}
