/*
 *  Programa de ejemplo para utilizar el módulo dimmer con cruce con cero, con Wemos y Fuente integradas. 
 *
 *  Más info:
 *  https://github.com/lu4ult/dimmer-modulo-desarrollo
 *
 *  En este ejemplo utilizamos un pulsador o una llave para encender o apagar la lámpara.
 *  Para utilizar un pulsador o botón debe definir modo en 2; #DEFINE MODO 2
 *  Y cada vez que los presiones la lámpara alternará su estado, si estaba encendida se apaga y si estaba apagada se enciende.
 *
 *  Para utilizar una llave o tecla defina: #define MODO 1
 *  Cada vez que accione la llave la lámapara cambiará su estado.
 *
 *  Vea ambos ejemplos en este video:
 *  https://youtu.be/ls5ejvqLH1E
 *
*/

#define MODO 1        //Colocar en 1 para utilizar una llave
                      //Colocar en 2 para utilizar un botón


//Lo llamamos "entrada" y no "entrada" o "botón" ya que puede ser cualquiera de los dos: un entrada o una llave, según se define arriba con "MODO"
#define PIN_ENTRADA 2  //GPIO2 = D4        //Ver esquemático en Fritzing adjunto.

bool entradaEstado=1, entradaEstadoAnterior=1;
int rampa = 20;                 //Velocidad con la que se enciende o apaga
byte brillo = 0;

void setup() {
  Serial.begin(115200);

  pinMode(PIN_ENTRADA,INPUT_PULLUP);               //Pin al que conectamos el botón o llave
  pinMode(14,OUTPUT);                               //Pin de salid para controlar el Triac (no se debe modificar ya que está integrado en la placa)
  attachInterrupt(12, zeroCrosssInt, RISING);       //Pin de entrada para el detector de cruce por cero (tampoco se debe modificar)


  Serial.println("\nHola!");
}

void loop() {
  #if MODO == 1
  entradaEstado = !digitalRead(PIN_ENTRADA);
  #endif

  #if MODO == 2
  if(!digitalRead(PIN_ENTRADA))
    entradaEstado = !entradaEstado;
  #endif

  if(entradaEstado != entradaEstadoAnterior) {
    entradaEstadoAnterior = entradaEstado;

    if(entradaEstado) {
      for(int i=128;i>0;i--){
        brillo = i;
        Serial.println("Valor actual: "+String(brillo));
        delay(rampa);
        }
      }
    else {
      for(int i=0;i<128;i++){
        brillo = i;
        Serial.println("Valor actual: "+String(brillo));
        delay(rampa);
        }
      }
    }
  }

ICACHE_RAM_ATTR
void zeroCrosssInt()  {
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