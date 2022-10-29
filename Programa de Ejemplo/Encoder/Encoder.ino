/*
 *  Programa de ejemplo para utilizar el módulo dimmer con cruce con cero, con Wemos y Fuente integradas. 
 *
 *  En este ejemplo utilizamos un encoder rotativo (ky-040) para variar el brillo de la lámpara.
 *  Video en Youtube:
 *  <pendiente>
 *
*/

/*
 * Bibliotecas empleadas:
 * ESPRotary:
 * https://github.com/LennartHennigs/ESPRotary
 *
 */


#include "credenciales.h"       //En este archivo configuramos el nombre y contraseña de nuestra red wifi, además de la latitud y longitud para calcular la salida y puesta del sol.
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

#include <ESPRotary.h>
ESPRotary encoder;
#define ROTARY_PIN1 0       //CLK
#define ROTARY_PIN2  13     //DT
#define ROTARY_SWITCH 2     //SW


bool encoderSwitch,encoderSwitchAnt;
int brilloDimmer,brilloDimmerAnterior=-1;

void setup() {
  pinMode(14,OUTPUT);
  pinMode(12,INPUT);
  pinMode(ROTARY_SWITCH,INPUT_PULLUP);
  digitalWrite(14,0);

  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n\nHola!");

  WiFi.begin(WIFI_NAME, WIFI_PASS);
  Serial.print("Conectando a: ");
  Serial.println(WIFI_NAME);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print( "." );
  }


  ArduinoOTA.setHostname("Dimmer Encoder");
  ArduinoOTA.begin();

  encoder.begin(ROTARY_PIN2, ROTARY_PIN1);
  //encoder.setChangedHandler(rotate);
  encoder.setLeftRotationHandler(showDirection);
  encoder.setRightRotationHandler(showDirection);

  attachInterrupt(12, zeroCrosssInt, RISING);
  }

void loop() {
  encoder.loop();

  encoderSwitch = !digitalRead(ROTARY_SWITCH);
  if(encoderSwitch != encoderSwitchAnt) {
    encoderSwitchAnt = encoderSwitch;
    if(encoderSwitch) {
      if(brilloDimmer)
        brilloDimmer = 0;
      else
        brilloDimmer = 100;
    }
  }

  if(brilloDimmer == 0 or brilloDimmer == 100) {
    ArduinoOTA.handle();
  }

  //Cada vez que el valor del brillo se cambia, analizamos si debe estar apagado, encendido o dimerizando (cualquier punto medio).
  //En caso de que esté totalmente encendido o apagado no tiene sentido ejecutar la interrupción del cruce por cero, y simplemente escribimos un 0 o un 1 en la salida conectada al triac.
  if(brilloDimmer != brilloDimmerAnterior) {
    if(brilloDimmer >100)
      brilloDimmer = 100;
    if(brilloDimmer < 0)
      brilloDimmer = 0;

    //Serial.println("Brillo Dimmer: "+String(brilloDimmer));
    if(brilloDimmer == 0 or brilloDimmer == 100) {
      detachInterrupt(12);
      if(brilloDimmer == 0)
        digitalWrite(14, LOW);
      if(brilloDimmer == 100)
        digitalWrite(14, HIGH);
    }
    else {
      attachInterrupt(12, zeroCrosssInt, RISING);
    }
    brilloDimmerAnterior = brilloDimmer;
  }
}


ICACHE_RAM_ATTR
void zeroCrosssInt()  {
  byte _dimming = map(brilloDimmer,0,100,128,0);

  if(_dimming<10 or _dimming>120) {
    if(_dimming<10)
      digitalWrite(14, HIGH);
    if(_dimming>120)
      digitalWrite(14, LOW);
  }
  else {
    delayMicroseconds(75*_dimming);
    digitalWrite(14, HIGH);
    delayMicroseconds(10);
    digitalWrite(14, LOW);
  }
}


// on left or right rotattion
void showDirection(ESPRotary& encoder) {
  //Serial.println(encoder.directionToString(encoder.getDirection()));
  //Serial.println(encoder.getDirection());

  if(encoder.directionToString(encoder.getDirection()) == "RIGHT")
    brilloDimmer++;
  else
    brilloDimmer--;
}
