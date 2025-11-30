#ifndef PSP_CLIENT_HTTP_CLIENT_H
#define PSP_CLIENT_HTTP_CLIENT_H

#include <psptypes.h>
#include <stddef.h>

struct HttpResponse {
    int status;
    size_t content_length;
    unsigned char* data;
    size_t data_size;
};

class HttpClient {
public:
    HttpClient(const char* host, int port);
    ~HttpClient();

    int get(const char* path, HttpResponse& resp, unsigned char* buffer, size_t bufferSize, bool binary);
    int post(const char* path, const char* body, HttpResponse& resp, unsigned char* buffer, size_t bufferSize);
    int openStream(const char* path);
    int readStream(unsigned char* buffer, size_t bufSize);
    void closeStream();
    bool isStreamOpen() const;

private:
    int connectSocket();
    int sendAll(const char* data, size_t len);

    char m_host[64];
    int m_port;
    int m_sock;
};

#endif
