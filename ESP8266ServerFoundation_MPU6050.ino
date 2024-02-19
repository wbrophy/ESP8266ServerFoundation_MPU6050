/* (Some) credits - those I missed I apologize
 * https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html#uploading-files-to-file-system
 * https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer
 * https://links2004.github.io/Arduino/d4/dd2/class_esp_class.html
 * https://github.com/esp8266/Arduino/tree/master/doc/esp8266wifi
 * https://github.com/esp8266/Arduino/blob/master/doc/esp8266wifi/generic-class.rst#mode
 * https://codebender.cc/library/Adafruit_Sensor#Adafruit_Sensor.h
 * https://github.com/adafruit/Adafruit_MPU6050
 * https://www.arduino.cc/reference/en/libraries/websockets/
 * https://github.com/gilmaimon/ArduinoWebsockets
 * https://protosupplies.com/product/mpu-6050-gy-521-3-axis-accel-gryo-sensor-module/
 * http://playground.arduino.cc/Main/MPU-6050
 * https://lastminuteengineers.com/mpu6050-accel-gyro-arduino-tutorial/
 * https://github.com/jrowberg/i2cdevlib/tree/master/Arduino/MPU6050
 * http://www-robotics.cs.umass.edu/~grupen/503/Projects/ArduinoMPU6050-Tutorial.pdf
 * https://github.com/arduino-libraries/Arduino_JSON/tree/master
 * https://www.youtube.com/watch?v=fREqfdCphRA
 * https://gitlab.com/MrDIYca/code-samples/-/blob/master/mrdiy_websocket_project_example.ino
 * https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html#uploading-files-to-file-system
 * https://www.arduino.cc/reference/en/language/functions/communication/wire/
 * https://github.com/arduino/reference-en/tree/master
 * https://github.com/esp8266/Arduino/tree/master/cores/esp8266
 * https://github.com/esp8266/Arduino/blob/master/libraries/DNSServer/examples/CaptivePortal/CaptivePortal.ino
 * https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer
 * https://www.w3schools.com/js/js_numbers.asp ****JavaScript Numbers are Always 64-bit Floating Point**** 
 * https://www.w3schools.com/js/js_bitwise.asp **** (WORD << 48); (WORD >> 48); to get signed WORD. 
 *  
*/
#include <Arduino_JSON.h>
#include <EEPROM.h>
#include <FS.h> // SPIFFS - included in <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h> // included in <ESP8266WebServer.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include "DefaultPages.h"
#include <Adafruit_MPU6050.h> /*MODIFICATION*/
#include <Adafruit_Sensor.h> /*MODIFICATION*/

// Global Variable(s) / Object(s)
DNSServer dnsServer;
ESP8266WebServer server(80); //**** make the port configurable
WebSocketsServer webSocket = WebSocketsServer(81); //**** make the port configurable
Adafruit_MPU6050 mpu; /*MODIFICATION*/

void setup(void)
{
  JSONVar objWiFiConfiguration;
  JSONVar objDataDevice; /*MODIFICATION*/
  
  // Start Serial Communication interface
  Serial.begin(115200);
  delay(1000);
  Serial.println(strBanner);
  
  Serial.println("\nBegin Setup......");

  //////////////////////////////////
  // Initialize data devices - Start
  // MPU6050 - Start/*MODIFICATION*/
  objDataDevice["device"][0]["type"] = "MPU6050";
  objDataDevice["device"][0]["exists"] = false;
  if (mpu.begin())
    { 
      objDataDevice["device"][0]["exists"] = true;
      mpu.setAccelerometerRange(MPU6050_RANGE_8_G); // MPU6050_RANGE_2_G, MPU6050_RANGE_4_G, MPU6050_RANGE_8_G, MPU6050_RANGE_16_G
      mpu.setGyroRange(MPU6050_RANGE_250_DEG); // 250, 500, 1000, 2000
      mpu.setFilterBandwidth(MPU6050_BAND_21_HZ); // 5, 10, 21, 44, 94, 184, 260
    }
  Serial.println("DataDevices:");
  Serial.println(objDataDevice);
  // MPU6050 - End/*MODIFICATION*/
  // Initialize data devices - End
  ////////////////////////////////
  

  Serial.println("\nExample wifi.cfg:");
  Serial.println(strWiFiCfgExample);
  Serial.println("\nLoad wifi.cfg.");
  Serial.println(readFile("wifi.cfg"));
  objWiFiConfiguration = JSON.parse(String((JSON.parse(readFile("wifi.cfg")))["content"]));
  Serial.println(objWiFiConfiguration);

  Serial.println("\nSet WiFi mode and disconnect any existing connections.");
  WiFi.mode(WIFI_AP_STA); /* WIFI_OFF = 0, WIFI_STA = 1, WIFI_AP = 2, WIFI_AP_STA = 3 */
  WiFi.disconnect();
  delay(1000);

  Serial.println(((bool)objWiFiConfiguration.hasOwnProperty("enableStation") == false || (bool)objWiFiConfiguration["enableStation"] == true) ? "WiFi Station Mode enabled." : "WiFi Station Mode disabled.");
  if ((bool)objWiFiConfiguration.hasOwnProperty("enableStation") == false || (bool)objWiFiConfiguration["enableStation"] == true )
    {
      if (objWiFiConfiguration["WiFiConnection"].length() > 0)
        {
          int x_WiFiConnection = 0;
          while (x_WiFiConnection < objWiFiConfiguration["WiFiConnection"].length())
            {
              if ((bool)objWiFiConfiguration["WiFiConnection"][x_WiFiConnection].hasOwnProperty("connect") == false || (bool)objWiFiConfiguration["WiFiConnection"][x_WiFiConnection]["connect"] == true)
                {
                  Serial.println(WNconnect
                    (
                      (objWiFiConfiguration["WiFiConnection"][x_WiFiConnection].hasOwnProperty("hostname") && String(objWiFiConfiguration["WiFiConnection"][x_WiFiConnection]["hostname"]) != "") ? String(objWiFiConfiguration["WiFiConnection"][x_WiFiConnection]["hostname"]) : DefaultID(),
                      String(objWiFiConfiguration["WiFiConnection"][x_WiFiConnection]["ssid"]),
                      objWiFiConfiguration["WiFiConnection"][x_WiFiConnection].hasOwnProperty("password") ? String(objWiFiConfiguration["WiFiConnection"][x_WiFiConnection]["password"]) : ""
                    ));
                  if (WiFi.isConnected()) { break; }
                }
              x_WiFiConnection++;
            }
            Serial.println(WiFi.isConnected() ? "WiFi Station connected." : "WiFi Station NOT connected.");
        }
      else { Serial.println("No WiFiConnection defined"); }
    }

  Serial.println(((bool)objWiFiConfiguration.hasOwnProperty("enableAP") == false || (bool)objWiFiConfiguration["enableAP"] == true) ? "WiFi AP Mode enabled." : "WiFi AP Mode disabled.");
  if ((bool)objWiFiConfiguration.hasOwnProperty("enableAP") == false || (bool)objWiFiConfiguration["enableAP"] == true )
    {
      Serial.println(APconnect
        (
          (objWiFiConfiguration["softAP"].hasOwnProperty("ssid") == true) && (String(objWiFiConfiguration["softAP"]["ssid"]) != "") ? String(objWiFiConfiguration["softAP"]["ssid"]) : DefaultID(),
          (objWiFiConfiguration["softAP"].hasOwnProperty("password") == true) && (String(objWiFiConfiguration["softAP"]["password"]) != "") ? String(objWiFiConfiguration["softAP"]["password"]) : "",
          (objWiFiConfiguration["softAP"].hasOwnProperty("hidden") == true) && ((bool)objWiFiConfiguration["softAP"]["hidden"] == true) ? true : false,
          (objWiFiConfiguration["softAP"].hasOwnProperty("maxconnection") == true) && ((int)objWiFiConfiguration["softAP"]["maxconnection"] >= 1) && ((int)objWiFiConfiguration["softAP"]["maxconnection"] <= 8) ? (int)objWiFiConfiguration["softAP"]["maxconnection"] : 4
        ));
    }

  Serial.println("Start DNS Server for SoftAP Captive Portal.");
  Serial.println(dnsServer.start(53, "*", WiFi.softAPIP()) ? "DNS Server Started." : "DNS Server NOT Started.");

  Serial.println("\nBegin webSocket server.");
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  Serial.println("\nMAP HTTP Requests to handlers.");
  server.onNotFound(handleNotFound);
  server.on("/", handleRoot);
  server.on("/api/scannetworks", handleScannetworks);
  server.on("/api/readeeprom", handleReadEEPROM);
  server.on("/api/formatspiffs", handleFormatSPIFFS);
  server.on("/api/listspiffs", handleListSPIFFS);
  server.on("/applications/editspiffsfile", handleEditSPIFFSfile);
  server.on("/api/savespiffsfile", handleSaveSPIFFSfile);
  server.on("/api/readspiffsfile", handleReadSPIFFSfile);
  server.on("/api/deletespiffsfile", handleDeleteSPIFFSfile);
  server.on("/api/getdata", handleGetData);
  server.on("/applications/websocket", handleWebSocket);
  server.on("/api/restartesp", handleRestart);

  Serial.println("\nBegin HTTP server.");
  server.begin();
  Serial.println("HTTP server started.");
  Serial.println("\nSetup Complete.");
}

void loop(void)
{
  // Get data from data provider
  String sData = getData();

  // Process Serial Input
  if (Serial.available())
    {
      String strSerialInput = Serial.readString();
      strSerialInput.trim();
      String strCommand = strSerialInput.substring(0,strSerialInput.indexOf(" "));
      strCommand.toLowerCase();
      String strParameters = strSerialInput.substring(strCommand.length()+1);
      strParameters.trim();
      
      Serial.println("Serial Input-> " + strSerialInput);
      if (strCommand == "restart")
        { ESP.restart(); }
      
      if (strCommand == "delete")
        { Serial.println(deleteFile(strParameters)); }
    }
  
  // Process DNS Requests
  dnsServer.processNextRequest();
  
  // Process webSocket
  webSocket.loop();
  webSocket.broadcastTXT(sData);

  // Process webserver
  server.handleClient();
}

/////////////////////////////////////////////////
void handleRoot() 
  { server.send(200, "text/html", strRootHTML); }
/////////////////////////////////////////////////

//////////////////////////////////////////////////////
void handleWebSocket() 
  { server.send(200, "text/html", strWebSocketHTML); }
//////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
void handleGetData()
  { server.send(200, "application/json", String(getData())); }
//////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
void handleListSPIFFS()
  { server.send(200, "application/json", String(ListSPIFFS())); }
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
void handleReadEEPROM() 
  { server.send(200, "application/json", String(ReadEEPROM())); }
/////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////
void handleRestart() 
  { 
    server.send(200, "text/html", strRestartESP);
    delay(1000);
    ESP.restart();
  }
/////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
void handleFormatSPIFFS() 
  {
    String response = strHttpResponseHead;
    SPIFFS.begin();
    if (SPIFFS.format()) { response += "SPIFFS Format Complete."; }
    else { response += "SPIFFS Format Failed."; }
    SPIFFS.end();
    response += "</body></html>";
    server.send(200, "text/html", response);
  }
////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handleEditSPIFFSfile()
  {
    String response = strEditSPIFFSfile;
    server.send(200, "text/html", response);
  }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////
void handleReadSPIFFSfile()
  {
    String x_file = "";
    if (server.args() > 0 ) 
      {
        if (server.arg("file") != "")
          { x_file = readFile(server.arg("file")); }
      }
    server.send(200, "application/json", x_file);
  }
/////////////////////////////////////////////////////

///////////////////////////////////////////////////////
void handleDeleteSPIFFSfile()
  {
    String x_file = "";
    if (server.args() > 0 ) 
      {
        if (server.arg("file") != "")
          { x_file = deleteFile(server.arg("file")); }
      }
    server.send(200, "application/json", x_file);
  }
///////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
void handleSaveSPIFFSfile()
  { // write this like handleDeleteSPIFFSfile
    //Serial.println(server.arg("plain"));
    if (server.arg("plain") != "") 
      {
        JSONVar x_file = JSON.parse(server.arg("plain"));
        String strPath = x_file["fileName"];
        if (!strPath.startsWith("/")) { strPath = "/" + strPath; }
        SPIFFS.begin();
        File x_f = SPIFFS.open(strPath, "w");
        x_f.print(String(x_file["content"]));
        x_f.close();
        SPIFFS.end();
        server.send(200, "application/json", server.arg("plain"));
      }
  }
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
void handleScannetworks() 
  { server.send(200, "application/json", ScanNetworks()); }
///////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handleNotFound() 
  {
    String strPath = server.uri();
    JSONVar objFile = JSON.parse(readFile(String(server.uri())));
    // ****** hack DIY option only serves .html from SPIFFS/filesystem - Revisit *******
    if (objFile["exists"] && ( strPath.endsWith(".css") || strPath.endsWith(".html") || strPath.endsWith(".js") ))
      {
        String strEncoding = "text/html";
        if (strPath.endsWith(".css")) strEncoding = "text/css";
        if (strPath.endsWith(".js")) strEncoding = "text/javascript";
        String strFileContent = String(objFile["content"]);
        server.send(200, strEncoding, strFileContent);
      }
    else
      {
        server.send(200, "text/html", strRedirectToRootHTML);
      }
  }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
String deleteFile(String strPath)
  {
    JSONVar x_file;
    if (!strPath.startsWith("/")) strPath = "/" + strPath;
    SPIFFS.begin();
    if (SPIFFS.exists(strPath)) { SPIFFS.remove(strPath); }
    x_file["status"] = SPIFFS.exists(strPath) ? false:true;
    SPIFFS.end();
    return JSON.stringify(x_file);
  }
////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
String getData() /*MODIFICATION*/
  {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    JSONVar x_data;

    x_data["accelleration"]["x"] = a.acceleration.x;
    x_data["accelleration"]["y"] = a.acceleration.y;
    x_data["accelleration"]["z"] = a.acceleration.z;
    x_data["rotation"]["x"] = g.gyro.x;
    x_data["rotation"]["y"] = g.gyro.y;
    x_data["rotation"]["z"] = g.gyro.z;
    x_data["temperature"] = temp.temperature;

    return JSON.stringify(x_data);
  }
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
String readFile(String strPath)
  {
    JSONVar x_file;
    String x_content = "";
    if (!strPath.startsWith("/")) strPath = "/" + strPath;
    SPIFFS.begin();
    x_file["path"] = strPath;
    if (SPIFFS.exists(strPath))
      {
        File x_f = SPIFFS.open(strPath, "r");
        x_file["fullName"] = String(x_f.fullName());
        x_file["size"] = String(x_f.size());
        while(x_f.available()) 
          { x_content += String(x_f.readStringUntil('\n')) + String("\n"); }
        x_file["exists"] = true;
        x_file["content"] = x_content;
        x_f.close();
      }
    else { x_file["exists"] = false; }
    SPIFFS.end();
    x_content = JSON.stringify(x_file);
    //Serial.println(x_content);
    x_file = null;
    return x_content;
  }
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////
String ReadEEPROM()
  {
    JSONVar x_return;
    String x_str = "";
    EEPROM.begin(512);
    delay(10);
    for (int x_i = 0; x_i < 512; x_i++)
      { x_str += String(char(EEPROM.read(x_i))); }
    EEPROM.end();
    x_return["EEPROM"] = x_str;
    return JSON.stringify(x_return);
  }
///////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
String WNconnect(String x_hostname, String x_ssid, String x_pwd)
  {
    JSONVar x_return;
    if (x_hostname != "") { WiFi.hostname(x_hostname); }
    WiFi.begin(x_ssid, x_pwd);
    x_return["ConnectionResult"] = WiFi.waitForConnectResult();
    x_return["network"] = WiFi.SSID();
    x_return["localIP"] = WiFi.localIP().toString();
    x_return["macAddress"] = WiFi.macAddress();
    x_return["hostname"] = WiFi.hostname();
    x_return["gateway"] = WiFi.gatewayIP().toString();
    x_return["dns"] = WiFi.dnsIP().toString();
    return JSON.stringify(x_return);
  }
////////////////////////////////////////////////////////////////////

  
////////////////////////////////////////////////////////////
String APconnect(String x_ssid, String x_pwd, bool x_hidden, int x_maxconnection)
  {
    JSONVar x_return;
    x_return["ssid"] = x_ssid;
    x_return["active"] = false;
    x_return["hidden"] = x_hidden;
    x_return["maxconnection"] = x_maxconnection;
    x_return["active"] = WiFi.softAP
      (
        (x_ssid != "") ? x_ssid : DefaultID(),
        (x_pwd != "") ? x_pwd : "",
        1,
        x_hidden,
        (x_maxconnection >= 1 && x_maxconnection <= 8) ? x_maxconnection : 4
      );
    x_return["ip"] = WiFi.softAPIP().toString();
    x_return["mac"] = WiFi.softAPmacAddress();
    return JSON.stringify(x_return);
  }
///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////
String ListSPIFFS()
  {
    int x_count = 0;
    JSONVar x_files;
    SPIFFS.begin();
    Dir x_dir = SPIFFS.openDir("/");
    while (x_dir.next()) 
      {
        JSONVar x_file;
        x_file["fileName"] = String(x_dir.fileName());
        x_file["fileSize"] = String(x_dir.fileSize());
        x_file["fileTime"] = String(x_dir.fileTime());
        x_file["fileCreationTime"] = String(x_dir.fileCreationTime());
        x_files["file"][x_count] = x_file;
        x_count++;
      }
    SPIFFS.end();
    return JSON.stringify(x_files);
  }
///////////////////////////////////////////////////////////////////////


/////////////////////////////////////
String DefaultID()
  {
    String x_mac = WiFi.macAddress();
    x_mac.replace(":", "");
    return "esp" + x_mac;
  }
/////////////////////////////////////


///////////////////////////////////////////////////
String ScanNetworks()
  {
    JSONVar x_networks;
    int x_n = WiFi.scanNetworks();
    x_networks["error"] = "";
    
    if (x_n == 0) { x_networks["error"] = "No Networks Found."; }
    else
    {
      for (int x_i = 0; x_i < x_n; ++x_i)
      {
        JSONVar x_network;
        String strEncType = "";
        switch (WiFi.encryptionType(x_i))
          {
            case ENC_TYPE_WEP:
              strEncType = "WEP";
              break;
            case ENC_TYPE_TKIP:
              strEncType = "WPA";
              break;
            case ENC_TYPE_CCMP:
              strEncType = "WPA2";
              break;
            case ENC_TYPE_NONE:
              strEncType = "None";
              break;
            case ENC_TYPE_AUTO:
              strEncType = "Auto";
              break;
          }
        x_network["ssid"] = String(WiFi.SSID(x_i));
        x_network["rssi"] = String(WiFi.RSSI(x_i));
        x_network["encription"] = strEncType;
        x_networks["network"][x_i] = x_network;
      }
    }
    return JSON.stringify(x_networks);
  }
///////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
  {
    switch (type)
      {
        case WStype_DISCONNECTED:
          Serial.printf("[%u] Disconnected!\n", num);
          break;

        case WStype_CONNECTED: 
          {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            // send message to client
            webSocket.sendTXT(num, "0");
          }
          break;

        case WStype_TEXT:
          Serial.printf("[%u] get Text: %s\n", num, payload);
          // send message to client
          // webSocket.sendTXT(num, "message here");
          // send data to all connected clients
          // webSocket.broadcastTXT("message here");
          break;
      
        case WStype_BIN:
          Serial.printf("[%u] get binary length: %u\n", num, length);
          hexdump(payload, length);
          // send message to client
          // webSocket.sendBIN(num, payload, length);
          break;
      }
  }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
