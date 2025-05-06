# Dot Matrix LED Panel Communication SDK

This project provides a C++ SDK for communicating with an HD-D16 Dot Matrix LED Panel over UDP. It includes utilities for networking, JSON, XML parsing, and more, and is designed for Windows environments.

## Features

- **UDP Communication**: Send and receive messages to/from the LED panel.
- **Event Core**: Thread-safe event-driven architecture for network sessions.
- **Utilities**: Includes TinyXML2 for XML parsing, ArduinoJson for JSON, and MD5 hashing.
- **Cross-Platform Codebase**: Primarily targets Windows, but uses portable C++ where possible.

## Getting Started

### Prerequisites

- Windows OS
- [CMake](https://cmake.org/) 3.12 or higher
- [MSYS2](https://www.msys2.org/) (for GCC/g++)
- Visual Studio Code (recommended)
- The LED panel hardware (HD-D16 or compatible)

### Building

1. **Clone the repository**
   ```sh
   git clone <your-repo-url>
   cd SDKNetApi
   ```

2. **Configure and build with CMake**
   ```sh
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

   The executable will be placed in the `Debug` directory.

### Running

- Edit `src/main.cpp` to set the correct `SERVER_IP` and `SERVER_PORT` for your network.
- Run the built executable. It will listen for UDP messages and print received data.

## Project Structure

- `src/main.cpp` — Main application logic (UDP server, JSON parsing, command dispatch).
- `ICApi.h` — SDK header for device communication.
- `src/HCatNet/Tool/` — Utility libraries (TinyXML2, ArduinoJson, MD5, etc.).
- `include/` — Public headers.
- `CMakeLists.txt` — Build configuration.

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