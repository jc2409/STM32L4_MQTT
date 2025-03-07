# STM32L4 MQTT Client 

## Overview
This project demonstrates how to run an MQTT client on an STM32 board (**B‑L475E‑IOT01A1**) using the **ES‑WiFi** module. The goal was to connect to a public Mosquitto MQTT broker, subscribe to a topic, and publish messages periodically. To achieve this, the **Eclipse Paho Embedded MQTT Client** library was integrated with ST board’s Wi‑Fi interface.

## Process and Key Changes

### 1. Integrating the MQTT Client Library

#### Library Selection
- **Eclipse Paho Embedded MQTT C client** library.  
- `MQTTClient` and `MQTTPacket` files were required.  
- These files were placed under the project’s **middleware** directory.

#### Conflict Resolution
- The HAL or board support packages sometimes define a symbol named `SUCCESS`.  
- Renamed the MQTT client’s return code `SUCCESS` to `MQTT_SUCCESS` to avoid conflicts.

### 2. Implementing the Network Interface Wrappers

#### Network Structure Setup
- The MQTT client requires a `Network` structure containing function pointers for reading and writing data.  
- Created wrapper functions:  
  - `mqtt_network_read()`  
  - `mqtt_network_write()`  
  - `mqtt_network_disconnect()`  
- These bridge the MQTT client library with the ES‑WiFi module.

### 3. Adapting the Timer Functions

#### Timer Interface
- The MQTT library relies on a timer interface for handling timeouts and keep‑alive mechanisms.  
- Provided implementations for:  
  - `TimerInit()`  
  - `TimerCountdownMS()`  
  - `TimerCountdown()`  
  - `TimerIsExpired()`  
  - `TimerLeftMS()`  
- These functions use the `HAL_GetTick()` as a time base.

### 4. Modifying the Main Application

#### Wi‑Fi Connection
- In the main file, it uses the custom `wifi_connect()` function to:  
  - Initialize the Wi‑Fi module  
  - Connect to the configured access point  
  - Retrieve an IP address

#### Broker Connection
1. Resolve the Mosquitto broker hostname to an IP address using `WIFI_GetHostAddress()`.  
2. Establish a TCP client connection with `WIFI_OpenClientConnection()`.

#### MQTT Client Initialization
1. Populate a `Network` structure with:  
   - The socket (opened by the Wi‑Fi module)  
   - The network read/write wrapper functions  
2. Initialize the MQTT client using `MQTTClientInit()`.  
3. Set connection parameters (MQTT version, client ID, keep‑alive interval, clean session flag).  
4. Call `MQTTConnect()`.

#### Subscription and Publishing
- Once connected, subscribe to the topic `sensor/data` via `MQTTSubscribe()`.  
- Implement a periodic publishing task that sends a message every second on the `sensor/data` topic.  
  - If using FreeRTOS, this can be done in a dedicated task with `osDelay(1000)`.  
  - Otherwise, use a loop in `main()` with `HAL_Delay(1000)`.

## Testing and Debugging

### Command‑Line Tools
- To confirm that the Mosquitto broker is running and reachable, we used `mosquitto_sub`:
```bash
mosquitto_sub -h test.mosquitto.org -p 1883 -t "sensor/data"
```
- This command shows any messages published on the test topic by the board.

### Testing Environment
- **IDE**: Keil uVision5 (Arm Compiler 5)
- **Network**: Android Hotspot (WPA2)
- **Board**: B-L745E-IOT01A1
- **Dependency**: Paho Embedded C MQTT / Mosquitto
- **Terminal**: Tera Term (Baud Rate 115200, Receive: LF, Transmit: CR+LF)

### Mosquitto Broker Configuration
1. Navigate to `/etc/mosquitto/conf.d`:
```bash
cd /etc/mosquitto/conf.d
```

2. Edit (or create) `listener.conf`:
```bash
nano listener.conf
```

3. Add the following lines:
```bash
listener 1883
allow_anonymous true
```

### Modifications in `main.c`
- Edit the **Wi‑Fi SSID** and **password**.
- Change the **hostname** accordingly (if connecting to a different broker or cloud instance).

### Compiler Requirement
- This project uses **Arm Compiler 5**.
- If it’s not added to your Keil uVision, you can install it:
    - **Arm Compiler 5** (https://developer.arm.com/documentation/107879/5-06u7/?lang=en)
    - **Installation Guide** (https://developer.arm.com/documentation/ka005073/latest/)


## Connecting to a Cloud (Without TLS)
### AWS EC2 Example
1. **Create an EC2 Instance** on AWS (Ubuntu 22.04).
2. **Install MQTT Broker**:
```bash
sudo apt-get update
sudo apt-get install mosquitto
sudo apt-get install mosquitto-clients
```
3. **Edit the inbound rule** in the Security Group to open port 1883.
4. Change the `MQTT_BROKER_HOST` in `main.c` to your **Public IPv4 DNS** of the EC2 instance.
- On your EC2 instance, subscribe to the topic with:
```bash
mosquitto_sub -h <MQTT_BROKER_HOST> -p 1883 -t "sensor/data"
```
(If you want to run the broker on a port other than 1883, adjust the above commands and your security group rules accordingly.)

*Note*: This project does not demonstrate TLS/SSL configuration. Ensure your IoT platform or broker configurations meet your security requirements for production environments.

## License
This project does not include a specific license file. However, the included Eclipse Paho Embedded MQTT files follow the Eclipse Public License. Please review the original library license for full details.

