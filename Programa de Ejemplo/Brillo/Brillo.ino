/*
 *  Programa de ejemplo para utilizar el módulo dimmer con cruce con cero, con Wemos y Fuente integradas. 
 *
 *  Más info:
 *  https://github.com/lu4ult/dimmer-modulo-desarrollo
 *
 *  Video ejemplo:
 *  https://youtube.com/shorts/cVwYHIf1eo4
 *
*/

int espera = 20;
byte brilloDimmer = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("\nHola!");

  pinMode(14,OUTPUT);
  attachInterrupt(12, zeroCrosssInt, RISING);
}

void loop() {

  for(int i=0;i<128;i++){
    brilloDimmer = i;
    delay(espera);
  }
  delay(500);

  for(int i=128;i>0;i--){
    brilloDimmer = i;
    delay(espera);
  }
  delay(500);
}


ICACHE_RAM_ATTR
void zeroCrosssInt()  {  // Esta interrupción se ejecuta en el cruce por cero, cada 10mS para 50Hz
  //Cálculo para 50Hz: 1/50=20mS
  //Entonces medio ciclo= 10mS = 10000uS
  //(10000uS - 10uS) / 128 = 75 (aproximadamente), 10uS propagación 

  if(brilloDimmer<10 or brilloDimmer>120) {
    if(brilloDimmer<10)
      digitalWrite(14, HIGH);
    if(brilloDimmer>120)
      digitalWrite(14, LOW);
  }
  else {
    delayMicroseconds(75*brilloDimmer);
    digitalWrite(14, HIGH);
    delayMicroseconds(10);
    digitalWrite(14, LOW);
  }
}
