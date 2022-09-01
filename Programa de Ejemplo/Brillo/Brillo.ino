/*
 *  Programa de ejemplo para utilizar el módulo dimmer con cruce con cero, con Wemos y Fuente integradas. 
 *  Requiere instalar la biblioteca:
 *
 *  https://github.com/fabianoriccardi/dimmable-light
 *
 *  Más info:
 *  https://github.com/lu4ult/dimmer-modulo-desarrollo
 *
*/

#include <dimmable_light.h>
#define ZC_PIN 12 //D6   //No modificar estos pines.
#define AC_LOAD 14 //D5
DimmableLight light(AC_LOAD);

int espera = 50;

void setup() {
  Serial.begin(115200);

  DimmableLight::setSyncPin(ZC_PIN);
  DimmableLight::begin();

  Serial.println("\nHola!");
}

void loop() {
  if(Serial.available()) {
    delay(10);
    int valor = Serial.parseInt();
    light.setBrightness(valor);
  }

  for(int i=0;i<255;i++){
    light.setBrightness(i);
    delay(espera);
  }

  for(int i=255;i>0;i--){
    light.setBrightness(i);
    delay(espera);
  }
}
