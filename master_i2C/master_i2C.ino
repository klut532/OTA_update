/*les librairies sont à
C:\Users\tomas\OneDrive\Documents\Arduino\libraries\Cayenne-MQTT-ESP-master\src
*/

#include <CayenneMQTTESP8266.h>
#include <DetectDevice.h>
#include <Wire.h>
#include <WireData.h>

//#define CAYENNE_DEBUG
#define CAYENNE_PRINT Serial
#define ACTUATOR_PIN D7 // Do not use digital pins 0 or 1 since those conflict with the use of Serial.
#define slaveAddress 0x08   // N.B. I2C 7-bit addresses can only be in the range of 0x08 -> 0x77
#define confort 10
#define eco 6
#define hors_gel 12
#define arret 8
#define reset_demand 13
#define temp_consigne_channel 14
#define RESET 0
#define SET 1
#define t_mesuree temp[1]

// WiFi network info.

char ssid[] = "Redmi";
char wifiPassword[] = "oignon10";
/*
char ssid[] = "Frenola";
char wifiPassword[] = "frenola@";
char ssid[] = "Livebox-D722";
char wifiPassword[] = "escargot";
*/
// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char* username = "6846eb50-9d55-11e9-be3b-372b0d2759ae";
char* password = "9a3c28484ddf8e815a2c5b8cdf5aaff5b0a98982";
// chauffage 1
//char* clientID = "0af9bca0-9e97-11e9-9636-f9904f7b864b";
//chauffage 2
char* clientID = "2b97d880-eda1-11e9-a38a-d57172a4b4d4";

unsigned long lastMillis = 0;
int i;
int value;
int* value_Ptr = &value;
int tab;
int tab2 = arret;
int* ordre = &tab2;
float temp[2];
int temp_consigne;


void(* resetFunc) (void) = 0;
void lancer_chauffage(void);
void arret_chauffage(void);
void init_variables (void);
void LancerMesure();
void reset_boutons(int ordre);


void setup() {
  
  Serial.begin(115200);
  Wire.begin();
  Cayenne.begin(username, password, clientID, ssid, wifiPassword);
  pinMode(ACTUATOR_PIN, OUTPUT);
  delay(1000);
  init_variables();
}


void loop() {
  Cayenne.loop();
  int* value_Ptr = &value;
  *ordre = tab2;
}


CAYENNE_OUT_DEFAULT()
{
  //delay(500);
  Wire.requestFrom(slaveAddress, sizeof temp);
  wireReadData(temp);
  Serial.println(temp[0]);
  Serial.println(temp[1]);
  Serial.println(temp_consigne);
  Cayenne.celsiusWrite(1, temp[0]);
  Cayenne.celsiusWrite(4, temp[1]);
}

void LancerMesure(){
  float data[2];
  Wire.beginTransmission(slaveAddress);
  wireWriteData(data);
  Wire.endTransmission();
}

void reset_boutons(int ordre){
  if (ordre == confort)
  {
    //Cayenne.virtualWrite(confort, 1);
    Cayenne.virtualWrite(eco, RESET);
    Cayenne.virtualWrite(hors_gel, RESET);
    Cayenne.virtualWrite(arret, RESET);
  }
  else if (ordre == eco)
  {
    //Cayenne.virtualWrite(eco, 1);
    Cayenne.virtualWrite(confort, RESET);
    Cayenne.virtualWrite(hors_gel, RESET);
    Cayenne.virtualWrite(arret, RESET);
  }
  else if (ordre == hors_gel)
  {
    //Cayenne.virtualWrite(hors_gel, 1);
    Cayenne.virtualWrite(eco, RESET);
    Cayenne.virtualWrite(confort, RESET);
    Cayenne.virtualWrite(arret, RESET);
  }
  else if (ordre == arret)
  {
    //Cayenne.virtualWrite(arret, 1);
    Cayenne.virtualWrite(eco, RESET);
    Cayenne.virtualWrite(hors_gel, RESET);
    Cayenne.virtualWrite(confort, RESET);
  }
}

CAYENNE_IN(confort)
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  int tab = confort;
  Serial.print("ordre Confort : ");
  Serial.println(tab);
  *ordre = tab;
  Wire.beginTransmission(slaveAddress);
  Wire.write(tab);
  Wire.endTransmission(slaveAddress);
  reset_boutons(tab);
}

CAYENNE_IN(eco)
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  int tab = eco;
  Serial.print("Eco : ");
  Serial.println(tab);
  *ordre = tab;
  Wire.beginTransmission(slaveAddress);
  Wire.write(tab);
  Wire.endTransmission(slaveAddress);
  reset_boutons(tab);
}

CAYENNE_IN(hors_gel)
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  int tab = hors_gel;
  Serial.print("Hors Gel : ");
  Serial.println(tab);
  *ordre = tab;
  Wire.beginTransmission(slaveAddress);
  Wire.write(tab);
  Wire.endTransmission(slaveAddress);
  reset_boutons(tab);
}

CAYENNE_IN(arret)
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  int tab = arret;
  Serial.print("Arret : ");
  Serial.println(tab);
  *ordre = tab;
  Wire.beginTransmission(slaveAddress);
  Wire.write(tab);
  Wire.endTransmission(slaveAddress);
  reset_boutons(tab);
}

CAYENNE_IN(reset_demand)
{
  Serial.print("Reset en cours ...");
  resetFunc();
}

CAYENNE_IN(temp_consigne_channel)
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  temp_consigne = (int) getValue.asInt();
  Serial.print("Température de consigne : ");
  Serial.println(temp_consigne);
  Serial.print("Température de Mesurée : ");
  Serial.println(t_mesuree);
  if (temp_consigne <= t_mesuree){
    arret_chauffage();
  }
  else if (t_mesuree <= temp_consigne){
    lancer_chauffage();
  }
  
}

void lancer_chauffage(void){
  Serial.println("Chauffage lancé");
}

void arret_chauffage(void){
  Serial.println("Chauffage arrêté");
}

void init_variables (void){
 CAYENNE_OUT(temp_consigne_channel);
}
