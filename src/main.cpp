#include <iostream>
#include "ICApi.h"
#include <winsock2.h>
#include <ArduinoJson.h>
#include <windows.h>
#include <vector>
#include <algorithm>
#include <atomic>
#include <thread>
// #define _CRTDBG_MAP_ALLOC
// #include <crtdbg.h>

using namespace std;
using namespace ArduinoJson::V731HB42;

// Global event core pointer so we can call Quit from callbacks.
static HEventCore *core = nullptr;

std::string Type;
std::string Device;
std::string Device_IP;
int Device_Port = 0;
volatile bool g_shutdown = false;

string Gate_TL = "";

const char* Name = nullptr;
const char* ScreenFunc= nullptr;
std::vector<std::string> IPDB = {"192.168.19.61"};

#define SERVER_IP           "192.168.19.62" 
#define SERVER_PORT         5000            // The port to listen on (must match server's port)

#define Gate_IN_Greeting    "پی ایس او فیصل آباد ٹرمینل میں خوش آمدید"
#define Gate_OUT_Greeting    "پی ایس او فیصل آباد ٹرمینل خدا حافظ"

#define XML_Gate            "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" \
                            "<sdk guid=\"b04f672504da67f19d48edb3b173c15f\">\n" \
                            "  <in method=\"AddProgram\">\n" \
                            "    <screen timeStamps=\"1744104321754\">\n" \
                            "      <program type=\"normal\" id=\"0\" guid=\"b41bbdb7-1b15-4360-9127-2f25dc3ec2b9\" name=\"\">\n" \
                            "        <playControl count=\"1\" disabled=\"false\" />\n" \
                            "        <area guid=\"78876cc1-b5f7-4b9c-8340-a50fa895a5ef\" name=\"\" alpha=\"255\">\n" \
                            "          <rectangle x=\"0\" y=\"5\" width=\"256\" height=\"64\" />\n" \
                            "          <resources>\n" \
                            "            <text guid=\"766942a6-5b3f-46e0-ac44-6560c3259063\" name=\"\" singleLine=\"false\">\n" \
                            "              <style align=\"center\"/>\n" \
                            "              <string>%s</string>\n" \
                            "              <font name=\"Arial\" size=\"22\" color=\"#ff0000\" bold=\"true\" italic=\"false\" underline=\"false\" />\n" \
                            "              <effect in=\"0\" inSpeed=\"4\" out=\"20\" outSpeed=\"4\" duration=\"0\" />\n" \
                            "            </text>\n" \
                            "          </resources>\n" \
                            "        </area>\n" \
                            "        <area alpha=\"255\" guid=\"areaclockGuid1\">\n" \
                            "          <rectangle x=\"0\" y=\"65\" width=\"256\" height=\"60\"/>\n" \
                            "          <resources>\n" \
                            "            <text singleLine=\"true\">\n" \
                            "              <style align=\"center\" />\n" \
                            "              <font name=\"Arial\" size=\"48\" color=\"#ff0000\" bold=\"true\" italic=\"false\" underline=\"false\" />\n" \
                            "              <effect duration=\"100\" in=\"0\" inSpeed=\"4\" outSpeed=\"4\" out=\"20\"/>\n" \
                            "              <string>%s</string>\n" \
                            "            </text>\n" \
                            "          </resources>\n" \
                            "        </area>\n" \
                            "      </program>\n" \
                            "    </screen>\n" \
                            "  </in>\n" \
                            "</sdk>"

// Callback function to handle received data
static void ReadData(HSession *currSession, const char *data, huint32 len, void *userData) {
    printf("--------------Read Data:--------------\n");   
    printf("  currSession: %p,\t userData: %p,\t len: %u\n", (void*)currSession, userData, (unsigned int)len);
    printf("  data:        %s\n", data);
    printf("-------------------------------------\n");
    Quit(core);
}

// Callback function to handle error codes
static void DebugLog(HSession *currSession, const char *log, huint32 len, void *userData) {
    printf("--------------Debug log:--------------\n");
    printf("  currSession: %p,\t userData: %p,\t len: %u\n", (void*)currSession, userData, (unsigned int)len);
    printf("  log:         %s\n", log);   
    printf("-------------------------------------\n");
    fflush(stdout);
    //Quit(core);
}

// Callback function to handle connection status
static void NetStatus(HSession *currSession, eNetStatus status, void *userData) {
    printf("--------------NetStatus:--------------\n");
    printf("  currSession: %p,\t userData: %p\n", (void*)currSession, userData);
    printf("  NetStatus:   %d\n", status); 
    printf("-------------------------------------\n");
    
    switch (status) {
    case kConnect:
        printf("NetStatus : Device Connected\n");
        break;
    case kDisconnect:
        printf("NetStatus : Device Disconnect\n");
        break;
    default:
        break;
    }
    //Quit(core);
}

// Callback function to handle detected device information
// static void DeviceInfo(HSession *currSession, const char *id, huint32 idLen, const char *ip, huint32 ipLen, const char *readData, huint32 dataLen, void *userData) {
//     printf("id[%s], ip[%s]\n", id, ip);  // Print device ID and IP address
// }

/*
static void DeviceInfo(HSession *currSession, const char *id, huint32 idLen, const char *ip, huint32 ipLen, const char *readData, huint32 dataLen, void *userData) {
    printf("--------------DeviceInfo:--------------\n");
    printf("  currSession: %p\n", (void*)currSession);
    printf("  id:          %.*s\n", (int)idLen, id);
    printf("  idLen:       %u\n", (unsigned int)idLen);
    printf("  ip:          %.*s\n", (int)ipLen, ip);
    printf("  ipLen:       %u\n", (unsigned int)ipLen);
    printf("  readData:    %.*s\n", (int)dataLen, readData);
    printf("  dataLen:     %u\n", (unsigned int)dataLen);
    printf("  userData:    %p\n", userData);
    printf("-------------------------------------\n");
}
*/

// Graceful shutdown function when Ctrl+C is pressed  or window is closed
BOOL WINAPI ConsoleHandler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_CLOSE_EVENT) {
        g_shutdown = true;
        return TRUE;  // We handled the event
    }
    return FALSE;  // Let other handlers process this event
}

// Function to upload the text to the display
void Display(HSession* session) {
    char finalXml[4096];
    const char* xmlTemplate = nullptr;
    
    if (Device == "Gate In")
    {
        xmlTemplate = XML_Gate; //Macro to define the XML template
        snprintf(finalXml, sizeof(finalXml), xmlTemplate, Gate_IN_Greeting, Gate_TL.c_str()); // Construct the XML command
    }
    else if (Device == "Gate Out")
    {
        xmlTemplate = XML_Gate; //Macro to define the XML template
        snprintf(finalXml, sizeof(finalXml), xmlTemplate, Gate_OUT_Greeting, Gate_TL.c_str()); // Construct the XML command  
    }
    else {
        printf("Unknown Device Type\n");
        return;
    }
    
    if (Connect(session, Device_IP.c_str(), Device_Port)) {        
        if (SendSDK(session, finalXml, strlen(finalXml))) {            
            // cout<<finalXml<<endl;
            printf("Text Updated Successfully.\n");
        } else {
            printf("Failed to update display.\n");
        }
    } else {
        printf("Connection to the device failed.\n");
    }
    //Exec(core); // Blocks the main thread and waits for Quit to be called from ReadData callback
}

int deserializeJson(string json) {
    int i = 0;
    StaticJsonDocument<256> doc;

    // Attempt to parse the JSON string
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        cout << "deserializeJson() failed: " << error.c_str() << endl;
        return 0;
    }
    
    JsonObject ServertoDevice = doc["ServertoDevice"];

    Type = ServertoDevice["Type"].as<std::string>();
    Device = ServertoDevice["Device"].as<std::string>();
    Device_IP = ServertoDevice["IP"].as<std::string>();
    Device_Port = ServertoDevice["Port"];

    if (std::find(IPDB.begin(), IPDB.end(), Device_IP) == IPDB.end()) {
        std::cout << "IP not found in the array!" << std::endl;
        return 0;
    }
    
    JsonArray dataArray = ServertoDevice["Data"].as<JsonArray>();  // Store the temporary in a variable
    for (JsonObject data : dataArray) // Iterate over the array of objects
    {
        Name = data["Name"];        // "Bay 1", ...   in the case of when configuration is received it will store brightness/power on/ power off.
        const char* Value = data["Value"];
        
        // Validate: check if Value exists or is null
        if (Value == nullptr || data["Value"].isNull()) {
            printf("Warning: 'Value' for item at index: %d is null; replacing with 0 \n",i);
            Value = "0"; // Set Value to an empty string if null
        }
    Gate_TL = Value;
      
    }
    return 1;
}

void cleanup_resources(SOCKET sock, HSession* session) {
    // Check if socket is valid before closing
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
    }
    
    // Clean up Winsock
    WSACleanup();
    
    // Clean up session if it exists
    if (session) {
        Disconnect(session);
        FreeNetSession(session);
    }
    
    // Clean up event core if it exists
    if (core) {
        FreeEventCore(core);
    }

}

int main() {    
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server_addr, client_addr;
    char buffer[2048];
    int addr_len = sizeof(client_addr);
    // DWORD lastExecTime = GetTickCount(); // Initialize timer
    // const DWORD intervalMs = 90 * 1000;
    
    // Enable automatic leak detection at program exit
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // Set up console handler
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        cerr << "Failed to set control handler!" << endl;
        return -1;
    }
    
    // Create the event core.
    core = CreateEventCore();
    std::atomic<bool> running = true;

    std::thread eventLoopThread([&] {
        while (running) {
            Exec(core);
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
    }); 

    if (!core) {
        cerr << "Failed to create event core!" << endl;
        cleanup_resources(INVALID_SOCKET, nullptr);  // No resources allocated yet
        return -1;
    }

    // Create a network session using the SDK2 protocol.
    HSession *session = CreateNetSession(core, kSDK2);
    if (!session) {
        cerr << "Failed to create network session!" << endl;
        cleanup_resources(INVALID_SOCKET, nullptr);  // Core created but no session
        return -1;
    }
    
    // Set the callbacks.
    SetNetSession(session, kReadyReadFunc, reinterpret_cast<void *>(ReadData));
    SetNetSession(session, kReadyReadUserData, session);
    SetNetSession(session, kNetStatusFunc, reinterpret_cast<void *>(NetStatus));
    SetNetSession(session, kNetStatusUserData, session);
    SetNetSession(session, kDebugLogFunc, reinterpret_cast<void *>(DebugLog));
    
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        cerr << "WSAStartup failed!" << endl;
        cleanup_resources(INVALID_SOCKET, session);  // Core and session created
        return -1;
    }
    
    // Create socket (UDP)
    sock = socket(AF_INET, SOCK_DGRAM, 0);  // UDP socket
    if (sock == INVALID_SOCKET)
    {
        cerr << "Socket creation failed!" << endl;
        cleanup_resources(INVALID_SOCKET, session);  // Winsock initialized but no socket
        return -1;
    }
    
    // Set socket to non-blocking mode
    u_long mode = 1;  // 1 = non-blocking, 0 = blocking
    if (ioctlsocket(sock, FIONBIO, &mode) != 0) {
        std::cerr << "Failed to set non-blocking mode: " << WSAGetLastError() << std::endl;
        cleanup_resources(INVALID_SOCKET, session);
        return -1;
    }
    
    // Set up the server address structure (bind to any available IP address)
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);  // The port to listen on
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);  // 0.0.0.0 listens on all available network interfaces
    
    // Bind the socket to the specified IP address and port
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) 
    {
        cerr << "Bind failed!" << endl;
        closesocket(sock);
        cleanup_resources(sock, session);  // All resources created
        return -1;
    }
    printf("Socket created and bound to %s : %d \n",SERVER_IP, SERVER_PORT);
    
    string lastMessage;  // Store the last received message

    // _CrtMemState sOld;
    // _CrtMemState sNew;
    // _CrtMemState sDiff;
    // _CrtMemCheckpoint(&sOld); //take a snapshot

    while (!g_shutdown) 
    {    
        int n = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_len);    
        if (n > 0){
            buffer[n] = '\0';  // Null-terminate the received string
            //printf("\nReceived new message: %s\n", buffer);                  
            // Check if the new message is different from the last one
            if (buffer != lastMessage) 
            {
                printf("\nReceived new message:%s \n", buffer);
                if (deserializeJson(buffer) == 0) {
                    printf("Failed to deserialize JSON \n");
                    lastMessage = buffer;  // Update the last received message
                    Sleep(500);  // Sleep for a short duration before retrying
                    continue;  // Skip this iteration if deserialization fails
                }
                
                if (Type == "Command") {
                    Display(session); // Call the Display function to update the display
                }
                lastMessage = buffer;  // Update the last received message
            }
         // Skip this iteration if the buffer is empty
        }
        else if (n == SOCKET_ERROR) {
            int errorCode = WSAGetLastError();
            if (errorCode == WSAEWOULDBLOCK) {
                // No data available, continue polling
                //Sleep(10);  // Small sleep to prevent CPU overuse (optional, see Option 2 for better approach)
                memset(buffer, 0, sizeof(buffer));
                
            } else {
                // Handle actual errors
                printf("recvfrom failed with error code: %d\n", errorCode);
                Sleep(1000);  // Sleep for a short duration before retrying
              
                memset(buffer, 0, sizeof(buffer));
            }
        } else {
            Sleep(500);  // Sleep for a short duration before retrying
            memset(buffer, 0, sizeof(buffer));
        }
        // _CrtMemCheckpoint(&sNew); //take a snapshot 
        // if (_CrtMemDifference(&sDiff, &sOld, &sNew)) // if there is a difference
        // {
        //     OutputDebugString(L"-----------_CrtMemDumpStatistics ---------");
        //     _CrtMemDumpStatistics(&sDiff);
        //     OutputDebugString(L"-----------_CrtMemDumpAllObjectsSince ---------");
        //     _CrtMemDumpAllObjectsSince(&sOld);
        //     OutputDebugString(L"-----------_CrtDumpMemoryLeaks ---------");
        //     _CrtDumpMemoryLeaks();
        // }
       // --- Timer logic to call Exec(core) every intervalMs milliseconds ---
        // DWORD now = GetTickCount();
        // if (now - lastExecTime >= intervalMs) {
        //     Quit(core);       // clean up old one
        //     Exec(core); // Call Exec to process the event core
        //     lastExecTime = now;
        //     printf("Exec called at %lu ms\n", now);
        
        
    }
    running = false;
    Quit(core);
    eventLoopThread.join();
    printf("Exiting the loop \n");
    cleanup_resources(sock, session);
    return 0;
}