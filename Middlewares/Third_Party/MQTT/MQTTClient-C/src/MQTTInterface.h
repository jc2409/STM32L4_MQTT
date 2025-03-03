#ifndef MQTT_INTERFACE_H
#define MQTT_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Network interface structure for the MQTT Embedded Client.
 *
 * The MQTT Embedded Client expects a Network structure that includes
 * function pointers for reading and writing data. Your Wi‑Fi driver must
 * provide implementations for these functions.
 */
typedef struct Network {
    uint32_t socket;  /**< Socket identifier; use uint32_t to match the Wi‑Fi driver */
    int (*mqttread)(struct Network* n, unsigned char* buffer, int len, int timeout_ms);
    int (*mqttwrite)(struct Network* n, unsigned char* buffer, int len, int timeout_ms);
} Network;

/**
 * @brief Read data from the network.
 *
 * This function wraps your Wi‑Fi driver's receive function.
 *
 * @param n          Pointer to the Network structure.
 * @param buffer     Buffer in which to store received data.
 * @param len        Maximum number of bytes to read.
 * @param timeout_ms Timeout in milliseconds.
 * @return Number of bytes read on success, or -1 on error.
 */
int mqtt_network_read(Network* n, unsigned char* buffer, int len, int timeout_ms);

/**
 * @brief Write data to the network.
 *
 * This function wraps your Wi‑Fi driver's send function.
 *
 * @param n          Pointer to the Network structure.
 * @param buffer     Data buffer containing the bytes to send.
 * @param len        Number of bytes to send.
 * @param timeout_ms Timeout in milliseconds.
 * @return Number of bytes written on success, or -1 on error.
 */
int mqtt_network_write(Network* n, unsigned char* buffer, int len, int timeout_ms);

/**
 * @brief Disconnect the network.
 *
 * This function wraps your Wi‑Fi driver's disconnect/close connection function.
 *
 * @param n Pointer to the Network structure.
 */
void mqtt_network_disconnect(Network* n);

#ifdef __cplusplus
}
#endif

#endif /* MQTT_INTERFACE_H */
