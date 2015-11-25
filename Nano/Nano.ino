#include <ArduinoJson.h>
#include <PID_v1.h>
#include <DHT.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"

// Definiciones de las llaves de direccion de memoria EEPROM
const char KEY_T_SET = 0,  KEY_T_P = 4,   KEY_T_I = 8,   KEY_T_D = 12;
const char KEY_H_SET = 16, KEY_H_P = 20,  KEY_H_I = 24,  KEY_H_D = 28;
const char KEY_I_SET = 32, KEY_I_P = 36,  KEY_I_I = 40,  KEY_I_D = 44;
const char KEY_M_SET = 48, KEY_M_P = 52,  KEY_M_I = 56,  KEY_M_D = 60;

// Definiciones de pines
#define PIN_O_HEATER 3      // PWM pin for resistive element heater (pidT Output)
#define PIN_O_HUMIDIFIER 2  // Digital pin for on/off humidifier (pidH Output)
#define PIN_I_DHT_SENSOR 11 // Digital pin for DHT sensor input (pidT & pidH Input)
#define PIN_O_LIGHT 4       // Digital pin output to rele for lights controll (pidI Output)
#define PIN_I_PHOTORES 4    // Photoresistor to Analog pin for lights and ambient light feedback (pidI Input)
#define PIN_O_FAN 5         // PWM pin for fan speed controll (pidCO2 Output)

// Definiciones de los parametros y variables del controlador PID
double setT, inputT, outputT, pT, iT, dT;
double setH, inputH, outputH, pH, iH, dH;
double setI, inputI, outputI, pI, iI, dI;
double setM, inputM, outputM, pM, iM, dM;

// Eespecidifcamos los enlances y  los parametros de tuneo iniciales
PID pidT(&inputT, &outputT, &setT, pT, iT, dT, DIRECT);
PID pidH(&inputH, &outputH, &setH, pH, iH, dH, DIRECT);
PID pidI(&inputI, &outputI, &setI, pI, iI, dI, DIRECT);
PID pidM(&inputM, &outputM, &setM, pM, iM, dM, DIRECT);

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

  // Esperamos que inicie el puerto serie
  while (!Serial) { }
}

void loop(){
  //digitalWrite(LightPin, LightState);//Turn ON/OF lights

  inputT = dht.readTemperature();
  inputH = dht.readHumidity();
  pidT.Compute();
  pidH.Compute();
  
  analogWrite(PIN_O_HEATER,outputT);
  analogWrite(PIN_O_HUMIDIFIER,outputH);
  
  processNextSerialRequest();
}


/********************************************
 * Serial Communication functions 
 ********************************************/

// json string format: {"variablesPIDT":[InputT, OutputT, SetpointT],"parametrosPIDT":[pT,iT,dT,PIDT_manual,OutputT_reverse], "luz":[LightState]}

void processNextSerialRequest(){

  while(Serial.available()){
  
    String request = Serial.readString();
    int request_len = request.length();

    if (request.equals("GETDATA")) { sendData(); }
    
    // Prepare the character array (the buffer) 
    char json[request_len];
    
    // Copy it over 
    request.toCharArray(json, request_len);
    
    StaticJsonBuffer<200> jsonBuffer;
    
    JsonObject& root = jsonBuffer.parseObject(json);
    
    if(!root.success()){ //In case json root fails
      Serial.flush();
      //Serial.println("Falied json root");
      return;
    }
    
    setT = root["variablesPIDT"][2];
    
    // * read in and set the controller tunings
    pT = root["parametrosPIDT"][0];           //
    iT = root["parametrosPIDT"][1];            //
    dT = root["parametrosPIDT"][2];
    
    pidT.SetTunings(pT, iT, dT);
        
    bool PIDT_manual, OutputT_reverse;
    PIDT_manual = root["parametrosPIDT"][3];
    OutputT_reverse = root["parametrosPIDT"][4];
    
    if(PIDT_manual==1) { // set the controller mode
      outputT = root["variablesPIDT"][1];
      pidT.SetMode(MANUAL);  
    }
    else pidT.SetMode(AUTOMATIC);
    
    if(OutputT_reverse==1) { // * set the Output direction 
      pidT.SetControllerDirection(REVERSE);
    }
    else pidT.SetControllerDirection(DIRECT);
    
    //LightState = root["luz"][0];
    
    Serial.flush();                         // * clear any random data from the serial buffer
  }
}
void sendData(){

  StaticJsonBuffer<400> jsonBuffer;
  
  JsonObject& root = jsonBuffer.createObject();
  
  JsonObject& temperature = root.createNestedObject("temperature");
  temperature.set("input", inputT);
  temperature.set("output", outputT);
  temperature.set("setpoint", setT);
  temperature.set("kp", pT);
  temperature.set("ki", iT);
  temperature.set("kd", dT);
  temperature.set("mode", pidT.GetMode());
  temperature.set("direction", pidT.GetDirection());

  JsonObject& humidity = root.createNestedObject("humidity");
  humidity.set("input", inputH);
  humidity.set("output", outputH);
  humidity.set("setpoint", setH);
  humidity.set("kp", pH);
  humidity.set("ki", iH);
  humidity.set("kd", dH);
  humidity.set("mode", pidH.GetMode());
  humidity.set("direction", pidH.GetDirection());

  JsonObject& illuminance = root.createNestedObject("illuminance");
  illuminance.set("input", inputI);
  illuminance.set("output", outputI);
  illuminance.set("setpoint", setI);
  illuminance.set("kp", pI);
  illuminance.set("ki", iI);
  illuminance.set("kd", dI);
  illuminance.set("mode", pidI.GetMode());
  illuminance.set("direction", pidI.GetDirection());

  JsonObject& moisture = root.createNestedObject("moisture");
  moisture.set("input", inputM);
  moisture.set("output", outputM);
  moisture.set("setpoint", setM);
  moisture.set("kp", pM);
  moisture.set("ki", iM);
  moisture.set("kd", dM);
  moisture.set("mode", pidM.GetMode());
  moisture.set("direction", pidM.GetDirection());

  root.printTo(Serial);
  
  /*
  JsonArray& variablesPIDT = root.createNestedArray("variablesPIDT");
  variablesPIDT.add(InputT);
  variablesPIDT.add(OutputT);
  variablesPIDT.add(SetpointT);
  
  JsonArray& parametrosPIDT = root.createNestedArray("parametrosPIDT");
  parametrosPIDT.add(PIDT.GetKp());
  parametrosPIDT.add(PIDT.GetKi());
  parametrosPIDT.add(PIDT.GetKd());
  parametrosPIDT.add(PIDT.GetMode());
  parametrosPIDT.add(PIDT.GetDirection());

  JsonArray& luz = root.createNestedArray("luz");
   luz.add(LightState);//Boolean 
   //luz.add(AmbientLight);// Int %

  JsonArray& DHTsensor = root.createNestedArray("DHTsensor");
    if (isnan(Humidity) || isnan(AmbientTemperature)) {
     DHTsensor.add("Error");
     DHTsensor.add("Error");
     DHTsensor.add("Error");
    }
    else {
     DHTsensor.add(Humidity);
     DHTsensor.add(AmbientTemperature);
     DHTsensor.add(HeatIndex);
    }
    
  root.printTo(Serial);
  // This prints:
  // {"variablesPIDT":[InputT, OutputT, SetpointT],"parametrosPIDT":[pT,iT,dT,PIDT_manual,OutputT_reverse], "luz":[LightState,AmbientLight(Aun no definido)], "DHTsensor":[Humidity, AmbientTemperature, HEatIndex]}
  */
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
    EEPROM_writeAnything(KEY_M_I, 1.47);    int a =EEPROM_writeAnything(KEY_M_D, 1675.0);

    Serial.println(a);
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
}





