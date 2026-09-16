// OpenAI Codex, GPL-2.0-or-later. Private Windows-user-bound, atomically replaced identity.
#pragma once
#include "../../tools/friends-probe/dtls.h"
#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#endif
#include <openssl/pem.h>
#include <filesystem>
#include <fstream>
#ifndef _WIN32
#include "private_posix.h"
#endif
namespace friends {
inline std::string gameInvite(const Invite& i){auto s=encode(i);s.replace(s.find("/probe?"),7,"/join?");return s;}
inline Invite parseGameInvite(const std::string& input){
    // Windows ShellExecute canonicalizes an empty URL path by inserting '/'.
    // Accept that single equivalent spelling; retain strict validation of all
    // other bytes and keep existing reusable invitations unchanged.
    auto s=input;
    if(s.rfind("ezquake-vulkan://join/?",0)==0)s.erase(std::string("ezquake-vulkan://join").size(),1);
    require(s.rfind("ezquake-vulkan://join?",0)==0,"Not a game invitation");
    s.replace(s.find("/join?"),6,"/probe?");return decode(s);
}
inline void appendField(std::string& s,const std::string& v){require(v.size()<16384,"Identity field too long");uint32_t n=uint32_t(v.size());s.append(reinterpret_cast<char*>(&n),4);s+=v;}
inline std::string takeField(const std::string& s,size_t& p){
    require(p+4<=s.size(),"Truncated identity");uint32_t n;memcpy(&n,s.data()+p,4);p+=4;
    require(n<16384&&n<=s.size()-p,"Invalid identity length");auto v=s.substr(p,n);p+=n;return v;
}
class StoredIdentity {
#ifdef _WIN32
    HANDLE lock=INVALID_HANDLE_VALUE;
#else
    int lock=-1;
#endif
    std::string path;
    static std::string pem(BIO* b){char* p=nullptr;long n=BIO_get_mem_data(b,&p);require(n>0,"Identity encoding failed");return std::string(p,n);}
public:
    Identity identity;
    Invite invitation;
    ~StoredIdentity(){
#ifdef _WIN32
        if(lock!=INVALID_HANDLE_VALUE)CloseHandle(lock);
#else
        if(lock>=0)::close(lock);
#endif
    }
    void save() {
        BIO* raw=BIO_new(BIO_s_mem());require(raw!=nullptr,"Identity buffer failed");
        std::unique_ptr<BIO,decltype(&BIO_free)> b(raw,BIO_free);
        require(PEM_write_bio_PrivateKey(b.get(),identity.key.get(),nullptr,nullptr,0,nullptr,nullptr)==1,"Identity private-key encoding failed");
        auto key=pem(b.get());BIO_reset(b.get());
        require(PEM_write_bio_X509(b.get(),identity.cert.get())==1,"Identity certificate encoding failed");
        std::string plain="EZVF1";appendField(plain,invitation.room);appendField(plain,invitation.key);appendField(plain,key);appendField(plain,pem(b.get()));
#ifdef _WIN32
        DATA_BLOB input{DWORD(plain.size()),reinterpret_cast<BYTE*>(plain.data())},output{};
        require(CryptProtectData(&input,L"ezQuake Friends identity",nullptr,nullptr,nullptr,CRYPTPROTECT_UI_FORBIDDEN,&output)!=0,"Cannot protect Friends identity");
        OPENSSL_cleanse(plain.data(),plain.size());OPENSSL_cleanse(key.data(),key.size());
        auto temporary=path+".tmp";
        HANDLE file=CreateFileA(temporary.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(file==INVALID_HANDLE_VALUE){LocalFree(output.pbData);throw std::runtime_error("Cannot save Friends identity");}
        DWORD written=0;bool ok=WriteFile(file,output.pbData,output.cbData,&written,nullptr)&&written==output.cbData&&FlushFileBuffers(file);
        CloseHandle(file);LocalFree(output.pbData);
        require(ok,"Friends identity write failed");
        require(MoveFileExA(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0,"Friends identity replace failed");
#else
        privateSave(path,plain);
        OPENSSL_cleanse(plain.data(),plain.size());OPENSSL_cleanse(key.data(),key.size());
#endif
    }
    void open(const std::string& file) {
        path=file;std::filesystem::create_directories(std::filesystem::path(path).parent_path());
#ifdef _WIN32
        lock=CreateFileA((path+".lock").c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
        require(lock!=INVALID_HANDLE_VALUE,"Another host is using this Friends identity");
        if(GetFileAttributesA(path.c_str())==INVALID_FILE_ATTRIBUTES){
            require(GetLastError()==ERROR_FILE_NOT_FOUND,"Cannot access Friends identity");
            invitation={"EZV-"+randomHex(8),randomHex(32),identity.fingerprint};save();return;
        }
        std::ifstream f(path,std::ios::binary|std::ios::ate);require(bool(f),"Cannot read Friends identity");
        auto n=f.tellg();require(n>0&&n<65536,"Invalid Friends identity file");std::string encrypted(size_t(n),'\0');f.seekg(0);f.read(encrypted.data(),n);require(bool(f),"Friends identity read failed");
        DATA_BLOB input{DWORD(encrypted.size()),reinterpret_cast<BYTE*>(encrypted.data())},out{};
        require(CryptUnprotectData(&input,nullptr,nullptr,nullptr,nullptr,CRYPTPROTECT_UI_FORBIDDEN,&out)!=0,"Cannot unlock identity for this Windows user; preserve the file");
        std::string plain(reinterpret_cast<char*>(out.pbData),out.cbData);SecureZeroMemory(out.pbData,out.cbData);LocalFree(out.pbData);
#else
        lock=privateOpen(path+".lock",O_RDWR|O_CREAT);
        require(!flock(lock,LOCK_EX|LOCK_NB),"Another host is using this Friends identity");
        struct stat st{};
        if(lstat(path.c_str(),&st)){
            require(errno==ENOENT,"Cannot access Friends identity");
            invitation={"EZV-"+randomHex(8),randomHex(32),identity.fingerprint};save();return;
        }
        std::string plain=privateRead(path);
#endif
        require(plain.substr(0,5)=="EZVF1","Unknown Friends identity format");size_t p=5;
        invitation.room=takeField(plain,p);invitation.key=takeField(plain,p);auto key=takeField(plain,p),cert=takeField(plain,p);
        require(p==plain.size(),"Trailing Friends identity data");
        BIO* b=BIO_new_mem_buf(key.data(),int(key.size()));require(b!=nullptr,"Identity parse buffer failed");
        identity.key.reset(PEM_read_bio_PrivateKey(b,nullptr,nullptr,nullptr));BIO_free(b);
        b=BIO_new_mem_buf(cert.data(),int(cert.size()));require(b!=nullptr,"Identity parse buffer failed");
        identity.cert.reset(PEM_read_bio_X509(b,nullptr,nullptr,nullptr));BIO_free(b);
        OPENSSL_cleanse(plain.data(),plain.size());OPENSSL_cleanse(key.data(),key.size());
        require(identity.key&&identity.cert&&X509_check_private_key(identity.cert.get(),identity.key.get())==1,"Identity key/certificate mismatch");
        identity.fingerprint=certFingerprint(identity.cert.get());invitation.fingerprint=identity.fingerprint;gameInvite(invitation);
    }
    void rotate(){auto old=invitation.key;invitation.key=randomHex(32);try{save();}catch(...){invitation.key=old;throw;}}
};
}
