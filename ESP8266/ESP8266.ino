#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <DNSServer.h>
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

  // Configuracion Servidor Web
  webServer.on("/", handleRoot);
  webServer.begin();

  // Configuracion Estacion WiFi
  WiFi.begin(ssid, password);
  updater.attach(2.0, [](){
    staConnected = (WiFi.status() == WL_CONNECTED);
  });
}

void loop() {
  delay(200);
  dnsServer.processNextRequest();
	webServer.handleClient();
}

void handleRoot() {
  char temp[400];
  snprintf(temp, sizeof temp, "<h1>You are Connected</h1><br><p>%s</p>", staConnected?"yes":"no");
  webServer.send(200, "text/html", temp);
}






