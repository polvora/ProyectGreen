#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <FS.h>
#include <Ticker.h>

#define DEBUG // Comentado default

// Parametros Punto de Acceso
const char *apSSID = "INDOOR";
const char *apPassword = "12345678";
IPAddress apIP(192, 168, 4, 1);
IPAddress apMask(255, 255, 255, 0);

// Parametros Servidor Web
ESP8266WebServer webServer(80);

// Parametros Servidor DNS
DNSServer dnsServer;
const byte DNS_PORT = 53;

// Parametros Estacion Wifi + Cliente Web
const char* ssid     = "GPT2";
const char* password = "otrotipo";
Ticker updater;
const char* host = "posttestserver.com";

// Variables
boolean staConnected = false;

void setup() {
  delay(1000);

  // Comuncacion Serie para Debug
  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);
  
  // Configuracion del Punto de Acceso
  WiFi.softAPConfig(apIP, apIP, apMask);
  WiFi.softAP(apSSID, apPassword);

  // Configuracion Servidor DNS
  dnsServer.start(DNS_PORT, "*", apIP);
  dnsServer.setTTL(300);

  // Configuracion Sistema de Arcvhivos SPIFFS
  SPIFFS.begin();

  // Configuracion Servidor Web
  webServer.onNotFound([](){
    String path = webServer.uri();
    if(path.endsWith("/")) path += "index.htm";
    String contentType = getContentType(path);
    if(SPIFFS.exists(path)) {
      File file = SPIFFS.open(path, "r");
      webServer.streamFile(file, contentType);
      file.close();
    }
    else webServer.send(404, "text/html", "<strong>404</strong> File Not Found");
  });
  webServer.begin();

  // Configuracion Estacion WiFi
  WiFi.begin(ssid, password);
  updater.attach(2.0, [](){
    staConnected = (WiFi.status() == WL_CONNECTED);
  });
}

void loop() {
  dnsServer.processNextRequest();
	webServer.handleClient();
}

String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
