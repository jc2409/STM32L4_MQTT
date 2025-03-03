#include "MQTTInterface.h"
#include <stdint.h>
#include "es_wifi.h"  // Provides WIFI_STATUS_OK, etc.
#include "wifi.h"     // Provides function prototypes like WIFI_SendData, WIFI_ReceiveData, etc.

// Optional: define a debug macro if not defined elsewhere
#ifndef LOG
#define LOG(a) // or you can define it as printf a
#endif

int mqtt_network_read(Network* n, unsigned char* buffer, int len, int timeout_ms) {
    uint16_t respLen;
    if(WIFI_STATUS_OK == WIFI_ReceiveData(n->socket, buffer, len, &respLen, timeout_ms)) {
        return respLen;
    }
    return -1;
}


int mqtt_network_write(Network* n, unsigned char* buffer, int len, int timeout_ms) {
    uint16_t sentLen = 0;
    int ret = WIFI_SendData(n->socket, buffer, len, &sentLen, timeout_ms);
    if (ret == WIFI_STATUS_OK) {
        LOG(("mqtt_network_write: Sent %d bytes\n", sentLen));
        return sentLen;
    }
    LOG(("mqtt_network_write: Error sending data (ret = %d)\n", ret));
    return -1;
}

void mqtt_network_disconnect(Network* n) {
    LOG(("mqtt_network_disconnect: Closing connection on socket %d\n", n->socket));
    WIFI_CloseServerConnection(n->socket);
}
