<div align="center">
<h1> SDTP library </h1>
<img src="doc/img/sdtp_logo.png" width="500">
</div>

>[!WARNING]
>
>Work in progress. Not ready for any use yet.

# Introduction
**libsdtp** is an open-source library implementing **SDTP** protocol. <br>
**Structured Data Transfer Protocol** (**SDTP**) is a project which aims to create a **flexible**, and **user-friendly** hardware-level data transmission protocol for physical data exchange between electronic devices. <br>
This protocol is suitable for **microcontrollers**, **sensors**, and **input/output devices**. <br>
**SDTP** implements a peer-to-peer (**P2P**) network architecture. **Client-server** and **Master-Slave** architecture also can be implemented at the software level. <br>

**libsdtp** is written on **C** with minimal use of third-party libraries.<br>
Code is fully documented which makes **libsdtp** pretty easy to use. <br>
I'm developing this protocol mainly for my college project, but feel free to use it! <br>

# Architecture
**libsdtp** consists of two isolated layers: **API** and **HAL (drivers)**. <br>
This component separation separates libsdtp from specific platforms and physical data transmission implementations, improving portability and flexibility.

### API Layer
**API layer** contains **software interface specification and processing logic**. Developers interact directly only with this layer. It implements **packet reading, creation, and transmission**. API layer eliminates the need for manual packet serialization/deserialization, signal handling, checksum generation, buffer handling, and etc.

### Hardware Abstraction Layer
**Hardware Abstraction Layer layer** contains only **platform-specific implementation** and it's separated from the main application code. It implements raw data reception/transmission from physical channels.

# Packet Structure

Each packet in the **SDTP protocol** consists of four main components:
1. **SoH (Start of Header) Byte**: 0x02
2. **Header**
3. **Body**
4. **EoT (End of Transmission) Byte**: 0x04

**Packet information and transmitted data are stored in only two components:**
### Header
- **Size**: 16 bytes
- **Contents**:
  - **Packet ID** (`sdtp_packet_header_t.id`): 4 bytes
  - **Data Size** (`sdtp_packet_header_t.data_size`): 4 bytes
  - **Packet Type** (`sdtp_packet_header_t.type`): 4 bytes
  - **Checksum** (`sdtp_packet_header_t.checksum`): 4 bytes

### Body
- **Size**: Defined by `sdtp_packet_header_t.data_size`
- **Contents**: Raw byte stream of `sdtp_packet_header_t.data_size` bytes

# Usage
Below is a small code example for **ESP32** <br>
This example sends "Hello SDTP" packet and reads any incoming packets:
```
#include <api/libsdtp.h>

#include <esp_random.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define OUTPUT_PIN GPIO_NUM_2
#define INPUT_PIN GPIO_NUM_4

sdtp_instance_t* instance = NULL;

void send_packet() {
  // Create payload with text and construct a new packet.
	const char payload[] = "Hello SDTP";
	sdtp_packet_t* packet = sdtp_construct_packet(payload, SDTP_DATA_PACKET);

  // If construction was successful write it to the output buffer and clear memory.
	if (packet) {
		sdtp_write_packet(instance, packet);
		sdtp_packet_free(packet);
	}
}

void read_packet() {
  // Read any incoming packets.
	sdtp_packet_t* packet = sdtp_read_packet(instance, SDTP_READ_PARTIAL);

  // If there's any get payload and process it.
	if (packet) {
		char* msg = sdtp_get_char_data(packet);
		// Process msg...
		sdtp_packet_free(packet);
		free(msg);
	}
}

// Entry point
void app_main() {
  // Configure SDTP connection.
	const sdtp_config_t config = {
		.input_bus_pin = INPUT_PIN,
		.output_bus_pin = OUTPUT_PIN,
		.buffer_size = 4096,
		.baud_rate = 115200,
		.device_id = esp_random(),
		.device_type = SDTP_CONTROLLER
	};

  // Create a new SDTP instance.
  instance = sdtp_instance_create(&config);

  // Send and recieve packets.
  send_packet();
  read_packet();

  // Clear memory and exit.
  sdtp_instance_close(instance);
}
```
