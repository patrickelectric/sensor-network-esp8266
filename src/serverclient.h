#pragma once
#include <Arduino.h>
#include <WiFiServer.h>
#include <WebSocketsServer.h>
#include <WebSocketsClient.h>

class ServerClient
{
public:
    void begin();

    bool isServer();

    bool onGoing();

    void setServer();

    void setClient(String ip, uint port);

    void run();

    static ServerClient& self();
    ~ServerClient();

private:
    void serverHandle(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght);
    void clientHandle(WStype_t type, uint8_t * payload, size_t lenght);

    ServerClient& operator = (ServerClient& other) = delete;
    ServerClient(const ServerClient& other) = delete;
    ServerClient();

    WebSocketsServer* server;
    WebSocketsClient* client;
    String clientIp;
    uint clientPort;
    bool started;
};