/*
 *  Programa de ejemplo para utilizar el módulo dimmer con cruce con cero, con Wemos y Fuente integradas. 
 *  Más info:
 *  https://github.com/lu4ult/dimmer-modulo-desarrollo
 *
 *
 *  En este ejemplo utilizamos un potenciómetro para variar la intensidad de la lámpara.
 *  Video ejemplo:
 *  https://youtube.com/shorts/EDdIExiOSi4
 *
*/

byte adcValor,adcValorAnterior, dimming;

void setup() {
  Serial.begin(115200);
  Serial.println("\nHola!");

  pinMode(14,OUTPUT);
  attachInterrupt(12, zeroCrosssInt, RISING);
}

void loop() {
  adcValor = map(analogRead(A0),0,1024,128,0);
  if(adcValor != adcValorAnterior)  {              //Sólo actualizamos el valor del dimmer cuando realmente cambió la lectura del ADC
    adcValorAnterior = adcValor;

    Serial.println("ADC: "+String(adcValor));
    dimming = adcValor;
  }
}


ICACHE_RAM_ATTR
void zeroCrosssInt()  {  // Esta interrupción se ejecuta en el cruce por cero, cada 10mS para 50Hz
  //Cálculo para 50Hz: 1/50=20mS
  //Entonces medio ciclo= 10mS = 10000uS
  //(10000uS - 10uS) / 128 = 75 (aproximadamente), 10uS propagación 

  if(dimming<10 or dimming>120) {
    if(dimming<10)
      digitalWrite(14, HIGH);
    if(dimming>120)
      digitalWrite(14, LOW);
  }
  else {
    delayMicroseconds(75*dimming);
    digitalWrite(14, HIGH);
    delayMicroseconds(10);
    digitalWrite(14, LOW);
  }
}
