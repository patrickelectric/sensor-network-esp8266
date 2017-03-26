#include <Arduino.h>
#include <Thread.h>
#include <ThreadController.h>

#include "debug.h"
#include "connection.h"
#include "ota.h"
#include "serverclient.h"

#define LED D4

void hardwareInit()
{
    Serial.begin(115200);
    debug("Starting Hardware.");
    pinMode(LED, OUTPUT);
}

Thread printDataThread = Thread();
Thread connectionThread = Thread();
Thread serverClientThread = Thread();
ThreadController groupOfThreads = ThreadController();

void setup()
{
    hardwareInit();

    //Start data thread
    Data::self();
    printDataThread.onRun([]() {
        const static unsigned long startTime = micros();
        Data::self().data->freeHeap = ESP.getFreeHeap();
        Data::self().data->upTime = (micros() - startTime)/1e3;
        Data::self().print();
    });
    printDataThread.setInterval(10000);

    //Start connection manager thread
    Connection::self();
    ServerClient::self();
    Connection::self().setData(Data::self().data);
    debug("Scanning for provant network...");
    if(Connection::self().scanForNetwork("provant")) {
        debug("provant network found, set Slave Mode");
        Connection::self().addWifi("provant");
        ServerClient::self().setClient(String("192.168.0.1"), 80);
    } else {
        debug("provant network not found, set Master Mode");
        const static char* apname = "provant";
        const static IPAddress ip(192, 168, 0, 1 );
        const static IPAddress gateway(192, 168, 1, 1);
        const static IPAddress subnet(255, 255, 255, 0);
        Connection::self().wifi()->softAPConfig(ip, gateway, subnet);
        Connection::self().wifi()->softAP(apname);
        ServerClient::self().setServer();
    }

    connectionThread.onRun([]() {
        if(Connection::self().run()) {
            digitalWrite(LED, !digitalRead(LED));
        }
    });
    connectionThread.setInterval(1000);

    //Start serverClient thread
    ServerClient::self().begin();
    serverClientThread.onRun([]() {
        ServerClient::self().run();
    });
    if(ServerClient::self().isServer())
        serverClientThread.setInterval(1000);
    else
        serverClientThread.setInterval(10);

    //Add all threads in ThreadController
    groupOfThreads.add(&printDataThread);
    groupOfThreads.add(&connectionThread);
    groupOfThreads.add(&serverClientThread);

    //Start OTA
    Ota::self();
}

void loop()
{
    groupOfThreads.run();
    Ota::self().handle();
    yield();
}