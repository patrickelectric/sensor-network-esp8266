#include <Arduino.h>
#include "serverclient.h"
#include "debug.h"

ServerClient::ServerClient() :
    server(nullptr), client(nullptr), started(false)
{
};

bool ServerClient::isServer()
{
    return server;
}

void ServerClient::setServer()
{
    if(server)
        return;
    server = new WebSocketsServer(80);
    if(!client)
        delete client;
}

void ServerClient::setClient(String ip, uint port)
{
    if(client)
        return;
    clientIp = ip;
    clientPort = port;
    client = new WebSocketsClient();
    if(!server)
        delete server;
}

void ServerClient::begin()
{
    if(!started) {
        if(server) {
            debug("Starting server..");
            server->begin();
            server->onEvent([=](uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
                serverHandle(num, type, payload, lenght);
            });
        } else if (client) {
            debug("Starting client..");
            client->begin(clientIp, clientPort);
            client->onEvent([=](WStype_t type, uint8_t * payload, size_t lenght) {
                clientHandle(type, payload, lenght);
            });
        } else {
            debug("[ERROR] No server and client to start !");
        }
        started = true;
    } else {
        debug("Already started !");
    }
};

void ServerClient::run()
{
    if(server) {
        server->broadcastTXT("Server broadcast message: provantRules !");
        server->loop();
    } else if (client) {
        client->loop();
    } else {
        debug("[ERROR] No server and client to start !");
    }
}

bool ServerClient::onGoing()
{
    return started;
};

void ServerClient::serverHandle(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght)
{
    debug("Running handle");
    switch(type) {
    case WStype_DISCONNECTED:
        debug("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED:
        debug("[%u] Connected from url: %s\n", num, payload);
        server->sendTXT(num, "Connected =D");
        break;
    case WStype_TEXT:
        debug("[%u] get Text: %s\n", num, payload);
        break;
    case WStype_BIN:
        debug("[%u] get binary lenght: %u\n", num, lenght);
        hexdump(payload, lenght);
        break;
    }
}

void ServerClient::clientHandle(WStype_t type, uint8_t * payload, size_t lenght)
{
    switch(type) {
    case WStype_DISCONNECTED:
        debug("[WSc] Disconnected!");
        break;
    case WStype_CONNECTED:
        debug("[WSc] Connected to url: %s\n",  payload);
        client->sendTXT("Connected!");
        break;
    case WStype_TEXT:
        debug("[WSc] get text: %s\n", payload);
        client->sendTXT("Ack!");
        break;
    case WStype_BIN:
        debug("[WSc] get binary lenght: %u\n", lenght);
        hexdump(payload, lenght);
        break;
    }

}

ServerClient& ServerClient::self()
{
    static ServerClient self;
    return self;
};

ServerClient::~ServerClient()
{
};