/*
 *  Programa de ejemplo para utilizar el módulo dimmer con cruce con cero, con Wemos y Fuente integradas.
 *
 *  Más info:
 *  https://github.com/lu4ult/dimmer-modulo-desarrollo
 *
 *  En este ejemplo se genera un web server al cuál se podrá acceder desde un navegador y modificar el brillo del dimmer
 *  (ver captura de pantalla adjunta)
 *
 *  El web server está basado en este tutorial: https://randomnerdtutorials.com/esp8266-web-server/
 *
*/

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


const char* ssid     = "miRedWiFi";
const char* password = "miContraseñaWiFi";

WiFiServer server(80);

byte brillo=0;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  pinMode(14,OUTPUT);                               //Pin de salid para controlar el Triac (no se debe modificar ya que está integrado en la placa)
  attachInterrupt(12, zeroCrosssInt, RISING);       //Pin de entrada para el detector de cruce por cero (tampoco se debe modificar)
}

void loop(){

  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    String currentLine = "";                // make a String to hold incoming data from the client
    uint32_t currentTime = millis();
    uint32_t previousTime = currentTime;
    String header;

    while (client.connected() && currentTime - previousTime <= 2000) { // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        //Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
         if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if(header.indexOf("?brillo=") >= 0) {
              String _subString = header.substring(header.indexOf("?brillo=") +8,header.indexOf("HTTP")-1);
              brillo = _subString.toInt();
              if(brillo > 100)
                brillo=100;
            }

            String css =  R"(
              <style>
              body, form {
                font-family: Helvetica;
                font-size: 1.5rem;
                background: #000;
                color: white;
                display: flex;
                justify-content: center;
                align-items: center;
                flex-direction: column;
              }

              form {
                border: 3px solid blue;
                width: 50%;
                padding: 2rem;
              }

              form input {
                margin: 1rem;
                width: 50%;
              }

              input[type="submit"] {
                background: green;
                border: none;
                border-radius: 10px;
                padding: 0.5rem;
                font-size: 1rem;
                color: white;
              }
              </style>
              )";


            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");

            client.println(css);
            client.println("</head>");
            client.println("<body><h1>Dimmer Web Server</h1>");

            client.print("<form>");
            client.print("<input type=\"range\" name=\"brillo\" min=\"0\" max=\"100\" value=\""+String(brillo)+"\"/>");
            client.print("<input type=\"submit\">");
            client.print("</form>");

            client.println("</body></html>");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}


ICACHE_RAM_ATTR
void zeroCrosssInt()  {
  byte _brillo = map(brillo,0,100,128,0);

  if(_brillo<10 or _brillo>120) {
    if(_brillo<10)
      digitalWrite(14, HIGH);
    if(_brillo>120)
      digitalWrite(14, LOW);
  }
  else {
    delayMicroseconds(75*_brillo);
    digitalWrite(14, HIGH);
    delayMicroseconds(10);
    digitalWrite(14, LOW);
  }
}
