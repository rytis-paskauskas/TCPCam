# TCPCam: remote control your camera

A fully functional IoT application implementing multiple concurrent client connections, image streaming and image capture on request over TCP.
A simple Linux client (in C) is provided for testing purposes.

## Hardware requirements
- ESP32-CAM board (with camera sensor). 
  It is possible to build for other ESP32 boards without camera support.
- Wifi connection and (optionally) Internet
- 5V or 3.3V power source (5V, if applicable for a board, works better with WiFi).

## Software prerequisited
This app required the [*ESP IDF*](https://github.com/espressif/esp-idf "ESP-IDF on Github") Integrated Development Environment.
The roadmap for the IDE installation and configuration is described [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#installation-step-by-step "install and setup ESP IDF").

## Configure and build TCPCam

### Getting the source code
Clone (or fork) this repository
```sh
git clone https://github.com/rytis-paskauskas/TCPCam
```

### Parameter configuration
```sh
idf.py menuconfig
```
Customize TCPCam-related parameters in the "TCPCam Configuration" menu.

### Build and flash
This app follows the [standard ESP-IDF workflow](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#step-6-connect-your-device "ESP IDF build workflow").

### Build types 
It is possible to build for ESP32 without camera support using the provided test builds. Also, it should be possible to extend this app to other camera sensors by replacing `esp32cam.c` and providing an appropriate driver and a custom frame generator (not tested).

The `menuconfig`s "Image and Streaming settings > Build type" provides several build options. The build with 'esp32-camera' module should be chosen for 'production' builds. The other two build types can be used for testing purposes:
- `Hello World` repeatedly transmits a `Hello World` string
- `Build with a JPEG test binary` repeatedly transmits a sample image file

## Client
The client will require an application to process received images (save them as file(s), stream over HTTP,...). 
Any client could be used, that is able to send commands and handle incoming packet data (header and payload).


A simple (and optional) Linux client is provided with this project for testing purposes. It is implemented using BSD socket library and is built using the GNU Make system.
```sh
cd client; make
```

## Usage
The server connects to the WiFi access point with provided credentials  listens to connections on the specified port. All communications use the same port. Multiple simultaneous connections (for example from different devices) is possible.

A client should first establish the connection with the server on specified IP address and port (see `menuconfig`s "TCP server settings").
After a successful connection, please send your request.

The request shall consist of PIN followed by COMMAND sent separately in this order. The commands will not be confirmed by server.
If the request is successful, a stream will start arriving on the same port.

The commands are
1. PIN code as specified in `menuconfig`s "TCP server settings > TCP pin/password"
2. CODE, the app supports numeric code values:
   - positive number `N` will request exactly N frames
   - zero or negative number will request a non-stop stream
  
## Client
A simple Linux client using BSD socket library implementation is provided for testing purposes.
This client connects with credentials and dumps the payload to `stdout` or to a file.
Usage:
```sh
TCPCam_client IP PORT PIN REP [FNAME]
```
Here
- IP:   IP address of server
- PORT:  TCP port as set in menuconfig
- PIN: pin/password as set in menuconfig
- REP: number of messages to wait for. Put 0 for indefinite transmission.
- FNAME: *optional* If specified the payload will be directed to a file instead of `stdout`. In case of multiple frames, they will be appended.

## TODO
- [] better file handling in `TCPCam_client` (multiple files)
- [] make a python client

## FAQ

## Authors
* [Rytis Paškauskas](https://github.com/rytis-paskauskas)

## License
See the LICENCE file.

## Acknowledgments
