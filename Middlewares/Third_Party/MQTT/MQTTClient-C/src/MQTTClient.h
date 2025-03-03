/*******************************************************************************
 * Copyright (c) 2014, 2017 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander/Ian Craggs - initial API and implementation and/or initial documentation
 *    Ian Craggs - documentation and platform specific header
 *    Ian Craggs - add setMessageHandler function
 *******************************************************************************/

 #ifndef MQTTCLIENT_CONFIG_H
 #define MQTTCLIENT_CONFIG_H
 
 /* Undefine SUCCESS to avoid conflicts */
 #ifdef SUCCESS
 #undef SUCCESS
 #endif
 
 #include "Timer.h"
 #include "MQTTInterface.h"
 
 #if !defined(__MQTT_CLIENT_C_)
 #define __MQTT_CLIENT_C_
 
 #if defined(__cplusplus)
  extern "C" {
 #endif
 
 #if defined(WIN32_DLL) || defined(WIN64_DLL)
   #define DLLImport __declspec(dllimport)
   #define DLLExport __declspec(dllexport)
 #elif defined(LINUX_SO)
   #define DLLImport extern
   #define DLLExport  __attribute__ ((visibility ("default")))
 #else
   #define DLLImport
   #define DLLExport
 #endif
 
 #include "MQTTPacket.h"
 #include "stdio.h"
 
 #if defined(MQTTCLIENT_PLATFORM_HEADER)
   /* Convert MQTTCLIENT_PLATFORM_HEADER value into a string constant */
   #define xstr(s) str(s)
   #define str(s) #s
   #include xstr(MQTTCLIENT_PLATFORM_HEADER)
 #endif
 
 #define MAX_PACKET_ID 65535 /* according to the MQTT specification - do not change! */
 
 #if !defined(MAX_MESSAGE_HANDLERS)
   #define MAX_MESSAGE_HANDLERS 5 /* redefinable - how many subscriptions do you want? */
 #endif
 
 enum QoS { QOS0, QOS1, QOS2, SUBFAIL = 0x80 };
 
 /* all failure return codes must be negative */
 #ifdef SUCCESS
 #undef SUCCESS
 #endif
 enum returnCode { BUFFER_OVERFLOW = -2, FAILURE = -1, MQTT_SUCCESS = 0 };
 
 /* The Platform specific header must define the Network and Timer structures and functions
  * which operate on them.
  * 
  * typedef struct Network {
  *    int (*mqttread)(Network*, unsigned char* read_buffer, int, int);
  *    int (*mqttwrite)(Network*, unsigned char* send_buffer, int, int);
  * } Network;
  */
 
 /* The Timer structure must be defined in the platform specific header,
  * and have the following functions to operate on it.
  */
 extern void TimerInit(Timer*);
 extern char TimerIsExpired(Timer*);
 extern void TimerCountdownMS(Timer*, unsigned int);
 extern void TimerCountdown(Timer*, unsigned int);
 extern int TimerLeftMS(Timer*);
 
 typedef struct MQTTMessage {
     enum QoS qos;
     unsigned char retained;
     unsigned char dup;
     unsigned short id;
     void *payload;
     size_t payloadlen;
 } MQTTMessage;
 
 typedef struct MessageData {
     MQTTMessage* message;
     MQTTString* topicName;
 } MessageData;
 
 typedef struct MQTTConnackData {
     unsigned char rc;
     unsigned char sessionPresent;
 } MQTTConnackData;
 
 typedef struct MQTTSubackData {
     enum QoS grantedQoS;
 } MQTTSubackData;
 
 typedef void (*messageHandler)(MessageData*);
 
 typedef struct MQTTClient {
     unsigned int next_packetid,
                  command_timeout_ms;
     size_t buf_size,
            readbuf_size;
     unsigned char *buf,
                   *readbuf;
     unsigned int keepAliveInterval;
     char ping_outstanding;
     int isconnected;
     int cleansession;
 
     struct MessageHandlers {
         const char* topicFilter;
         void (*fp)(MessageData*);
     } messageHandlers[MAX_MESSAGE_HANDLERS];  /* Indexed by subscription topic */
 
     void (*defaultMessageHandler)(MessageData*);
 
     Network* ipstack;
     Timer last_sent, last_received;
 #if defined(MQTT_TASK)
     Mutex mutex;
     Thread thread;
 #endif
 } MQTTClient;
 
 #define DefaultClient {0, 0, 0, 0, NULL, NULL, 0, 0, 0}
 
 /**
  * Create an MQTT client object.
  * @param client Pointer to the MQTTClient object.
  * @param network Pointer to the Network object.
  * @param command_timeout_ms Timeout value in milliseconds.
  * @param sendbuf Buffer for sending MQTT packets.
  * @param sendbuf_size Size of the send buffer.
  * @param readbuf Buffer for reading MQTT packets.
  * @param readbuf_size Size of the read buffer.
  */
 DLLExport void MQTTClientInit(MQTTClient* client, Network* network, unsigned int command_timeout_ms,
                                 unsigned char* sendbuf, size_t sendbuf_size, unsigned char* readbuf, size_t readbuf_size);
 
 /** MQTT Connect - send an MQTT CONNECT packet and wait for a CONNACK.
  *  The network object must be connected to the network endpoint before calling this.
  *  @param options - CONNECT options.
  *  @return success code.
  */
 DLLExport int MQTTConnectWithResults(MQTTClient* client, MQTTPacket_connectData* options,
                                       MQTTConnackData* data);
 
 /** MQTT Connect - send an MQTT CONNECT packet and wait for a CONNACK.
  *  @param options - CONNECT options.
  *  @return success code.
  */
 DLLExport int MQTTConnect(MQTTClient* client, MQTTPacket_connectData* options);
 
 /** MQTT Publish - send an MQTT PUBLISH packet and wait for acknowledgements.
  *  @param topic The topic to publish to.
  *  @param message The MQTT message.
  *  @return success code.
  */
 DLLExport int MQTTPublish(MQTTClient* client, const char* topic, MQTTMessage* message);
 
 /** MQTT SetMessageHandler - set or remove a per-topic message handler.
  *  @param topicFilter The topic filter for which the message handler is set.
  *  @param messageHandler Pointer to the message handler function, or NULL to remove.
  *  @return success code.
  */
 DLLExport int MQTTSetMessageHandler(MQTTClient* c, const char* topicFilter, messageHandler messageHandler);
 
 /** MQTT Subscribe - send an MQTT SUBSCRIBE packet and wait for SUBACK.
  *  @param topicFilter The topic filter to subscribe to.
  *  @param messageHandler Pointer to the message handler.
  *  @return success code.
  */
 DLLExport int MQTTSubscribe(MQTTClient* client, const char* topicFilter, enum QoS, messageHandler);
 
 /** MQTT Subscribe with results - send an MQTT SUBSCRIBE packet and wait for SUBACK.
  *  @param topicFilter The topic filter to subscribe to.
  *  @param messageHandler Pointer to the message handler.
  *  @param data Pointer to MQTTSubackData to receive the granted QoS.
  *  @return success code.
  */
 DLLExport int MQTTSubscribeWithResults(MQTTClient* client, const char* topicFilter, enum QoS, messageHandler, MQTTSubackData* data);
 
 /** MQTT Unsubscribe - send an MQTT UNSUBSCRIBE packet and wait for UNSUBACK.
  *  @param topicFilter The topic filter to unsubscribe from.
  *  @return success code.
  */
 DLLExport int MQTTUnsubscribe(MQTTClient* client, const char* topicFilter);
 
 /** MQTT Disconnect - send an MQTT DISCONNECT packet and close the connection.
  *  @return success code.
  */
 DLLExport int MQTTDisconnect(MQTTClient* client);
 
 /** MQTT Yield - process incoming MQTT packets.
  *  @param time Time in milliseconds to yield.
  *  @return success code.
  */
 DLLExport int MQTTYield(MQTTClient* client, int time);
 
 /** MQTT isConnected - Macro to check if the client is connected.
  *  @param client The MQTT client.
  *  @return Non-zero if connected, zero otherwise.
  */
 #define MQTTIsConnected(client) ((client)->isconnected)
 
 #if defined(MQTT_TASK)
 /** MQTT StartTask - start the MQTT background thread.
  *  After calling this, MQTTYield should not be used.
  *  @return success code.
  */
 DLLExport int MQTTStartTask(MQTTClient* client);
 #endif
 
 #if defined(__cplusplus)
      }
 #endif
 
 #endif /* __MQTT_CLIENT_C_ */
 #endif /* MQTTCLIENT_CONFIG_H */
 