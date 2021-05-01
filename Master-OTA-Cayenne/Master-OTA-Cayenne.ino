/*les librairies sont à
  C:\Users\tomas\OneDrive\Documents\Arduino\libraries\Cayenne-MQTT-ESP-master\src
*/

#include <CayenneMQTTESP8266.h>
#include <DetectDevice.h>
#include <Wire.h>
#include <WireData.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <CertStoreBearSSL.h>
#include <ESP_OTA_GitHub.h>
#include <EEPROM.h>
#include <math.h>

//#define CAYENNE_DEBUG
//#define debug
//#define debug_eeprom

#define CAYENNE_PRINT Serial
#define ACTUATOR_PIN D7 // Do not use digital pins 0 or 1 since those conflict with the use of Serial.
#define slaveAddress 0x08   // N.B. I2C 7-bit addresses can only be in the range of 0x08 -> 0x77
#define confort 10
#define eco 6
#define hors_gel 12
#define arret 8
#define reset_demand 13
#define temp_consigne_channel 14
#define consigne_activation_channel 16
#define version_channel 20
#define ordre_channel 21
#define etat_consigne_channel 22
#define temp_consigne_aff_channel 23
#define RESET 0
#define SET 1
#define cu16consigne_defaut 20
#define cu16ordre_defaut  hors_gel
#define addr_Ordre 0x00
#define addr_Consigne 0x03
#define nbrValueEEPROM  2
#define cu8TailleMemoire  3*nbrValueEEPROM

/* Set up values for your repository and binary names */
#define GHOTA_USER "klut532"
#define GHOTA_REPO "OTA_update"
#define GHOTA_CURRENT_TAG "2.01"    //attention a ne pas mettre vxx.xx pour le tag mais xx.xx
#define GHOTA_BIN_FILE "Master-OTA-Cayenne.ino.d1_mini.bin"
#define GHOTA_ACCEPT_PRERELEASE 0


// WiFi network info.
#ifndef STASSID
#define STASSID "Frenola"
#define STAPSK  "frenola@"
#endif
/*
  char ssid[] = "Frenola";
  char wifiPassword[] = "frenola@";
  char ssid[] = "Livebox-D722";
  char wifiPassword[] = "escargot";
*/

struct sTemp{
  float I2C_Value;
  float CTN_Value;
};

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char* username = "6846eb50-9d55-11e9-be3b-372b0d2759ae";
char* password = "9a3c28484ddf8e815a2c5b8cdf5aaff5b0a98982";
// chauffage 1
//char* clientID = "0af9bca0-9e97-11e9-9636-f9904f7b864b";
//chauffage 2
char* clientID = "2b97d880-eda1-11e9-a38a-d57172a4b4d4";

BearSSL::CertStore certStore;
uint16_t u16ValueReceived;
uint8_t u8ValueReceived[2];
int temp_consigne;
bool consigne_enable;
uint8_t ordre_state = arret;
sTemp usTemperature;


void(* resetFunc) (void) = 0;
void lancer_chauffage(char ordre);
void arret_chauffage(char ordre);
void init_function (void);
void LancerMesure();
void reset_boutons(char ordre);
void envoi_ordre_arduino(uint8_t ordre);
void gestion_chauffe_consigne (void);
uint8_t u8Lecture_chaine_en_memoire (uint8_t u8Adresse);
void vEcritureE2prom (uint8_t adresse, uint8_t u8Value);
sTemp usDemandeMesure (sTemp usTemperature);


void setup() {

  Serial.begin(115200);
  EEPROM.begin(cu8TailleMemoire);
  pinMode(ACTUATOR_PIN, OUTPUT);
  Wire.begin();
  // Start SPIFFS and retrieve certificates.
  SPIFFS.begin();
  int numCerts = certStore.initCertStore(SPIFFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
  Serial.print(F("Number of CA certs read: "));
  Serial.println(numCerts);
  if (numCerts == 0) {
    Serial.println(F("No certs found. Did you run certs-from-mozill.py and upload the SPIFFS directory before running?"));
    return; // Can't connect to anything w/o certs!
  }

  // Connect to WiFi
  Serial.print("Connecting to WiFi... ");
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  if ((WiFi.status() != WL_CONNECTED)) {
    Serial.print("... ");
  }
  Serial.println();

  /* This is the actual code to check and upgrade */
  handle_upgade();
  Serial.print("Current Version : ");
  Serial.println(GHOTA_CURRENT_TAG);
  /* End of check and upgrade code */
  Cayenne.begin(username, password, clientID, STASSID, STAPSK);
  delay(1000);
  init_function();
}


void loop() {
  Cayenne.loop();
}

sTemp usDemandeMesure (sTemp usTemperature){
  uint8_t u8counter = 0;
  Wire.requestFrom(slaveAddress, (sizeof usTemperature.I2C_Value) + 2 );
  wireReadData(usTemperature.I2C_Value);
    while (Wire.available()) { // slave may send less than requested
    u8ValueReceived[u8counter] = Wire.read();
    u8counter++;
    }
  
  #ifdef debug_I2C
    Serial.print("Value received : ");
    Serial.print(temp[0]);
    Serial.print(" , ");
    Serial.print((byte) u8ValueReceived[0]);
    Serial.print(" , ");
    Serial.print((byte) u8ValueReceived[1]);
    Serial.print(" , ");
    Serial.println( (uint16_t)((u8ValueReceived[0]<<8) | (u8ValueReceived[1])) );
  #endif
  
  u16ValueReceived = (uint16_t)((u8ValueReceived[0]<<8) | (u8ValueReceived[1]));
  const long Rr = 100000;
  float Ur =  u16ValueReceived/1024.*5;
  float Rctn = Rr*(5/Ur-1);
  //float temp = (log(Rctn) - 12.617)/-0.043718;    //CTN Câble HP
  usTemperature.CTN_Value = (log(Rctn) - 12.649)/-0.044616;      // CTN Câble mesure
  
  #ifdef debug
    Serial.print("Temperature : ");
    Serial.print(usTemperature.I2C_Value);
    Serial.print(" , ");
    Serial.print(usTemperature.CTN_Value);
    Serial.print(" , ");
    Serial.print(temp_consigne);
    Serial.print(" , ");
    Serial.print(ordre_state);
    Serial.print(" , ");
    Serial.println(consigne_enable);
  #endif
  return usTemperature;
}

CAYENNE_OUT_DEFAULT()
{

  usTemperature = usDemandeMesure (usTemperature);
  
  Cayenne.celsiusWrite(1, usTemperature.I2C_Value);
  Cayenne.celsiusWrite(4, usTemperature.CTN_Value);
  Cayenne.virtualWrite(ordre_channel, ordre_state);
  Cayenne.virtualWrite(etat_consigne_channel, consigne_enable);
  Cayenne.virtualWrite(temp_consigne_aff_channel, temp_consigne);

  if (consigne_enable == SET) {
    gestion_chauffe_consigne();
  }
  envoi_ordre_arduino(ordre_state); //envoi ordre en cas d'erreur
}

void LancerMesure() {
  float data[2];
  Wire.beginTransmission(slaveAddress);
  wireWriteData(data);
  Wire.endTransmission();
}

void reset_boutons(void) {
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
    //Cayenne.virtualWrite(arret, 1);
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
  vEcritureE2prom (addr_Ordre, ordre_state);
  envoi_ordre_arduino(confort);
  reset_boutons();
}

CAYENNE_IN(eco)
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  Serial.println("ordre Eco");
  ordre_state = eco;
  vEcritureE2prom (addr_Ordre, ordre_state);
  envoi_ordre_arduino(eco);
  reset_boutons();
}

CAYENNE_IN(hors_gel)
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  Serial.println("ordre Hors Gel");
  ordre_state = hors_gel;
  vEcritureE2prom (addr_Ordre, ordre_state);
  envoi_ordre_arduino(hors_gel);
  reset_boutons();
}

CAYENNE_IN(arret)
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  Serial.println("ordre Arret");
  ordre_state = arret;
  vEcritureE2prom (addr_Ordre, ordre_state);
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
  vEcritureE2prom (addr_Consigne, temp_consigne);
  Serial.print("Température de consigne : ");
  Serial.println(temp_consigne);
}

CAYENNE_IN(consigne_activation_channel)
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asInt());
  consigne_enable = (int) getValue.asInt();
  
  if (consigne_enable == SET) {
    Serial.println("Consigne activée");
    reset_boutons();
    consigne_enable = SET;
    vEcritureE2prom (addr_Ordre, ordre_state);
    ordre_state = consigne_activation_channel;
  }
  else if (consigne_enable == RESET) {
    Serial.println("Consigne désactivée");
    ordre_state = hors_gel;
    reset_boutons();
  }
}

void lancer_chauffage(uint8_t ordre) {
  if (confort != ordre) {
    envoi_ordre_arduino(confort);
    ordre_state = confort;

    #ifdef debug
      Serial.println("Chauffage lancé");
    #endif
  }
}

void arret_chauffage(uint8_t ordre) {
  if (arret != ordre) {
    envoi_ordre_arduino(arret);
    ordre_state = arret;

    #ifdef debug
      Serial.println("Chauffage arrêté");
    #endif
  }
}

void init_function (void) {
  Serial.println("Lancement ...");
  ordre_state = arret;
  temp_consigne = 9;
  consigne_enable = RESET;
  Cayenne.virtualWrite(version_channel, GHOTA_CURRENT_TAG);
  reset_boutons();
  /* Lecture de l'eeprom */
  ordre_state = u8Lecture_chaine_en_memoire(addr_Ordre);
  temp_consigne = u8Lecture_chaine_en_memoire(addr_Consigne);
  if (ordre_state == consigne_activation_channel){
    consigne_enable = SET;
    usTemperature = usDemandeMesure (usTemperature);
    gestion_chauffe_consigne();
  }
  else{
    envoi_ordre_arduino(ordre_state);
  }
  Cayenne.virtualWrite(temp_consigne_aff_channel, temp_consigne);
  Serial.println("Init ... done");
}

uint8_t u8Lecture_chaine_en_memoire (uint8_t u8Adresse){
  uint8_t u8OctetLu[cu8TailleMemoire];
  uint8_t u8return;
  Serial.print("Lecture de l'eeprom : ");
  
  for (uint8_t i=0; i<3; i++){
    u8OctetLu[i] = EEPROM.read(u8Adresse + i);
    Serial.print(u8OctetLu[i]);
    Serial.print(" ");
  }
  Serial.println(" ");
  if ( (u8OctetLu[0]==u8OctetLu[1]) || (u8OctetLu[0]==u8OctetLu[2]) ){
    u8return =  u8OctetLu[0];      
  }
  else if ( u8OctetLu[1]==u8OctetLu[2] ){
    u8return =  u8OctetLu[1]; 
  }
  else{Serial.println("valeur EEPROM non prise en compte" );}
  if( (u8return == 255) ){//cas où la mémoire est non initialisée
    if(u8Adresse == addr_Ordre){
      u8return = cu16ordre_defaut;
      vEcritureE2prom (u8Adresse, cu16ordre_defaut);
    }
    else{
      u8return = cu16consigne_defaut;
      vEcritureE2prom (u8Adresse, cu16consigne_defaut);
    }
  }
  
  #ifdef debug_eeprom
    Serial.print("Lu : ");
    Serial.println(u8return);
  #endif
  return u8return;
}

void vEcritureE2prom (uint8_t u8Adresse, uint8_t u8Value){
  EEPROM.write(u8Adresse  , u8Value);
  EEPROM.write(u8Adresse+1, u8Value);
  EEPROM.write(u8Adresse+2, u8Value);
  EEPROM.commit();

  #ifdef debug
    Serial.println("Ecriture de la mémoire...");
  #endif
}

void envoi_ordre_arduino(uint8_t ordre) {
  Wire.beginTransmission(slaveAddress);
  Wire.write(ordre);
  Wire.endTransmission();
}

void gestion_chauffe_consigne (void) {
  if (temp_consigne < usTemperature.CTN_Value) {
    arret_chauffage(ordre_state);
  }
  else if (temp_consigne >= usTemperature.CTN_Value) {
    lancer_chauffage(ordre_state);
  }
  else{
    #ifdef debug  
      Serial.println("Erreur gestion chauffage");
    #endif
  }
}

void handle_upgade() {
  // Initialise Update Code
  //We do this locally so that the memory used is freed when the function exists.
  ESPOTAGitHub ESPOTAGitHub(&certStore, GHOTA_USER, GHOTA_REPO, GHOTA_CURRENT_TAG, GHOTA_BIN_FILE, GHOTA_ACCEPT_PRERELEASE);

  Serial.println("Checking for update...");
  if (ESPOTAGitHub.checkUpgrade()) {
    Serial.print("Upgrade found at: ");
    Serial.println(ESPOTAGitHub.getUpgradeURL());
    if (ESPOTAGitHub.doUpgrade()) {
      Serial.println("Upgrade complete."); //This should never be seen as the device should restart on successful upgrade.
    } else {
      Serial.print("Unable to upgrade: ");
      Serial.println(ESPOTAGitHub.getLastError());
    }
  } else {
    Serial.print("Not proceeding to upgrade: ");
    Serial.println(ESPOTAGitHub.getLastError());
  }
}
