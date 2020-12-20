#include <Wire.h>
#include <WireData.h>
#include <DS18B20.h>
#include <EEPROM.h>
#include <math.h>
#include <movingAvg.h>

#define optoC1 2
#define optoC2 3
#define confort 10
#define eco 6
#define hors_gel 12
#define arret 8

char adresse  = 0x08;
int addr_EEPROM = 0;
float temperature, t, temp;
float* temperaturePtr = &temperature;
float* tempPtr = &temp;
int recu = 8;
int* ordre = &recu;
char longueur_moyenne_glissante = 10;
movingAvg mv_avg(longueur_moyenne_glissante);

void requestEvent();
void ordre_confort();
void ordre_eco();
void ordre_hors_gel();
void ordre_arret();
int mesure_DS18B20();
float mesure_CTN();
void attribut_ordre(int ordre);


void setup() {
  Serial.begin(115200);
  Wire.begin(adresse);
  Wire.onRequest(requestEvent);
  Wire.onReceive(i2cReceive);
  mv_avg.begin();
  pinMode(BROCHE_DS18B20, INPUT);
  pinMode(optoC1, OUTPUT);
  pinMode(optoC2, OUTPUT);
  recu = chaine_en_memoire();
  movingAvg mv_avg(10);
}

void loop() {
  mesure_DS18B20();
  temp = mesure_CTN();
  float* temperaturePtr = &temperature;
  float* tempPtr = &temp;
  Serial.println(*temperaturePtr);
  //Serial.print("temp loop : ");
  Serial.println(*tempPtr);
  *ordre = recu;
  attribut_ordre (*ordre);
}

void attribut_ordre (int ordre){
  if (ordre == confort){
    ordre_confort();
  }
  else if (ordre == eco){
    ordre_eco();
  }
  else if (ordre == hors_gel){
    ordre_hors_gel();
  }
  else if  (ordre == arret){
    ordre_arret();
  }
}

void ordre_confort(){
  digitalWrite(optoC1,0);
  digitalWrite(optoC2,0);
}

void ordre_eco(){
  digitalWrite(optoC1,1);
  digitalWrite(optoC2,1);
}

void ordre_hors_gel(){ //demi alternance négative
  digitalWrite(optoC1,1);
  digitalWrite(optoC2,0);
}

void ordre_arret(){ //demi alternance positive
  digitalWrite(optoC1,0);
  digitalWrite(optoC2,1);
}

int mesure_DS18B20()
{
  if (getTemperature(&temperature, true) != READ_OK) {
    //Serial.println(F("Erreur de lecture du capteur"));
    return temperature;
  }
}

void i2cReceive (int howMany) 
{
  int x = Wire.read();    // receive byte as an integer
  Serial.print("ordre : ");
  Serial.println(x);         // print the integer
  *ordre = x;
  EEPROM.write(addr_EEPROM, x);
}

void requestEvent(){
  float t[2] = {*temperaturePtr, *tempPtr};
  wireWriteData(t);
  //Serial.println(t[1]);
  //retourne les temperatures;
}

int chaine_en_memoire ()
{
  int recu = EEPROM.read(addr_EEPROM);
  //Serial.println(recu);
  return recu;
}

float mesure_CTN()
{
  const long Rr = 100000;
  int avg = mv_avg.reading(analogRead(1));
  float Ur =  avg/1024.*5;
  float Rctn = Rr*(5/Ur-1);
  //CTN Câble HP
  float temp = (log(Rctn) - 12.617)/-0.043718;
   // CTN Câble mesure
  //float temp = (log(Rctn) - 12.649)/-0.044616;
  Serial.println(temp);
  return temp;
}
