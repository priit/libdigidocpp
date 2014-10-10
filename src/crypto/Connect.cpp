#include "Connect.h"

#include "Conf.h"
#include "Container.h"
#include "crypto/OpenSSLHelpers.h"

#include <openssl/bio.h>
#include <openssl/ocsp.h>
#include <openssl/ssl.h>

using namespace digidoc;
using namespace std;

Connect::Connect(const string &url, const string &method)
{
    char *_host = nullptr, *_port = nullptr, *_path = nullptr;
    int usessl = 0;
    if(!OCSP_parse_url(const_cast<char*>(url.c_str()), &_host, &_port, &_path, &usessl))
        THROW_OPENSSLEXCEPTION("Incorrect URL provided: '%s'.", url.c_str());

    std::string host, port;
    string chost = host = _host ? _host : "";
    string cport = port = _port ? _port : "80";
    string path = _path ? _path : "/";
    OPENSSL_free(_host);
    OPENSSL_free(_port);
    OPENSSL_free(_path);

    Conf *c = Conf::instance();
    if(!c->proxyHost().empty())
    {
        chost = c->proxyHost();
        cport = c->proxyPort();
    }

    string hostname = chost + ":" + cport;
    SCOPE2(BIO, bio, BIO_new_connect(const_cast<char*>(hostname.c_str())), BIO_free_all);
    if(!bio)
        THROW_OPENSSLEXCEPTION("Failed to create connection with host: '%s'", hostname.c_str());

    if(usessl > 0)
    {
        ssl.reset(SSL_CTX_new(SSLv23_client_method()), function<void(SSL_CTX*)>(SSL_CTX_free));
        if(!ssl)
            THROW_OPENSSLEXCEPTION("Failed to create connection with host: '%s'", hostname.c_str());
        SSL_CTX_set_mode(ssl.get(), SSL_MODE_AUTO_RETRY);
        BIO *sbio = BIO_new_ssl(ssl.get(), 1);
        if(!sbio)
            THROW_OPENSSLEXCEPTION("Failed to create ssl connection with host: '%s'", hostname.c_str());
        d.reset(BIO_push(sbio, bio.release()), function<void(BIO*)>(BIO_free_all));
    }
    else
        d.reset(bio.release(), function<void(BIO*)>(BIO_free_all));

    if(!d)
        THROW_OPENSSLEXCEPTION("Failed to create connection with host: '%s'", hostname.c_str());

    if(!BIO_do_connect(d.get()))
        THROW_OPENSSLEXCEPTION("Failed to connect to host: '%s'", hostname.c_str());

    BIO_printf(d.get(), "%s %s HTTP/1.0\r\n", method.c_str(), path.c_str());
    BIO_printf(d.get(), "Host: %s:%s\r\n", host.c_str(), port.c_str());
    BIO_printf(d.get(), "User-Agent: LIB libdigidocpp/%s APP %s\r\n",
        VER_STR(MAJOR_VER.MINOR_VER.RELEASE_VER.BUILD_VER), appInfo().c_str());

    if(!c->proxyUser().empty() || !c->proxyPass().empty())
    {
        BIO_printf(d.get(), "Proxy-Authorization: Basic ");
        SCOPE2(BIO, b64, BIO_new(BIO_f_base64()), BIO_free_all);
        BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);
        BIO_push(b64.get(), d.get());
        BIO_printf(b64.get(), "%s:%s", c->proxyUser().c_str(), c->proxyPass().c_str());
        (void)BIO_flush(b64.get());
        BIO_pop(b64.get());
        BIO_printf(d.get(), "\r\n");
    }
}

void Connect::addHeader(const string &key, const string &value)
{
    BIO_printf(d.get(), "%s: %s\r\n", key.c_str(), value.c_str());
}

Connect::Result Connect::exec(const vector<unsigned char> &send)
{
    if(!send.empty())
    {
        BIO_printf(d.get(), "Content-Length: %ld\r\n\r\n", send.size());
        BIO_write(d.get(), &send[0], int(send.size()));
    }
    else
        BIO_printf(d.get(), "\r\n");

    int rc = 0;
    size_t pos = 0;
    string data(1024, 0);
    do {
        if(size_t(pos += rc) >= data.size())
            data.resize(data.size()*2);
    } while((rc = BIO_read(d.get(), &data[pos], int(data.size() - pos))) > 0);

    data.resize(pos);
    pos = data.find("\r\n\r\n");

    stringstream header(data.substr(0, pos));
    string line;
    Result r;
    while(getline(header, line))
    {
        line.resize(line.size() - 1);
        if(line.empty())
            continue;
        if(r.result.empty())
        {
            r.result = line;
            continue;
        }
        size_t split = line.find(": ");
        if(split != string::npos)
            r.headers[line.substr(0, split)] = line.substr(split + 2);
        else
            r.headers[line] = string();
    }

    if(pos != string::npos)
        r.content = data.substr(pos + 4);

    return r;
}