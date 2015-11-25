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
char CONFIG_AP_SSID[32] = "INDOOR";
char CONFIG_AP_PSWD[64] = "12345678";
char CONFIG_WEB_HOST[32] = "indoor.com";
char CONFIG_STA_SSID[32] = "GPT2";
char CONFIG_STA_PSWD[64] = "otrotipo";

// Variables de estado
char STATUS_NETWORK = 0;
char STATUS_GREEN = 0;

// Valores sensores
float VALUE_TEMPERATURE = 22;
float VALUE_HUMIDITY = 45;
float VALUE_ILLUMINANCE = 67;
float VALUE_GROUND_MOISTURE = 31;
float VALUE_PRESSURE = 760;
float VALUE_PH = 6.2;
float VALUE_CO2 = 452;
float VALUE_EC = 1.2;

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

void webRequestGetData() {
    // Se usa el constructor String(valor, base) porque si el valor es 0 lo entiende como null terminator
    String json = "{";
    json += "\"page_refresh_time\":"      +   String(PAGE_REFRESH_TIME, 10);
    json += ", \"status_network\":"       +   String(STATUS_NETWORK, 10);
    json += ", \"status_green\":"         +   String(STATUS_GREEN, 10);
    json += ", \"value_temperature\":"    +   String(VALUE_TEMPERATURE, 1);
    json += ", \"value_humidity\":"       +   String(VALUE_HUMIDITY, 1);
    json += ", \"value_illuminance\":"    +   String(VALUE_ILLUMINANCE, 1);
    json += ", \"value_ground_moisture\":"+   String(VALUE_GROUND_MOISTURE, 1);
    json += ", \"value_pressure\":"       +   String(VALUE_PRESSURE, 0);
    json += ", \"value_ph\":"             +   String(VALUE_PH, 2);
    json += ", \"value_co2\":"            +   String(VALUE_CO2, 0);
    json += ", \"value_ec\":"             +   String(VALUE_EC, 2);
    json += "}";
    webServer.send(200, "application/json", json);
    json = String();
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
