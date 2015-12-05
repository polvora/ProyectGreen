#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <FS.h>
#include <Ticker.h>

//#define DEBUG // Comentado default

// CONFIGURACIONES PRE-PROGRAMABLES
#define SERIAL_SPEED      115200
#define AP_IP             192, 168, 4, 1 // Usar comas en vez de puntos
#define AP_MASK           255, 255, 255, 0 // Usar comas en vez de puntos
#define WEB_PORT          80
#define DNS_PORT          53
#define DNS_TTL           300 // Time To Load
#define PAGE_REFRESH_TIME 10 // Segundos
#define GREEN_HOST        "green.com"

// CONFIGURACIONES PROGRAMABLES
char CONFIG_AP_SSID[32] = "PID";
char CONFIG_AP_PSWD[64] = "12345678";
char CONFIG_WEB_HOST[32] = "pid.com";
char CONFIG_STA_SSID[32] = "GPT2";
char CONFIG_STA_PSWD[64] = "otrotipo";

// Variables de estado
char STATUS_NETWORK = 0;
char STATUS_GREEN = 0;

// Globales
ESP8266WebServer webServer(WEB_PORT);
DNSServer dnsServer;
Ticker updater;

void setup() {
  delay(200);

  // Comuncacion Serie
  Serial.begin(SERIAL_SPEED);

  // Activa del modo Access Point y Station al mismo tiempo
  WiFi.mode(WIFI_AP_STA);
  
  // Configuracion del Punto de Acceso
  WiFi.softAPConfig(IPAddress(AP_IP), IPAddress(AP_IP), IPAddress(AP_MASK));
  WiFi.softAP(CONFIG_AP_SSID, CONFIG_AP_PSWD);

  // Configuracion Servidor DNS
  dnsServer.start(DNS_PORT, CONFIG_WEB_HOST, IPAddress(AP_IP));
  dnsServer.setTTL(DNS_TTL);

  // Configuracion Sistema de Arcvhivos SPIFFS
  SPIFFS.begin();

  // Configuracion Servidor Web
  webServer.on("/data", webRequestGetData);
  webServer.onNotFound(webRequestAny);
  webServer.begin();

  // Configuracion Estacion WiFi
  WiFi.begin(CONFIG_STA_SSID, CONFIG_STA_PSWD);
  updater.attach(2.0, [](){ 
    // Actualizamos cada 2 segundos el estado de conexion del WiFi
    STATUS_NETWORK = (WiFi.status() == WL_CONNECTED)?1:0;
  });
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
  if (Serial.available()) handleIncommingData();
}

void handleIncommingData() {
  
  Serial.flush();
}

void webRequestGetData() {
  Serial.print("GETVALUES;");
  // Se usa el constructor String(valor, base) porque si el valor es 0 lo entiende como null terminator
  String json = "{";
  json += "\"page_refresh_time\":"      +   String(PAGE_REFRESH_TIME, 10);
  json += ", \"status_network\":"       +   String(STATUS_NETWORK, 10);
  json += ", \"status_green\":"         +   String(STATUS_GREEN, 10);
  json += ", \"value_temperature\":"    +   Serial.readStringUntil('/');
  Serial.readStringUntil(';'); // All other TEMPERATURE values
  json += ", \"value_humidity\":"       +   Serial.readStringUntil('/');
  Serial.readStringUntil(';'); // All other HUMIDITY values
  json += ", \"value_illuminance\":"    +   Serial.readStringUntil('/');
  Serial.readStringUntil(';'); // All other ILLUMINANCE values
  json += ", \"value_ground_moisture\":"+   Serial.readStringUntil('/');
  Serial.readStringUntil(';'); // All other GROUND_MOISTURE values
  json += ", \"value_pressure\":"       +   Serial.readStringUntil('/');
  Serial.readStringUntil(';'); // All other PRESSURE values
  json += ", \"value_ph\":"             +   Serial.readStringUntil('/');
  Serial.readStringUntil(';'); // All other PH values
  json += ", \"value_co2\":"            +   Serial.readStringUntil('/');
  Serial.readStringUntil(';'); // All other CO2 values
  json += ", \"value_ec\":"             +   Serial.readStringUntil('/');
  Serial.readStringUntil(';'); // All other CO2 values
  json += "}";
  Serial.flush();
  
  webServer.send(200, "application/json", json);
  json = String();
}

/* De esta manera podremos acceder a cualquier archivo
 * del directorio sin tener que hacer un metodo para cada uno */
void webRequestAny() {
  String path = webServer.uri();
  if(path.endsWith("/")) path += "index.htm"; // Si se pide el root se envia index
  String contentType = getContentType(path);
  if(SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    webServer.streamFile(file, contentType);
    file.close();
  }
  else webServer.send(404, "text/html", "<strong>404</strong><br>File Not Found");
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
