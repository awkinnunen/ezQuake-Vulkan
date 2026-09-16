// OpenAI Codex, GPL-2.0-or-later. Owner-only Unix files, locks and atomic replacement.
#pragma once
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <vector>
namespace friends {
struct FileDescriptor {
    int value;
    explicit FileDescriptor(int fd):value(fd){}
    ~FileDescriptor(){if(value>=0)::close(value);}
};
inline int privateOpen(const std::string& path,int flags){
    int fd=::open(path.c_str(),flags|O_NOFOLLOW|O_CLOEXEC,0600);
    require(fd>=0,"Cannot open private Friends file");struct stat st{};
    if(fstat(fd,&st)||!S_ISREG(st.st_mode)||st.st_uid!=getuid()||(st.st_mode&077)){
        ::close(fd);throw std::runtime_error("Friends file must be owned by you and have mode 0600");
    }
    return fd;
}
inline std::string privateRead(const std::string& path){
    FileDescriptor f(privateOpen(path,O_RDONLY));struct stat st{};
    require(!fstat(f.value,&st)&&st.st_size>0&&st.st_size<65536,"Invalid Friends identity size");
    std::string data(size_t(st.st_size),'\0');size_t done=0;
    while(done<data.size()){ssize_t n=::read(f.value,data.data()+done,data.size()-done);if(n<0&&errno==EINTR)continue;require(n>0,"Friends identity read failed");done+=size_t(n);}
    return data;
}
inline void privateSave(const std::string& path,const std::string& data){
    std::string name=path+".tmp.XXXXXX";std::vector<char> temp(name.begin(),name.end());temp.push_back(0);
    FileDescriptor f(mkstemp(temp.data()));require(f.value>=0,"Cannot create Friends identity");
    try{
        require(!fchmod(f.value,0600),"Cannot protect Friends identity permissions");size_t done=0;
        while(done<data.size()){ssize_t n=::write(f.value,data.data()+done,data.size()-done);if(n<0&&errno==EINTR)continue;require(n>0,"Friends identity write failed");done+=size_t(n);}
        require(!fsync(f.value),"Friends identity sync failed");
        require(!rename(temp.data(),path.c_str()),"Friends identity replace failed");
        FileDescriptor dir(::open(std::filesystem::path(path).parent_path().c_str(),O_RDONLY|O_DIRECTORY|O_CLOEXEC));
        if(dir.value>=0)fsync(dir.value);
    }catch(...){unlink(temp.data());throw;}
}
}
