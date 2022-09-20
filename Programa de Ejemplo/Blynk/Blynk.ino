/*
 *  Programa de ejemplo para utilizar el módulo dimmer con cruce con cero, con Wemos y Fuente integradas. 
 *
 *  Más info:
 *  https://github.com/lu4ult/dimmer-modulo-desarrollo
 *
*/


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "MyAuthDeBlynk";
char ssid[] = "MiSSID";
char pass[] = "MiContraseña";

byte brillo = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("\nHola!");
  
  Blynk.begin(auth, ssid, pass);
  
  pinMode(14,OUTPUT);
  attachInterrupt(12, zeroCrosssInt, RISING);
}

void loop() {
  Blynk.run();
}


ICACHE_RAM_ATTR
void zeroCrosssInt()  {  // Esta interrupción se ejecuta en el cruce por cero, cada 10mS para 50Hz
  //Cálculo para 50Hz: 1/50=20mS
  //Entonces medio ciclo= 10mS = 10000uS
  //(10000uS - 10uS) / 128 = 75 (aproximadamente), 10uS propagación 

  if(brillo<10 or brillo>120) {
    if(brillo<10)
      digitalWrite(14, HIGH);
    if(brillo>120)
      digitalWrite(14, LOW);
  }
  else {
    delayMicroseconds(75*brillo);
    digitalWrite(14, HIGH);
    delayMicroseconds(10);
    digitalWrite(14, LOW);
  }
}

BLYNK_WRITE(V1) {
  brillo = param.asInt();                    // Recibimos el valor del deslizador en la App Blynk
}
