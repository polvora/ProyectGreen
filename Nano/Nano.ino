#include <PID_v1.h>
#include <DHT.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"

// Definiciones de las llaves de direccion de memoria EEPROM
const char KEY_T_SET = 0,    KEY_T_P = 4,     KEY_T_I = 8,     KEY_T_D = 12;
const char KEY_H_SET = 16,   KEY_H_P = 20,    KEY_H_I = 24,    KEY_H_D = 28;
const char KEY_I_SET = 32,   KEY_I_P = 36,    KEY_I_I = 40,    KEY_I_D = 44;
const char KEY_M_SET = 48,   KEY_M_P = 52,    KEY_M_I = 56,    KEY_M_D = 60;
const char KEY_P_SET = 64,   KEY_P_P = 68,    KEY_P_I = 72,    KEY_P_D = 76;
const char KEY_PH_SET = 80,  KEY_PH_P = 84,   KEY_PH_I = 88,   KEY_PH_D = 92;
const char KEY_CO_SET = 96,  KEY_CO_P = 100,  KEY_CO_I = 104,  KEY_CO_D = 108;
const char KEY_EC_SET = 112, KEY_EC_P = 116,  KEY_EC_I = 120,  KEY_EC_D = 124;

// Definiciones de pines
#define PIN_O_HEATER 3      // PWM pin for resistive element heater (pidT Output)
#define PIN_O_HUMIDIFIER 2  // Digital pin for on/off humidifier (pidH Output)
#define PIN_I_DHT_SENSOR 11 // Digital pin for DHT sensor input (pidT & pidH Input)
#define PIN_I_THERMISTOR 10 // Analog pin input for a thermistor (pidT Input)  -> EXTRA <-
#define PIN_O_LIGHT 4       // Digital pin output to rele for lights controll (pidI Output)
#define PIN_I_PHOTORES 4    // Photoresistor to Analog pin for lights and ambient light feedback (pidI Input)
#define PIN_O_FAN 5         // PWM pin for fan speed controll (pidCO2 Output)

// Definiciones de los parametros y variables del controlador PID
double inputT,  setT,  outputT,  pT,  iT,  dT;
double inputH,  setH,  outputH,  pH,  iH,  dH;
double inputI,  setI,  outputI,  pI,  iI,  dI;
double inputM,  setM,  outputM,  pM,  iM,  dM;
double inputP,  setP,  outputP,  pP,  iP,  dP;
double inputPH, setPH, outputPH, pPH, iPH, dPH;
double inputCO, setCO, outputCO, pCO, iCO, dCO;
double inputEC, setEC, outputEC, pEC, iEC, dEC;

// Eespecidifcamos los enlances y  los parametros de tuneo iniciales
PID pidT(&inputT,   &outputT,  &setT,  pT,  iT,  dT,  DIRECT);
PID pidH(&inputH,   &outputH,  &setH,  pH,  iH,  dH,  DIRECT);
PID pidI(&inputI,   &outputI,  &setI,  pI,  iI,  dI,  DIRECT);
PID pidM(&inputM,   &outputM,  &setM,  pM,  iM,  dM,  DIRECT);
PID pidP(&inputP,   &outputP,  &setP,  pP,  iP,  dP,  DIRECT);
PID pidPH(&inputPH, &outputPH, &setPH, pPH, iPH, dPH, DIRECT);
PID pidCO(&inputCO, &outputCO, &setCO, pCO, iCO, dCO, DIRECT);
PID pidEC(&inputEC, &outputEC, &setEC, pEC, iEC, dEC, DIRECT);

// Configuracion del sensor ambiental de temperatura/humedad DHT
DHT dht(PIN_I_DHT_SENSOR, DHT21);

void setup()
{ 
  Serial.begin(115200);
  dht.begin();
  
  pinMode(PIN_O_HEATER,     OUTPUT);
  pinMode(PIN_O_HUMIDIFIER, OUTPUT);
  pinMode(PIN_I_DHT_SENSOR, INPUT);
  pinMode(PIN_O_LIGHT,      OUTPUT);
  pinMode(PIN_I_PHOTORES,   INPUT);
  pinMode(PIN_O_FAN,        OUTPUT);
  
  // Cargamos las configuraciones del PID desde la EEPROM
  loadSettings();
  
  // Encendemos el controlador
  pidT.SetMode(AUTOMATIC);
  pidH.SetMode(AUTOMATIC);
  pidI.SetMode(AUTOMATIC);
  pidM.SetMode(AUTOMATIC);
  pidP.SetMode(AUTOMATIC);
  pidPH.SetMode(AUTOMATIC);
  pidCO.SetMode(AUTOMATIC);
  pidEC.SetMode(AUTOMATIC);

  // Esperamos que inicie el puerto serie
  while (!Serial) { }
}

void loop(){
  //digitalWrite(LightPin, LightState);//Turn ON/OF lights

  inputT = dht.readTemperature();
  //inputT = convertTermistorData(analogRead(TermistorPin));
  inputH = dht.readHumidity();
  pidT.Compute();
  pidH.Compute();
  
  analogWrite(PIN_O_HEATER,outputT);
  analogWrite(PIN_O_HUMIDIFIER,outputH);
  
  processNextSerialRequest();
}

void processNextSerialRequest() {
  String mBuffer = Serial.readStringUntil(';');
  if(mBuffer.equals("GETVALUES")) sendValues();

  Serial.flush();
}

void sendValues() {
  String mBuffer = String(inputT, 2) + "/" + String(setT, 2) + "/" + String(outputT, 2) + "/" + String(pT, 2) + "/" + String(iT, 2) + "/" + String(dT, 2) + ";";
  Serial.print(mBuffer);
  mBuffer = String(inputH, 2)  + "/" + String(setH, 2)  + "/" + String(outputH, 2)  + "/" + String(pH, 2)  + "/" + String(iH, 2)  + "/" + String(dH, 2)  + ";";
  Serial.print(mBuffer);
  mBuffer = String(inputI, 2)  + "/" + String(setI, 2)  + "/" + String(outputI, 2)  + "/" + String(pI, 2)  + "/" + String(iI, 2)  + "/" + String(dI, 2)  + ";";
  Serial.print(mBuffer);
  mBuffer = String(inputM, 2)  + "/" + String(setM, 2)  + "/" + String(outputM, 2)  + "/" + String(pM, 2)  + "/" + String(iM, 2)  + "/" + String(dM, 2)  + ";";
  Serial.print(mBuffer);
  mBuffer = String(inputP, 2)  + "/" + String(setP, 2)  + "/" + String(outputP, 2)  + "/" + String(pP, 2)  + "/" + String(iP, 2)  + "/" + String(dP, 2)  + ";";
  Serial.print(mBuffer);
  mBuffer = String(inputPH, 2) + "/" + String(setPH, 2) + "/" + String(outputPH, 2) + "/" + String(pPH, 2) + "/" + String(iPH, 2) + "/" + String(dPH, 2) + ";";
  Serial.print(mBuffer);
  mBuffer = String(inputCO, 2) + "/" + String(setCO, 2) + "/" + String(outputCO, 2) + "/" + String(pCO, 2) + "/" + String(iCO, 2) + "/" + String(dCO, 2) + ";";
  Serial.print(mBuffer);
  mBuffer = String(inputEC, 2) + "/" + String(setEC, 2) + "/" + String(outputEC, 2) + "/" + String(pEC, 2) + "/" + String(iEC, 2) + "/" + String(dEC, 2) + ";";
  Serial.print(mBuffer);
  
  mBuffer = String();
}

void loadSettings() {
  // Si los primeros cuatro bytes estan vacios (0XFF) es porque la memoria no se ha escrito nunca
  if (EEPROM.read(0) == 0xFF && EEPROM.read(1) == 0xFF &&
        EEPROM.read(2) == 0xFF && EEPROM.read(3) == 0xFF) {
          
    // En ese caso seteamos todos los valores a default
    EEPROM_writeAnything(KEY_T_SET, 25.0);  EEPROM_writeAnything(KEY_T_P, 97.0);
    EEPROM_writeAnything(KEY_T_I, 1.47);    EEPROM_writeAnything(KEY_T_D, 1675.0);
    
    EEPROM_writeAnything(KEY_H_SET, 40.0);  EEPROM_writeAnything(KEY_H_P, 97.0);
    EEPROM_writeAnything(KEY_H_I, 1.47);    EEPROM_writeAnything(KEY_H_D, 1675.0);
    
    EEPROM_writeAnything(KEY_I_SET, 1075.0);EEPROM_writeAnything(KEY_I_P, 97.0);
    EEPROM_writeAnything(KEY_I_I, 1.47);    EEPROM_writeAnything(KEY_I_D, 1675.0);
    
    EEPROM_writeAnything(KEY_M_SET, 28.0);  EEPROM_writeAnything(KEY_M_P, 97.0);
    EEPROM_writeAnything(KEY_M_I, 1.47);    EEPROM_writeAnything(KEY_M_D, 1675.0);

    EEPROM_writeAnything(KEY_P_SET, 760.0); EEPROM_writeAnything(KEY_P_P, 97.0);
    EEPROM_writeAnything(KEY_P_I, 1.47);    EEPROM_writeAnything(KEY_P_D, 1675.0);

    EEPROM_writeAnything(KEY_PH_SET, 1.6);  EEPROM_writeAnything(KEY_PH_P, 97.0);
    EEPROM_writeAnything(KEY_PH_I, 1.47);   EEPROM_writeAnything(KEY_PH_D, 1675.0);

    EEPROM_writeAnything(KEY_CO_SET, 452.0);EEPROM_writeAnything(KEY_CO_P, 97.0);
    EEPROM_writeAnything(KEY_CO_I, 1.47);   EEPROM_writeAnything(KEY_CO_D, 1675.0);

    EEPROM_writeAnything(KEY_EC_SET, 1.2);  EEPROM_writeAnything(KEY_EC_P, 97.0);
    EEPROM_writeAnything(KEY_EC_I, 1.47);   EEPROM_writeAnything(KEY_EC_D, 1675.0);
  }

  // Leemos los valores de la EEPROM
  EEPROM_readAnything(KEY_T_SET, setT);   EEPROM_readAnything(KEY_T_P, pT);
  EEPROM_readAnything(KEY_T_I, iT);       EEPROM_readAnything(KEY_T_D, dT);

  EEPROM_readAnything(KEY_H_SET, setH);   EEPROM_readAnything(KEY_H_P, pH);
  EEPROM_readAnything(KEY_H_I, iH);       EEPROM_readAnything(KEY_H_D, dH);

  EEPROM_readAnything(KEY_I_SET, setI);   EEPROM_readAnything(KEY_I_P, pI);
  EEPROM_readAnything(KEY_I_I, iI);       EEPROM_readAnything(KEY_I_D, dI);

  EEPROM_readAnything(KEY_M_SET, setM);   EEPROM_readAnything(KEY_M_P, pM);
  EEPROM_readAnything(KEY_M_I, iM);       EEPROM_readAnything(KEY_M_D, dM);

  EEPROM_readAnything(KEY_P_SET, setP);   EEPROM_readAnything(KEY_P_P, pP);
  EEPROM_readAnything(KEY_P_I, iP);       EEPROM_readAnything(KEY_P_D, dP);

  EEPROM_readAnything(KEY_PH_SET, setPH); EEPROM_readAnything(KEY_PH_P, pPH);
  EEPROM_readAnything(KEY_PH_I, iPH);     EEPROM_readAnything(KEY_PH_D, dPH);

  EEPROM_readAnything(KEY_CO_SET, setCO); EEPROM_readAnything(KEY_CO_P, pCO);
  EEPROM_readAnything(KEY_CO_I, iCO);     EEPROM_readAnything(KEY_CO_D, dCO);

  EEPROM_readAnything(KEY_EC_SET, setEC); EEPROM_readAnything(KEY_EC_P, pEC);
  EEPROM_readAnything(KEY_EC_I, iEC);     EEPROM_readAnything(KEY_EC_D, dEC);
}

// Thermister raw data to actual temperature
double convertTermistorData(int RawADC) {
  double Temp;
  Temp = log(((10240000 / RawADC) - 10000));
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp )) * Temp );
  Temp = Temp - 273.15;            // Convert Kelvin to Celcius
  return Temp;
}













