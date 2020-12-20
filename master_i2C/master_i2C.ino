/*les librairies sont à
C:\Users\tomas\OneDrive\Documents\Arduino\libraries\Cayenne-MQTT-ESP-master\src

*/

#include <CayenneMQTTESP8266.h>
#include <DetectDevice.h>
#include <Wire.h>
#include <WireData.h>

//#define CAYENNE_DEBUG
#define CAYENNE_PRINT Serial
#define slaveAddress 0x08   // N.B. I2C 7-bit addresses can only be in the range of 0x08 -> 0x77

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
int tab2 = 8;
int* ordre = &tab2;

#define confort 10
#define eco 6
#define hors_gel 12
#define arret 8
#define test 0.5

#define ACTUATOR_PIN D7 // Do not use digital pins 0 or 1 since those conflict with the use of Serial.

void setup() {
  
  Serial.begin(115200);
  Wire.begin();
  Cayenne.begin(username, password, clientID, ssid, wifiPassword);
  pinMode(ACTUATOR_PIN, OUTPUT);
  delay(1000);
}


void loop() {
  Cayenne.loop();
  int* value_Ptr = &value;
  *ordre = tab2;
}


CAYENNE_OUT_DEFAULT()
{
  float temp[2];
  delay(500);
  Wire.requestFrom(slaveAddress, sizeof temp);
  wireReadData(temp);
  Serial.println((float)temp[0]);
  Serial.println((float)temp[1]);
  Cayenne.celsiusWrite(1, temp[0]);
  Cayenne.celsiusWrite(4, temp[1]);
}

void LancerMesure()
{
  float data[2];
  Wire.beginTransmission(slaveAddress);
  wireWriteData(data);
  Wire.endTransmission();
}

void reset_boutons(int ordre)
{
  if (ordre == confort)
  {
    //Cayenne.virtualWrite(confort, 1);
    Cayenne.virtualWrite(eco, 0);
    Cayenne.virtualWrite(hors_gel, 0);
    Cayenne.virtualWrite(arret, 0);
  }
  else if (ordre == eco)
  {
    //Cayenne.virtualWrite(eco, 1);
    Cayenne.virtualWrite(confort, 0);
    Cayenne.virtualWrite(hors_gel, 0);
    Cayenne.virtualWrite(arret, 0);
  }
  else if (ordre == hors_gel)
  {
    //Cayenne.virtualWrite(hors_gel, 1);
    Cayenne.virtualWrite(eco, 0);
    Cayenne.virtualWrite(confort, 0);
    Cayenne.virtualWrite(arret, 0);
  }
  else if (ordre == arret)
  {
    //Cayenne.virtualWrite(arret, 1);
    Cayenne.virtualWrite(eco, 0);
    Cayenne.virtualWrite(hors_gel, 0);
    Cayenne.virtualWrite(confort, 0);
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
