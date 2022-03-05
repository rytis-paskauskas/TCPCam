# TCPCam: remote control your ESP32 camera
FreeRTOS request-response IoT server, implementing multiple concurrent client connections, image streaming and on-demand image capture requests over TCP.
A simple Linux client is provided for testing purposes.
## Hardware requirements
- ESP32-CAM board. 
  It is possible to build for other ESP32 boards without camera support.
- Wifi connection and (optionally) Internet
- 5V or 3.3V power source (5V, if applicable for a board, works better with WiFi).
## Software prerequisites
This project uses the [*ESP IDF*](https://github.com/espressif/esp-idf "ESP-IDF on Github") Integrated Development Environment.
See the [official tutorial](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#installation "install and setup ESP IDF") for installation options.

GCC or similar, and GNU Make or similar, are needed to build the included client.
## Configure and build TCPCam
### Getting the source code
Clone (or fork) this repository
```sh
git clone https://github.com/rytis-paskauskas/TCPCam
```
### Configuration
```sh
idf.py menuconfig
```
Customize TCPCam-related parameters in the "TCPCam Configuration" section.
### Build types
It is possible to build for ESP32 without camera support using the provided test builds.

The menuconfig's *"Image and Streaming settings > Build type"* section provides several build options. 
The *"Build with ESP32-CAMERA sensor"* option should be chosen for 'production' builds.
The other two build types can be used for testing purposes:
- *"Hello World"* repeatedly transmits a string
- *"Build with a JPEG test binary"* repeatedly transmits a sample image file
### Build and flash
This project for the server follows the standard [ESP-IDF build workflow](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html#build-the-project "ESP IDF build workflow").

The client can be built using the included non-portable `Makefile`. 
The client has been tested on Linux only.
## Client
Saving received images as files or passing them to a HTTP streaming server are the two most likely uses of TCPCam server.

Any suitable client, adaptable to TCPCam's simple [server request protocol](#server-request-protocol), could be used for that.

A simple (and optional) Linux client is provided with this project for testing purposes. 
It is a vanilla implementation using BSD socket library. The GNU Make system *might* be necessary to build it.
```sh
cd client
make
```
## Usage
The server connects to a WiFi AP with provided credentials and listens to incoming connections on a specified port. The WiFi credentials are hard-coded through menuconfig.

A client should first establish the connection with the server, then initiate the request, then handle the data.

All communications are unencrypted and use the same port.

The server accepts multiple client connections and serves each in turn in LIFO order.
### Server request protocol
A server request shall consist of PIN followed by COMMAND sent separately in this order. These two commands are required to initiate the connection. Neither command will be confirmed by server.
If the initiation request is successful, a stream will start arriving on the same port.

Description of initiation commands:
1. PIN code as specified in the "TCP server settings > TCP pin/password" menu
2. CMD, TCPCam currently supports numeric command values:
   - a positive number N>0 will request exactly N frames
   - a zero or a negative number will request a non-stop stream
### Using `TCPCam_client`
The provided Linux client connects with credentials and dumps the payload to `stdout` or to a file. Usage:
```sh
TCPCam_client IP PORT PIN CMD [FNAME]
```
- IP --  server's IP address (menuconfig)
- PORT -- server's port (menuconfig)
- PIN -- pin/password (menuconfig)
- CMD -- number of messages to wait for. Put `0` for non-stop transmission.
- FNAME (*optional*) -- direct the payload to a file instead of `stdout`. 
  Multiple frames are appended.

## TODO
- [ ] better file handling in `TCPCam_client` (multiple files)
- [ ] make a python client

It should also be possible to extend this project to other camera sensors by replacing `esp32cam.c` with an appropriate driver and providing a custom frame generator (this has not been tested).
## FAQ
## Authors
* [Rytis Paškauskas](https://github.com/rytis-paskauskas)
## License
See the LICENCE file.

## Acknowledgments
