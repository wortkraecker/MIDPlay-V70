#include "http_client.h"
#include "utils.h"
#include <3ds/services/httpc.h>
#include <cstring>
#include <cstdio>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

HttpClient::HttpClient() {}

bool HttpClient::init() {
    return true;
}

void HttpClient::exit() {}

static Result http_download(httpcContext* ctx, HttpResponse& out) {
    Result rc = httpcBeginRequest(ctx);
    if (R_FAILED(rc)) return rc;

    u32 statusCode = 0;
    rc = httpcGetResponseStatusCode(ctx, &statusCode);
    if (R_FAILED(rc)) return rc;
    out.status = (int)statusCode;

    u32 chunkSize = 0;
    do {
        rc = httpcDownloadData(ctx, nullptr, 0, &chunkSize);
        if (R_FAILED(rc) && rc != HTTPC_RESULTCODE_DOWNLOADPENDING) break;
        if (chunkSize) {
            size_t cur = out.body.size();
            out.body.resize(cur + chunkSize);
            size_t downloaded = 0;
            rc = httpcDownloadData(ctx, out.body.data() + cur, chunkSize, &downloaded);
            if (R_FAILED(rc)) break;
            out.body.resize(cur + downloaded);
        }
    } while (chunkSize > 0 || rc == HTTPC_RESULTCODE_DOWNLOADPENDING);
    return rc;
}

bool HttpClient::request(const std::string& method, const std::string& path, const std::string& payload, HttpResponse& out) {
    httpcContext ctx;
    Result rc = httpcOpenContext(&ctx, method == "POST" ? HTTPC_METHOD_POST : HTTPC_METHOD_GET,
                                 (http_base() + path).c_str(), 1);
    if (R_FAILED(rc)) return false;

    httpcSetKeepAlive(&ctx, HTTPC_KEEPALIVE_ENABLED);
    if (!payload.empty()) {
        httpcSetPostDataRaw(&ctx, (u32*)payload.data(), payload.size());
    }

    rc = http_download(&ctx, out);
    httpcCloseContext(&ctx);
    return R_SUCCEEDED(rc) && out.status == 200;
}

bool HttpClient::get(const std::string& path, HttpResponse& out) { return request("GET", path, "", out); }
bool HttpClient::post(const std::string& path, const std::string& payload, HttpResponse& out) { return request("POST", path, payload, out); }

// --- streaming over raw sockets for lower latency ---
bool HttpClient::resolve(addrinfo** res) {
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int rc = getaddrinfo(http_host().c_str(), std::to_string(SERVER_PORT).c_str(), &hints, res);
    return rc == 0;
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
    if (r <= 0) return -1;
    return (int)r;
}

void HttpClient::closeStream(int sock) { if (sock >= 0) close(sock); }
