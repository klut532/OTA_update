#include <Wire.h>
#include <WireData.h>
#include <DS18B20.h>
#include <movingAvg.h>

#define optoC1 2
#define optoC2 3
#define confort 10
#define eco 6
#define hors_gel 12
#define arret 8
#define longueur_moyenne_glissante 10
#define adresse  0x08

//#define debug


uint16_t i16MesureCTN_value;
float temperature;
float* temperaturePtr = &temperature;
uint16_t* pi16MesureCTN_value = &i16MesureCTN_value;
movingAvg mv_avg(longueur_moyenne_glissante);

void requestEvent();
void ordre_confort();
void ordre_eco();
void ordre_hors_gel();
void ordre_arret();
int mesure_DS18B20();
uint16_t mesure_CTN();
void attribut_ordre(int u8Ordre);


void setup() {
  Serial.begin(115200);
  Wire.begin(adresse);
  Wire.onRequest(requestEvent);
  Wire.onReceive(i2cReceive);
  mv_avg.begin();
  pinMode(BROCHE_DS18B20, INPUT);
  pinMode(optoC1, OUTPUT);
  pinMode(optoC2, OUTPUT);
  movingAvg mv_avg(10);
}

void loop() {
  mesure_DS18B20();
  i16MesureCTN_value = mesure_CTN();
  float* temperaturePtr = &temperature;
  uint16_t* pi16MesureCTN_value = &i16MesureCTN_value;
}

void attribut_ordre (uint8_t u8Ordre){
  if (u8Ordre == confort){
    ordre_confort();
    #ifdef debug_ordre
      Serial.println("confort");
    #endif
  }
  else if (u8Ordre == eco){
    ordre_eco();
    #ifdef debug_ordre
      Serial.println("eco");
    #endif
  }
  else if (u8Ordre == hors_gel){
    ordre_hors_gel();
    #ifdef debug_ordre
      Serial.println("hors gel");
    #endif
  }
  else if  (u8Ordre == arret){
    ordre_arret();
    #ifdef debug_ordre
      Serial.println("arret");
    #endif
  }  
  else{//cas d'erreur 
    u8Ordre = hors_gel;
    ordre_hors_gel();
    #ifdef debug
      Serial.println("Cas d'erreur ordre");
    #endif
  }
}

void ordre_confort(void){
  digitalWrite(optoC1,0);
  digitalWrite(optoC2,0);
}

void ordre_eco(void){
  digitalWrite(optoC1,1);
  digitalWrite(optoC2,1);
}

void ordre_hors_gel(void){ //demi alternance négative
  digitalWrite(optoC1,1);
  digitalWrite(optoC2,0);
}

void ordre_arret(void){ //demi alternance positive
  digitalWrite(optoC1,0);
  digitalWrite(optoC2,1);
}

int mesure_DS18B20(void)
{
  if (getTemperature(&temperature, true) != READ_OK) {
    //Serial.println(F("Erreur de lecture du capteur"));
    return temperature;
  }
}

void i2cReceive (uint8_t u8Received, uint8_t * pu8Ordre) 
{
  u8Received = Wire.read();    // receive byte as an integer
  attribut_ordre(u8Received);
  
  #ifdef debug_I2C
    Serial.print("Ordre : ");
    Serial.println(u8Received);
  #endif
}

/* Retourne les temperatures */
void requestEvent(){
  float t = *temperaturePtr;
  uint16_t i16MesureCTN_value = *pi16MesureCTN_value;
  wireWriteData(t);  
  Wire.write( (byte) ((i16MesureCTN_value>>8)) );
  Wire.write( (byte) (i16MesureCTN_value) );
  
  #ifdef debug_temp
    Serial.print(t);
    Serial.print(" , ");
    Serial.println(i16MesureCTN_value);
  #endif
}

uint16_t mesure_CTN(void){
  uint16_t avg = mv_avg.reading(analogRead(1));
  
  #ifdef debug_CTN
    Serial.println(avg);
  #endif
  return avg;
}
