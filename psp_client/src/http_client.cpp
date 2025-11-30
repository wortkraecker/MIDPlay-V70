#include "http_client.h"
#include "utils.h"

#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

HttpClient::HttpClient(const char* host, int port) : m_port(port), m_sock(-1) {
    strncpy(m_host, host, sizeof(m_host));
    m_host[sizeof(m_host) - 1] = '\0';
}

HttpClient::~HttpClient() {
    closeStream();
}

int HttpClient::connectSocket() {
    if (m_sock >= 0) return m_sock;

    struct hostent* he = gethostbyname(m_host);
    if (!he) {
        LOG_ERROR("DNS failed for %s", m_host);
        return -1;
    }

    m_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sock < 0) {
        LOG_ERROR("socket failed");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    memcpy(&addr.sin_addr, he->h_addr, he->h_length);

    if (connect(m_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        LOG_ERROR("connect failed");
        closeStream();
        return -1;
    }
    return m_sock;
}

int HttpClient::sendAll(const char* data, size_t len) {
    size_t total = 0;
    while (total < len) {
        int sent = send(m_sock, data + total, len - total, 0);
        if (sent <= 0) return -1;
        total += sent;
    }
    return 0;
}

int HttpClient::get(const char* path, HttpResponse& resp, unsigned char* buffer, size_t bufferSize, bool binary) {
    if (connectSocket() < 0) return -1;

    char request[256];
    snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, m_host);
    if (sendAll(request, strlen(request)) < 0) return -1;

    int received = recv(m_sock, buffer, bufferSize, 0);
    if (received <= 0) return -1;
    buffer[received] = 0;

    // crude header parsing
    char* headerEnd = strstr((char*)buffer, "\r\n\r\n");
    if (!headerEnd) return -1;
    size_t headerSize = headerEnd - (char*)buffer + 4;
    resp.status = 200;
    resp.content_length = 0;
    resp.data = buffer + headerSize;
    resp.data_size = received - headerSize;
    if (!binary) resp.data[resp.data_size] = 0;

    closeStream();
    return 0;
}

int HttpClient::post(const char* path, const char* body, HttpResponse& resp, unsigned char* buffer, size_t bufferSize) {
    if (connectSocket() < 0) return -1;
    char request[512];
    int bodyLen = body ? strlen(body) : 0;
    snprintf(request, sizeof(request),
             "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\nConnection: close\r\n\r\n",
             path, m_host, bodyLen);

    if (sendAll(request, strlen(request)) < 0) return -1;
    if (bodyLen > 0 && sendAll(body, bodyLen) < 0) return -1;

    int received = recv(m_sock, buffer, bufferSize, 0);
    if (received <= 0) return -1;
    buffer[received] = 0;
    char* headerEnd = strstr((char*)buffer, "\r\n\r\n");
    if (!headerEnd) return -1;
    size_t headerSize = headerEnd - (char*)buffer + 4;
    resp.status = 200;
    resp.content_length = 0;
    resp.data = buffer + headerSize;
    resp.data_size = received - headerSize;

    closeStream();
    return 0;
}

int HttpClient::openStream(const char* path) {
    if (connectSocket() < 0) return -1;
    char request[256];
    snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n", path, m_host);
    if (sendAll(request, strlen(request)) < 0) return -1;

    // Consume headers
    char headerBuf[1024];
    int headerLen = 0;
    while (headerLen < (int)sizeof(headerBuf) - 1) {
        int r = recv(m_sock, headerBuf + headerLen, 1, 0);
        if (r <= 0) break;
        headerLen += r;
        if (headerLen >= 4 && memcmp(headerBuf + headerLen - 4, "\r\n\r\n", 4) == 0) break;
    }
    return 0;
}

int HttpClient::readStream(unsigned char* buffer, size_t bufSize) {
    if (m_sock < 0) return -1;
    int r = recv(m_sock, buffer, bufSize, 0);
    if (r <= 0) {
        closeStream();
        return -1;
    }
    return r;
}

void HttpClient::closeStream() {
    if (m_sock >= 0) {
        sceNetInetShutdown(m_sock, SHUT_RDWR);
        sceNetInetClose(m_sock);
        m_sock = -1;
    }
}

bool HttpClient::isStreamOpen() const {
    return m_sock >= 0;
}

