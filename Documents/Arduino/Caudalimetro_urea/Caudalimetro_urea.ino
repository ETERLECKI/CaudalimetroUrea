#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <LiquidCrystal.h>
#include <stdlib.h>

const int RS = D2, EN = D3, d4 = D5, d5 = D6, d6 = D7, d7 = D8;   //Descripción de pines LCD
LiquidCrystal lcd(RS, EN, d4, d5, d6, d7); //Parametrizo LCD

String msg; //Valor leído del llavero
char msg2[5]; //Array del valor leído para convertirlo en entero
long int valor_decimal; //Valor decimal de los datos leídos
const byte CAUDALIMETRO = D4; //Conexión de caudalímetro, con resistencia de 4K7 a 5v, el otro cable de caudalímetro a masa
//const byte LED_HABI = D1; // Habilitación de bomba (Relé + led)
const byte PALANCA = D1; // Conectado con una resistencia a +5V y con la palanca de habilitación (NC) a masa
volatile int estado = HIGH; //estado inicial
volatile float pulsos = 0;
volatile float litros = 0;
int habilita = 0;
char c; //Captura los caracteres individuales tomados del RFID

const char* ssid = "NBcargp";
const char* password = "(*2468*)";
ESP8266WebServer server(80); //Llamo server al servidor


//void ICACHE_RAM_ATTR parpadeo();
//void ICACHE_RAM_ATTR reseteo();

void parpadeo() {

  if (habilita == 1) { // Si todoas las comprobaciones son correctas
    estado = !estado;
    pulsos = pulsos + 1;
    lcd.setCursor(9, 0);
    lcd.print(pulsos);
    litros = pulsos / 88;
    lcd.setCursor(9, 1);
    lcd.print(litros);
    //Serial.print("Litros: ");
    //Serial.println(litros);
  }

}

void reseteo() {
  //Serial.println("Entro a reseteo");
  if (digitalRead(PALANCA) == HIGH) {
    //Serial.println("Reset LCD");
    lcd.setCursor(9, 1);
    lcd.print(litros);
    habilita = 1;
    lcd.setCursor(0, 0);
    lcd.print("Pulsos:         ");
    //Serial.println("Enciendo Led");
    //digitalWrite(LED_HABI,HIGH);
  } else {
    pulsos = 0;
    habilita = 0;
    delay(2000);
    //Serial.println("Apago Led");
    //digitalWrite(LED_HABI,LOW);
  }

}


void getLED() { // Funcion al recibir petición GET
  // devolver respuesta
  server.send(200, "text/plain", String("GET ") + server.arg(String("Id")));
}


void setLED() { // Funcion al recibir petición POST
  // mostrar por puerto serie
  //Serial.println(server.argName(0));

  // devolver respuesta
  server.send(200, "text/plain", String("Litros: ") +  litros);
}

void handleNotFound() { // Página de error
  server.send(404, "text/plain", "Not found");
}

void verifica() {

  //Serial.print("Valor string: ");
  //Serial.println(msg); //Muestra valor del string
  //Serial.print("Valor int: ");
  msg.toCharArray(msg2, 10);
  //Serial.println(msg2); //Muestra el valor int
  valor_decimal = strtol(msg2, NULL, 16);
  //Serial.print("Valor decimal: ");
  //Serial.println(valor_decimal);
  lcd.setCursor(0, 0);
  lcd.print("ID:");
  lcd.setCursor(4, 0);
  lcd.print(valor_decimal);
}

void setup(void) {

  Serial.begin(9600);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
    delay(500);

  //Serial.println("");
  //Serial.println(WiFi.localIP());
  lcd.setCursor(4, 0);
  lcd.print(WiFi.localIP());

  // Definimos dos routeos
  server.on("/led", HTTP_GET, getLED);
  server.on("/led", HTTP_POST, setLED);

  server.onNotFound(handleNotFound);

  server.begin();
  //Serial.println("HTTP server started");

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("IP:");
  lcd.setCursor(0, 1);
  lcd.print("Litros:");


  //pinMode(LED_HABI,OUTPUT);
  //digitalWrite(LED_HABI,LOW);
  pinMode(CAUDALIMETRO, INPUT_PULLUP);
  pinMode(PALANCA, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(CAUDALIMETRO), parpadeo, FALLING);
  attachInterrupt(digitalPinToInterrupt(PALANCA), reseteo, CHANGE);
}

void loop() {
  server.handleClient();
  while (Serial.available() > 0) {
    delay(5); //================================================ without this delay it dosn't  work!===============
    c = Serial.read();
    msg += c;
    //Serial.println(msg);  //Uncomment to view your tag ID
    //Serial.println(msg.length());
  }
  msg = msg.substring(5, 11);
  if (msg.length() > 5) {
    verifica();
    msg = "";
  }

}
/*

   LCD RS pin 12
   LCD Enable pin 11
   LCD D4 pin 5
   LCD D5 pin 4
   LCD D6 pin 3
   LCD D7 pin 2
   LCD R/W a masa
   LCD VSS a masa
   LCD VCC a 5V
   Potenciometro de 10K:
   A +5V y masa
   Centro a VO del LCD
*/
