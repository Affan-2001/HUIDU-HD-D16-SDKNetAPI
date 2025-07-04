# Dot Matrix LED Panel Communication SDK

This project provides a C++ SDK for communicating with an HD-D16 Dot Matrix LED Panel over UDP. It includes functions to update the display text, color, size and font following the xml user manual, using this code the following commands can be sent to the end device:
- GET DEVICE INFO 
- GET IP INFO 
- SET IP INFO 
- POWER ON, POIWER OFF, REBOOT.

## Features

- **UDP Communication**: creates a UDP socket to recieve commands which are then parsed. The connection with the led uses TCP.
- **Event Core**: Thread-safe event-driven architecture for network sessions.
- **Utilities**: Includes TinyXML2 for XML parsing, ArduinoJson for JSON, and MD5 hashing.
- **Cross-Platform Codebase**: Primarily targets Windows, but uses portable C++ where possible.


## Getting Started

### Environment Setup

- Windows OS
- [CMake](https://cmake.org/) 3.12 or higher
- [MSYS2](https://www.msys2.org/) (for GCC/g++)
- Visual Studio Code (recommended)
- The LED panel hardware (HD-D16 or compatible)

### Building

1. **Clone the repository**
   ```sh
   git clone <https://github.com/Affan-2001/HUIDU-HD-D16-SDKNetAPI.git>
   cd SDKNetApi
   ```

2. **Configure and build with CMake**
   ```sh
   Add MSYS2 UCRT binaries to your system PATH:
   $env:Path += ";C:\msys64\ucrt64\bin"

#### METHOD 1 (MANUAL BUILDING)
   mkdir build
   cd build
   cmake .. -G "Unix Makefiles" -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe
   cmake --build .

#### METHOD 2 (AUTOMATIC USING CMAKE EXTENSION IN VSCODE)
  PRESS "CONTROL + SHIFT + P"
  CMake: Configure
  Select "MinGW Makefiles" as the generator 
  Choose "GCC for MSYS2 UCRT64" as the compiler
  CMake: Build
   ```

   The executable will be placed in the `Debug` directory namely SDKNetApp.exe.

### Running

- Edit `src/main.cpp` to set the correct `SERVER_IP` and `SERVER_PORT` for your network.
- Run the built executable. It will create a UDP socket to listen for messages.

## Project Structure

- `src/main.cpp` — Main application logic (UDP server, JSON parsing, command dispatch).
- `ICApi.h` — SDK header for device communication.
- `src/HCatNet/Tool/` — Utility libraries (TinyXML2, ArduinoJson, MD5, etc.).
- `include/` — Public headers.
- `CMakeLists.txt` — Build configuration.
- `external` — tinyxml library added as git submodule. So no need to download it externally. 

## Example JSON Command

```json
{
  "ServertoDevice": {
    "Type": "Command",
    "Device": "Gate In",
    "IP": "192.168.10.60",
    "Port": 10001,
    "Data": [
      { "Name": "Data", "Value": "Welcome!" }
    ]
  }
}
```

## Supported Commands

- **Display Text**: Update gate/gantry display with custom messages.
- **Brightness**: Set display brightness.
- **Power**: Turn display on/off or reboot.
- **Network**: Get or set device IP.
- **Firmware Info**: Query device firmware and configuration.

## Customization

- Edit the `SERVER_IP` and `SERVER_PORT` macros to match your network.
- Add device IPs to the `IPDB` vector for validation.
- Extend the JSON parsing logic in `deserializeJson()` to support new commands.

## License

- SDK code: MIT License
- TinyXML2: zlib License
- ArduinoJson: MIT License

See source files for details.

## Acknowledgements

- [TinyXML2](https://github.com/leethomason/tinyxml2)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [nlohmann/json](https://github.com/nlohmann/json)

---

**Note:**  
All network and SDK operations are performed synchronously. For production use, consider adding more robust error handling and logging.