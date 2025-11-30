#pragma once
#include <3ds.h>
#include <string>
#include <vector>
#include <netdb.h>

struct HttpResponse {
    int status;
    std::vector<u8> body;
    std::string contentType;
};

class HttpClient {
public:
    HttpClient();
    bool init();
    void exit();

    bool get(const std::string& path, HttpResponse& out);
    bool post(const std::string& path, const std::string& payload, HttpResponse& out);

    // Stream helpers
    int openStream(const std::string& path);
    int readStream(int sock, u8* dst, size_t maxLen);
    void closeStream(int sock);

private:
    bool request(const std::string& method, const std::string& path, const std::string& payload, HttpResponse& out);
    bool resolve(addrinfo** res);
};
