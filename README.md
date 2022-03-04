# TCPCam: remote control your camera

A fully functional IoT application implementing multiple concurrent client connections, image streaming and image capture on request over TCP.
A simple Linux client (in C) is provided for testing purposes.

## Hardware requirements
- ESP32-CAM board. It is possible to build for ESP32 boards without camera support using one of two provided test builds
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
See the `TCPCam Configuration` menu.
We have two modes for testing the TCP server configurable from the `Image and Streaming settings > Build type` menu:
- `Hello World` repeatedly transmits the `Hello World` string
- `Build with a JPEG test binary` repeatedly transmits a sample image

### Build and flash
The buid of this app follows the [standard ESP-IDF workflow](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#step-6-connect-your-device "ESP IDF build workflow").

### Build types 
The `menuconfig`s "Image and Streaming settings > Build type" provides several build options.
The build using the 'esp32-camera' module should be chosen for 'production' builds. The other two build types can be used for testing purposes.

## Build the client
To process the images (save as files, stream over HTTP, etc.) on the client side, a dedicated app will be required.

A simple (and optional) BSD socket library Linux client is provided with this project. It is built using the GNU Make system.
```sh
cd client; make
```

## Usage
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
