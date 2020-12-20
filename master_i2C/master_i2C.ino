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
#define consigne_activation_channel 15
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


int ordre = arret;
float temp[2];
int temp_consigne;
bool consigne_enable;
int ordre_state;


void(* resetFunc) (void) = 0;
void lancer_chauffage(int ordre);
void arret_chauffage(int ordre);
void init_function (void);
void LancerMesure();
void reset_boutons(int ordre);
void envoi_ordre_arduino (void);
void gestion_chauffe_consigne (void);


void setup() {
  
  Serial.begin(115200);
  Wire.begin();
  Cayenne.begin(username, password, clientID, ssid, wifiPassword);
  pinMode(ACTUATOR_PIN, OUTPUT);
  delay(1000);
  init_function();
}


void loop() {
  Cayenne.loop();
  
  if (consigne_enable == SET){
    gestion_chauffe_consigne();
  }
}


CAYENNE_OUT_DEFAULT()
{
  //delay(500);
  Wire.requestFrom(slaveAddress, sizeof temp);
  wireReadData(temp);
  Cayenne.celsiusWrite(1, temp[0]);
  Cayenne.celsiusWrite(4, temp[1]);
}

void LancerMesure(){
  float data[2];
  Wire.beginTransmission(slaveAddress);
  wireWriteData(data);
  Wire.endTransmission(slaveAddress);
}

void reset_boutons(void){
  if (ordre_state == confort)
  {
    //Cayenne.virtualWrite(confort, 1);
    Cayenne.virtualWrite(eco, RESET);
    Cayenne.virtualWrite(hors_gel, RESET);
    Cayenne.virtualWrite(arret, RESET);
    Cayenne.virtualWrite(consigne_activation_channel, RESET);
    consigne_enable = RESET;
  }
  else if (ordre_state == eco)
  {
    //Cayenne.virtualWrite(eco, 1);
    Cayenne.virtualWrite(confort, RESET);
    Cayenne.virtualWrite(hors_gel, RESET);
    Cayenne.virtualWrite(arret, RESET);
    Cayenne.virtualWrite(consigne_activation_channel, RESET);
    consigne_enable = RESET;
  }
  else if (ordre_state == hors_gel)
  {
    //Cayenne.virtualWrite(hors_gel, 1);
    Cayenne.virtualWrite(eco, RESET);
    Cayenne.virtualWrite(confort, RESET);
    Cayenne.virtualWrite(arret, RESET);
    Cayenne.virtualWrite(consigne_activation_channel, RESET);
    consigne_enable = RESET;
  }
  else if (ordre_state == arret)
  {
    Cayenne.virtualWrite(arret, 1);
    Cayenne.virtualWrite(eco, RESET);
    Cayenne.virtualWrite(hors_gel, RESET);
    Cayenne.virtualWrite(confort, RESET);
    Cayenne.virtualWrite(consigne_activation_channel, RESET);
    consigne_enable = RESET;
  }
  else if (ordre_state == consigne_activation_channel)
  {
    Cayenne.virtualWrite(eco, RESET);
    Cayenne.virtualWrite(hors_gel, RESET);
    Cayenne.virtualWrite(confort, RESET);
    Cayenne.virtualWrite(arret, RESET);
  }
}

CAYENNE_IN(confort)
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  Serial.println("ordre Confort");
  ordre_state = confort;
  envoi_ordre_arduino(confort);
  reset_boutons();
}

CAYENNE_IN(eco)
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  Serial.println("ordre Eco");
  ordre_state = eco;
  envoi_ordre_arduino(eco);
  reset_boutons();
}

CAYENNE_IN(hors_gel)
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  Serial.println("ordre Hors Gel");
  ordre_state = hors_gel;
  envoi_ordre_arduino(hors_gel);
  reset_boutons();
}

CAYENNE_IN(arret)
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  Serial.println("ordre Arret");
  ordre_state = arret;
  envoi_ordre_arduino(arret);
  reset_boutons();
}

CAYENNE_IN(reset_demand)
{
  Serial.println("Reset en cours ...");
  resetFunc();
}

CAYENNE_IN(temp_consigne_channel)
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  temp_consigne = (int) getValue.asInt();
  ordre_state = temp_consigne_channel;
  Serial.print("Température de consigne : ");
  Serial.println(temp_consigne);
  reset_boutons();
}

CAYENNE_IN(consigne_activation_channel)
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asInt());
  consigne_enable = (int) getValue.asInt();
  if (consigne_enable == SET){
    Serial.println("Consigne activée");
  }
  else if (consigne_enable == RESET){
    Serial.println("Consigne désactivée");
  }
}

void lancer_chauffage(int ordre){
  if (confort != ordre){
    Serial.println("Chauffage lancé");
    envoi_ordre_arduino(confort);
    ordre_state = confort;
  }
}

void arret_chauffage(int ordre){
  if (arret != ordre){
    Serial.println("Chauffage arrêté");
    envoi_ordre_arduino(arret);
    ordre_state = arret;
  }
}

void init_function (void){
  Serial.println("Lancement ...");
  ordre_state = arret;
  temp_consigne = 9;
  consigne_enable = 0;
  Cayenne.virtualWrite(temp_consigne_channel, temp_consigne);
  reset_boutons();
  Serial.println("Init ... done");
}

void envoi_ordre_arduino(int ordre){
  Wire.beginTransmission(slaveAddress);
  Wire.write(ordre);
  Wire.endTransmission(slaveAddress);
}

void gestion_chauffe_consigne (void){
    if (temp_consigne <= t_mesuree){
      arret_chauffage(ordre_state);
    }
    else if (temp_consigne >= t_mesuree){
      lancer_chauffage(ordre_state);
    }
}
