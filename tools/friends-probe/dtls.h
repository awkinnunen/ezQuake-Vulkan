// FRIENDS-002, OpenAI Codex, 2026-09-16. GPL-2.0-or-later.
#pragma once
#include "protocol.h"
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/x509.h>
#include <memory>

namespace friends {
template<class T,void(*Free)(T*)> using Handle=std::unique_ptr<T,decltype(Free)>;
inline std::string asHex(const unsigned char* bytes,size_t n) {
    const char* digits="0123456789ABCDEF";std::string s;
    for(size_t i=0;i<n;++i){s+=digits[bytes[i]>>4];s+=digits[bytes[i]&15];}return s;
}
inline std::string randomHex(size_t n) {
    std::vector<unsigned char> bytes(n);require(RAND_bytes(bytes.data(),int(n))==1,"Random generator failed");return asHex(bytes.data(),n);
}
inline std::string certFingerprint(X509* cert) {
    unsigned char digest[EVP_MAX_MD_SIZE];unsigned n=0;
    require(cert&&X509_digest(cert,EVP_sha256(),digest,&n)==1&&n==32,"Certificate digest failed");
    return asHex(digest,n);
}
class Identity {
public:
    Handle<EVP_PKEY,EVP_PKEY_free> key{nullptr,EVP_PKEY_free};
    Handle<X509,X509_free> cert{nullptr,X509_free};
    std::string fingerprint;
    Identity() {
        key.reset(EVP_EC_gen("prime256v1"));cert.reset(X509_new());
        require(key&&cert,"Certificate allocation failed");
        require(X509_set_version(cert.get(),2)==1,"Certificate version failed");
        require(ASN1_INTEGER_set(X509_get_serialNumber(cert.get()),1)==1,"Certificate serial failed");
        require(X509_gmtime_adj(X509_getm_notBefore(cert.get()),-60)&&X509_gmtime_adj(X509_getm_notAfter(cert.get()),86400),"Certificate validity failed");
        require(X509_set_pubkey(cert.get(),key.get())==1,"Certificate public key failed");
        auto name=X509_get_subject_name(cert.get());
        require(X509_NAME_add_entry_by_txt(name,"CN",MBSTRING_ASC,reinterpret_cast<const unsigned char*>("ezv-probe"),-1,-1,0)==1,"Certificate subject failed");
        require(X509_set_issuer_name(cert.get(),name)==1&&X509_sign(cert.get(),key.get(),EVP_sha256())>0,"Certificate signing failed");
        fingerprint=certFingerprint(cert.get());
    }
};
class Dtls {
    Handle<SSL_CTX,SSL_CTX_free> context{nullptr,SSL_CTX_free};
    Handle<SSL,SSL_free> ssl{nullptr,SSL_free};
    std::string expected;
    bool authenticated=false;
    void check(int n) {
        if(n>0)return;
        int e=SSL_get_error(ssl.get(),n);
        require(e==SSL_ERROR_WANT_READ||e==SSL_ERROR_WANT_WRITE,"DTLS transport failed");
    }
public:
    Dtls(bool host,const Identity& id,const std::string& expectedFingerprint):expected(expectedFingerprint) {
        require(hex(expected,64),"Missing expected certificate identity");
        context.reset(SSL_CTX_new(DTLS_method()));require(bool(context),"DTLS context failed");
        require(SSL_CTX_set_min_proto_version(context.get(),DTLS1_2_VERSION)==1,"DTLS minimum version failed");
        require(SSL_CTX_use_certificate(context.get(),id.cert.get())==1&&SSL_CTX_use_PrivateKey(context.get(),id.key.get())==1,"DTLS identity failed");
        // Ephemeral self-signed peer certificates: explicit fingerprint pinning below,
        // before any application read/write. Do not substitute this for broker CA validation.
        SSL_CTX_set_verify(context.get(),SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT,[](int,X509_STORE_CTX*){return 1;});
        ssl.reset(SSL_new(context.get()));require(bool(ssl),"DTLS connection failed");
        BIO* input=BIO_new(BIO_s_dgram_mem());BIO* output=BIO_new(BIO_s_dgram_mem());
        if(!input||!output){BIO_free(input);BIO_free(output);throw std::runtime_error("DTLS datagram queues failed");}
        SSL_set_bio(ssl.get(),input,output);
        SSL_set_options(ssl.get(),SSL_OP_NO_QUERY_MTU);
        require(SSL_set_mtu(ssl.get(),1200)>0,"DTLS MTU failed");
        if(host)SSL_set_accept_state(ssl.get());else SSL_set_connect_state(ssl.get());
    }
    void input(const std::string& data) {
        require(!data.empty()&&data.size()<=MaxDatagram,"Invalid encrypted datagram size");
        require(BIO_write(SSL_get_rbio(ssl.get()),data.data(),int(data.size()))==int(data.size()),"DTLS input overflow");
    }
    void tick() {
        if(!authenticated){
            int n=SSL_do_handshake(ssl.get());check(n);
            if(n==1){
                Handle<X509,X509_free> cert{SSL_get1_peer_certificate(ssl.get()),X509_free};
                auto fp=certFingerprint(cert.get());
                require(CRYPTO_memcmp(fp.data(),expected.data(),64)==0,"Host/peer certificate does not match invitation or SDP");
                authenticated=true;
            }
        }
        require(DTLSv1_handle_timeout(ssl.get())>=0,"DTLS handshake retransmission failed");
    }
    bool ready()const{return authenticated;}
    std::string read() {
        require(authenticated,"Read before certificate authentication");
        char data[1200];int n=SSL_read(ssl.get(),data,sizeof(data));check(n);
        return n>0?std::string(data,n):std::string();
    }
    void write(const std::string& data) {
        require(authenticated && !data.empty() && data.size()<=1100,"Invalid protected payload");
        int n=SSL_write(ssl.get(),data.data(),int(data.size()));check(n);
        require(n==int(data.size()),"DTLS send would block");
    }
    bool output(std::string& data) {
        char bytes[MaxDatagram];int n=BIO_read(SSL_get_wbio(ssl.get()),bytes,sizeof(bytes));
        if(n<=0)return false;data.assign(bytes,n);return true;
    }
};
}
