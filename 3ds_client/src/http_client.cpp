#include "http_client.h"
#include "utils.h"
#include <cstring>
#include <cstdio>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

HttpClient::HttpClient() {}

bool HttpClient::init() {
    // socInit must be called once in the app; done in main
    return true;
}

void HttpClient::exit() {
}

bool HttpClient::resolve(addrinfo** res) {
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    std::string host = http_host();
    int rc = getaddrinfo(host.c_str(), std::to_string(SERVER_PORT).c_str(), &hints, res);
    return rc == 0;
}

bool HttpClient::request(const std::string& method, const std::string& path, const std::string& payload, HttpResponse& out) {
    addrinfo* res = nullptr;
    if (!resolve(&res)) return false;
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) { freeaddrinfo(res); return false; }

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        close(sock); freeaddrinfo(res); return false; }

    char hdr[512];
    int hdr_len = snprintf(hdr, sizeof(hdr), "%s %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\nContent-Length: %zu\r\n\r\n",
                           method.c_str(), path.c_str(), SERVER_IP, SERVER_PORT, payload.size());
    std::vector<u8> buf;
    buf.reserve(hdr_len + payload.size());
    buf.insert(buf.end(), (u8*)hdr, (u8*)hdr + hdr_len);
    buf.insert(buf.end(), payload.begin(), payload.end());

    ssize_t sent = send(sock, buf.data(), buf.size(), 0);
    if (sent < 0) { close(sock); freeaddrinfo(res); return false; }

    // Simple HTTP response parsing (status + body)
    std::string header;
    char tmp[512];
    int status = 0;
    while (true) {
        ssize_t r = recv(sock, tmp, sizeof(tmp), 0);
        if (r <= 0) break;
        header.append(tmp, tmp + r);
        size_t pos = header.find("\r\n\r\n");
        if (pos != std::string::npos) {
            std::string head = header.substr(0, pos);
            size_t bodyStart = pos + 4;
            // parse status
            size_t sp = head.find(' ');
            if (sp != std::string::npos && head.size() > sp + 4) {
                status = std::atoi(head.substr(sp + 1, 3).c_str());
            }
            // content-type
            size_t ct = head.find("Content-Type:");
            if (ct != std::string::npos) {
                size_t end = head.find("\r\n", ct);
                out.contentType = head.substr(ct + 13, end - ct - 13);
            }
            out.status = status;
            out.body.insert(out.body.end(), header.begin() + bodyStart, header.end());
            break;
        }
    }
    // read rest
    while (true) {
        ssize_t r = recv(sock, tmp, sizeof(tmp), 0);
        if (r <= 0) break;
        out.body.insert(out.body.end(), (u8*)tmp, (u8*)tmp + r);
    }
    close(sock);
    freeaddrinfo(res);
    return status == 200;
}

bool HttpClient::get(const std::string& path, HttpResponse& out) {
    return request("GET", path, "", out);
}

bool HttpClient::post(const std::string& path, const std::string& payload, HttpResponse& out) {
    return request("POST", path, payload, out);
}

int HttpClient::openStream(const std::string& path) {
    addrinfo* res = nullptr;
    if (!resolve(&res)) return -1;
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) { freeaddrinfo(res); return -1; }
    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) { close(sock); freeaddrinfo(res); return -1; }

    char hdr[256];
    int hdr_len = snprintf(hdr, sizeof(hdr), "GET %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: keep-alive\r\n\r\n",
                           path.c_str(), SERVER_IP, SERVER_PORT);
    if (send(sock, hdr, hdr_len, 0) < 0) { close(sock); freeaddrinfo(res); return -1; }

    // consume headers
    std::string header;
    char ch;
    while (recv(sock, &ch, 1, 0) == 1) {
        header.push_back(ch);
        if (header.size() >= 4 && header.substr(header.size() - 4) == "\r\n\r\n") break;
    }
    freeaddrinfo(res);
    return sock;
}

int HttpClient::readStream(int sock, u8* dst, size_t maxLen) {
    if (sock < 0) return -1;
    ssize_t r = recv(sock, dst, maxLen, 0);
    if (r == 0) return -1; // closed
    return (int)r;
}

void HttpClient::closeStream(int sock) {
    if (sock >= 0) close(sock);
}
