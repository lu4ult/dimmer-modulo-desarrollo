/*
 *  Programa de ejemplo para utilizar el módulo dimmer con cruce con cero, con Wemos y Fuente integradas. 
 *
 *  En este ejemplo simularemos la salida y puesta del sol, dimerizando una lámpara a la hora de la salida y puesta real del sol.
 *  Para ello el wemos se conecta a internet y obtiene la fecha y hora a través de NTP, luego calcula las horas de salida y puesta del sol (sunRise y sunSet).
 *  También definimos cuánto tiempo demorará de pasar de 0 a 100% y visceversa para el momento de sunrise y sunset respectivamente.
 *  Una vez alcanzada la hora de la salida del sol empieza a aumentar el brillo durante el periodo en minutos que hayamos definido, y visceversa al momento de la puesta del sol (sunset).
 *
 *  Opcional: display LCD por I2C.
 *  Más info:
 *  https://github.com/lu4ult/dimmer-modulo-desarrollo
 *
 *  Video en Youtube:
 *  https://youtu.be/x4mMS1b3m24
 *
*/

/*
 * Bibliotecas empleadas:
 * NTP Client:
 * https://github.com/arduino-libraries/NTPClient
 *
 * TimeLord:
 * https://github.com/probonopd/TimeLord
 *
 */


#define LCD                   //Descomente esta línea para utilizar el display LCD por I2C
//#define MODO_INVERTIDO      //Descomente esta línea para activar el modo invertido, es decir que a la salida del sol se apague y en la puesta del sol se encienda.
                              //Con este modo invertido se puede utilizar por ejemplo para luz tipo pasillo o LDR.

#include "credenciales.h"       //En este archivo configuramos el nombre y contraseña de nuestra red wifi, además de la latitud y longitud para calcular la salida y puesta del sol.
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLord.h>
#include <ArduinoOTA.h>
#ifdef LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#endif
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -3*60*60);       //-3 ya que en Argentina es UTC-3


/*Definimos cuánto tiempo (en minutos) tardará el dimmer en pasar de 0% al 100% al momento de la salida del sol,
* y de 100% al 0% al momento de la puesta del sol.
* Valores posibles: 1,2,4,5,10,20,25,50,100 (todos los que 100/tiempoRampo no tiene resto, otros valores se pueden utilizar con resultado inexacto)
*/
byte tiempoRampa=100;

/*Para facilitar las cuentas no utilizaremos la hora y minuto de la salida y puesta del sol, sino lo que llamaremos "momento",
 * tal que:
 * momento = hora*60 + minuto;
 *
 * Ejemplo:
 * La hora 8:25 equivale a 505 (8*60 + 25)
 * La hors 19:45 equivale a 1185 (19*60 + 45)
 *
 * Estos "momentos" los utilizaremos para la hora actual, la hora de la salida del sol (sunrise) y la hora de la puesta del sol (sunset)
 */
unsigned int momentoActual, momentoSunrise,momentoSunset;


/* Otras variables utilizadas */
int brilloDimmer,brilloDimmerAnterior=-1;
unsigned long previousMillis;
byte segundo, minuto,minutoAnterior, hora,
dia,diaAnterior,mes,ano;



void setup() {
  pinMode(14,OUTPUT);
  pinMode(12,INPUT);
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

  #ifdef LCD
  lcd.begin();
  lcd.backlight();
  lcd.print("Hola mundo!");
  #endif

  ArduinoOTA.setHostname("Dimmer Simulador Sunset/Sunrise");
  ArduinoOTA.begin();
  timeClient.begin();
  attachInterrupt(12, zeroCrosssInt, RISING);
  }

void loop() {
  timeClient.update();
  if(brilloDimmer == 0 or brilloDimmer == 100) {
    ArduinoOTA.handle();
  }

  if (millis() - previousMillis >= 1000) {
    previousMillis = millis();

    hora = timeClient.getHours();                                                 //Aquí simplemente obtenemos la hora y fecha desde el cliente NTP.
    minuto = timeClient.getMinutes();
    segundo = timeClient.getSeconds();
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime ((time_t *)&epochTime);
    dia = ptm->tm_mday;
    mes = ptm->tm_mon+1;
    ano = ptm->tm_year+1900-2000;

    momentoActual = hora*60 + minuto;
    #ifdef LCD
    lcd.setCursor(0,0);
    lcd.print(formatedTimeInMinutesAsString(hora*3600 + minuto*60 + segundo) + " - " + String(brilloDimmer) + "%");
    #endif

    if(minuto != minutoAnterior) {
      minutoAnterior = minuto;
      Serial.println("\nHora actual: "+formatedTimeInMinutesAsString(hora*3600 + minuto*60 + segundo));

      if(momentoActual >= momentoSunrise and momentoActual < momentoSunset) {     //Cuando alcanzamos el "momento" de la salida del sol (y aún es temprano para la puesta del sol),
        byte step = 100/tiempoRampa;
        //brilloDimmer += step;
        #ifdef MODO_INVERTIDO
        brilloDimmer -= step;                                                     //Empezamos a disminuir el brillo si está en modo invertido
        #elif
        brilloDimmer += step;                                                     //Empezamos a aumentar el brillo en modo normal
        #endif
      }

      if(momentoActual >= momentoSunset) {                                        //Cuando alcanzamos el momento de la puesta de sol, vamos disminuyendo el brillo hasta llegar al 0% y queda totalmente apagado. Si está en modo invertido funciona al revés: lo va aumentando.
        byte step = 100/tiempoRampa;
        //brilloDimmer -= step;
        #ifdef MODO_INVERTIDO
        brilloDimmer += step;
        #elif
        brilloDimmer -= step;
        #endif
      }

      if(brilloDimmer > 100)
        brilloDimmer = 100;
      if(brilloDimmer < 0)
        brilloDimmer = 0;

      #ifdef LCD
      lcd.clear();
      lcd.print(formatedTimeInMinutesAsString(hora*3600 + minuto*60 + segundo) + " - " + String(brilloDimmer) + "%");
      lcd.setCursor(0,1);
      lcd.print(String(momentoSunrise) + "-" + String(momentoActual) + "-" + String(momentoSunset));
      #endif
    }
  }

  //Cada vez que el valor del brillo se cambia, analizamos si debe estar apagado, encendido o dimerizando (cualquier punto medio).
  //En caso de que esté totalmente encendido o apagado no tiene sentido ejecutar la interrupción del cruce por cero, y simplemente escribimos un 0 o un 1 en la salida conectada al triac.
  if(brilloDimmer != brilloDimmerAnterior) {
    Serial.println("Brillo Dimmer: "+String(brilloDimmer));
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

  if(dia != diaAnterior) {                                              //Cada vez que se cambia de día analizamos el momento de Sunrise y Sunset.
    TimeLord tardis;
    tardis.TimeZone(-3 * 60);                                           //-3 ya que la hora en Argentina es UTC-3
    tardis.Position(LATITUD, LONGITUD);

    byte today[] = {segundo,minuto,hora,dia,mes,ano};

    if(tardis.SunRise(today))  {
     Serial.print("Sunrise: ");
     Serial.print((int) today[tl_hour]);
     Serial.print(":");
     Serial.println((int) today[tl_minute]);
     momentoSunrise = today[tl_hour]*60 + today[tl_minute];
     //momentoSunrise = 14*60 + 0;                                       //Para usar un valor falso de salida del sol y poder probar el funcionamiento del programa
     }

     if(tardis.SunSet(today))  {
     Serial.print("Sunset: ");
     Serial.print((int) today[tl_hour]);
     Serial.print(":");
     Serial.println((int) today[tl_minute]);
     momentoSunset = today[tl_hour]*60 + today[tl_minute];
     //momentoSunset = 18*60 + 15;                                        //Para usar un valor falso de puesta del sol y poder probar el funcionamiento del programa
     }

    Serial.println("Momento Sunrise: "+String(momentoSunrise));
    Serial.println("Momento Actual: "+String(momentoActual));
    Serial.println("Momento Sunset: "+String(momentoSunset));

    if(diaAnterior == 0) {                                              //Detectamos el primer inicio del módulo (no existe un día 0
      Serial.println("Primer inicio");
      minutoAnterior = 61;
      if(momentoActual > momentoSunrise and momentoActual < momentoSunset)
        #ifdef MODO_INVERTIDO
        brilloDimmer = 0;
        #elif
        brilloDimmer = 100;
        #endif
      else
        #ifdef MODO_INVERTIDO
        brilloDimmer = 100;
        #elif
        brilloDimmer = 0;
        #endif
    }
    diaAnterior = dia;
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


//Esta función recibe un valor en segundos y retorna un string en formato hh:mm:ss
//Ejemplo, si le pasamos el valor 72 devuelve 00:01:12 (ya que 72=1*60+12)
String formatedTimeInMinutesAsString(uint32_t _totalSeconds) {
  byte _hora = 0, _minuto=0 ,_segundo=0;
  String _horaS, _minutoS, _segundoS;

  if(_totalSeconds < 10)
    return "0:00:0"+String(_totalSeconds);

  if(_totalSeconds < 60)                      //Menos de un minuto
    return "0:00:"+String(_totalSeconds);

  if(_totalSeconds < 3600)  {                 //Mas de un minuto pero menos de una hora
    _minuto = (int) _totalSeconds/60;
    _totalSeconds -= _minuto*60;

    if(_minuto<10)
      _minutoS = "0";
    _minutoS += String(_minuto);

    if(_totalSeconds <10)
      _segundoS = "0";
    _segundoS += String(_totalSeconds);

    return "0:"+_minutoS+":"+_segundoS;
    }

  if(_totalSeconds >= 3600) {                 //Si es más de una hora
    _hora = (int) _totalSeconds/3600;
    _totalSeconds -=  _hora*3600;
    _minuto = (int) _totalSeconds/60;
    _totalSeconds -=  _minuto*60;
    _segundo = _totalSeconds;

    if(_minuto<10)
      _minutoS = "0";
    _minutoS += String(_minuto);

    if(_totalSeconds <10)
      _segundoS = "0";
    _segundoS += String(_totalSeconds);

    return String(_hora)+":"+_minutoS+":"+_segundoS;
    }
  }
