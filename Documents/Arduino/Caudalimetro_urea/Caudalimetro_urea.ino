#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>
#include <stdlib.h>
#include <FS.h>

const int RS = D2, EN = D3, d4 = D5, d5 = D6, d6 = D7, d7 = D8;   //Descripción de pines LCD
LiquidCrystal lcd(RS, EN, d4, d5, d6, d7); //Parametrizo LCD

String msg; //Valor leído del llavero
char msg2[5]; //Array del valor leído para convertirlo en entero
long int valor_decimal; //Valor decimal de los datos leídos
const byte CAUDALIMETRO = D4; //Conexión de caudalímetro, con resistencia de 4K7 a 5v, el otro cable de caudalímetro a masa
const byte HABILITA_B = D0; // Habilitación de bomba (Relé + led)
const byte PALANCA = D1; // Conectado con una resistencia a +5V y con la palanca de habilitación (NC) a masa
volatile int estado = HIGH; //estado inicial
volatile float pulsos = 0;
volatile float litros;
int habilita = 0;
char c; //Captura los caracteres individuales tomados del RFID
boolean autorizado = false;
const char* filename = "/despachos.txt"; //Archivo donde se guardan los despachos no guardados
JsonObject obj;
String datos;

const char* ssid = "NBcargp";
const char* password = "(*2468*)";
String AutorizaHost = "http://192.168.5.14/surtidores/autorizacion_urea.php/?IdTarjeta="; //url de la autorización del RFID (Falta el Id de la tarjeta)
String Guarda_cargaHost = "http://192.168.5.14/surtidores/Guarda_Carga.php";

int Json_IdChofer;
int Json_IdVehiculo;
int Json_Odometro;
int Json_RegDatosTarjeta;

ESP8266WebServer server(80); //Llamo server al servidor
HTTPClient http;
WiFiClient cliente;

//void ICACHE_RAM_ATTR parpadeo();
//void ICACHE_RAM_ATTR reseteo();

//Creo caracter flecha hacia abajo para LCD
byte flechaAbajo[] = {
  B00000,
  B00100,
  B00100,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100
};

void resetLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Aproxime llavero");
  lcd.setCursor(0, 1);
  lcd.print("  al lector ");
  lcd.setCursor(13, 1);
  lcd.write(1);
}

void parpadeo() {

  if (habilita == 1) { // Si todoas las comprobaciones son correctas
    estado = !estado;
    pulsos = pulsos + 1;
    //lcd.setCursor(9, 0);
    //lcd.print(pulsos);
    litros = pulsos / 88;
    lcd.setCursor(0, 1);
    lcd.print(litros);
    //Serial.print("Litros: ");
    //Serial.println(litros);
  }

}

void reseteo() {
  delay(200);
  //Serial.println("Entro a reseteo");
  if ((digitalRead(PALANCA) == HIGH) and (autorizado = true)) {
    //Serial.println("Reset LCD");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Litros:");
    lcd.setCursor(0, 1);
    lcd.print(litros);
    habilita = 1;
    digitalWrite(HABILITA_B, HIGH); //Habilito la bomba
  } else if ((digitalRead(PALANCA) == LOW) and (autorizado = true)) {
    pulsos = 0;
    habilita = 0;
    digitalWrite(HABILITA_B, LOW); //Corto la bomba
    if (litros > 0) {
    
      //Codifico datos en Json
      StaticJsonDocument<189> doc2;
 
      doc2["IdChofer"] = Json_IdChofer;
      doc2["IdVehiculo"] = Json_IdVehiculo;
      doc2["Odometro"] = Json_Odometro;
      doc2["IdProducto"] = 1;
      doc2["VentaManual"] = false;
      doc2["Litros"] = litros;
      doc2["IdRegDatosTarjeta"] = Json_RegDatosTarjeta;
 
      serializeJson(doc2, datos);
      delay(10);
      //Fin codifico datos en Json
      
      if (cliente){
      
       if (http.begin(Guarda_cargaHost)) //Iniciar conexión
   {
      Serial.print("[HTTP] POST...\n");
      int httpCode = http.POST(datos);  // Realizar petición
 
      if (httpCode > 0) {
         Serial.printf("[HTTP] POST... code: %d\n", httpCode);
 
         if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload1 = http.getString();   // Obtener respuesta
            Serial.println(payload1);   // Mostrar respuesta por serial
         }
      }
      else {
         Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
 
      http.end();
   }
   else {
      Serial.printf("[HTTP} Unable to connect\n");
   }
   Serial.println("Error wifi");
      }
            //Especifica la url del servidor con el valor de los parámetros
            
            
            
      /*http.begin(cliente,Guarda_cargaHost);
      //int httpCode1 = http.GET();            //Envia solicitud
      int httpCode1 = http.POST(datos);
      //Envia solicitud
      String payload1 = http.getString();    //Obtiene respuesta de servidor


      Serial.print("Response Code:"); //200 es OK
      Serial.println(httpCode1);

      if (httpCode1 == 200) {
        Serial.print("Respuesta:");
        Serial.println(payload1);*/

        // JsonBuffer
        // Usar arduinojson.org/assistant para calcular.
        const size_t capacity1 = JSON_OBJECT_SIZE(2) + 120;
        DynamicJsonDocument doc1(capacity1);

        // Parse JSON object
        DeserializationError err1 = deserializeJson(doc1, payload1);
        if (err1) {
          Serial.print(F("deserializeJson() returned "));
          Serial.println(err1.c_str());
          return;
        }

        // Decode JSON/Extraer valores

        //int Json_IdChofer = doc[0];
        String Json_Status = doc1["error"];
        if (Json_Status) {
          String Json_Msg_Error = doc1["message"];

          //Guardar datos en memoria EEPROM
          //Create New File And Write Data to It
          //w=Write Open file for writing
          File f = SPIFFS.open(filename, "w");

          if (!f) {
            Serial.println("file open failed");
          }
          else
          {
            //Write data to file
            Serial.println("Writing Data to File");
            f.print("{Id_Chofer:" + String(Json_IdChofer) + ",Id_Vehiculo:" + String(Json_IdVehiculo) + ",Odometro:" + String(Json_Odometro) + ",Id_RegDatosTarjeta:" + String(Json_RegDatosTarjeta) + "}");
            f.close();  //Close file
          }
          //Fin de guardar datos en EEPROM

          lcd.clear();
          lcd.print("Error en el guardado");
          Serial.println("Error en el guardado");

        } else {
          lcd.clear();
          lcd.print("Despacho Guardado");
          Serial.println("Despacho Guardado");
        }
      }
      
    } else {
      Serial.println("Litros bajos");
    }
    
    http.end();  //Cierro la conexión
    litros = 0;
    delay(5000);
    autorizado = false;
    resetLCD();
    
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
  Serial.print("Valor decimal: ");
  Serial.println(valor_decimal);
  Serial.println(AutorizaHost + valor_decimal);
  //Agregado Autoriza.PHP

  http.begin(AutorizaHost + valor_decimal);     //Especifica la url del servidor con el valor de la tarjeta

  int httpCode = http.GET();            //Envia solicitud
  String payload = http.getString();    //Obtiene respuesta de servidor


  Serial.print("Response Code:"); //200 es OK
  Serial.println(httpCode);

  if (httpCode == 200) {
    Serial.print("Respuesta:");
    Serial.println(payload);

    // JsonBuffer
    // Usar arduinojson.org/assistant para calcular.
    const size_t capacity = JSON_OBJECT_SIZE(5) + 80;
    DynamicJsonDocument doc(capacity);

    // Parse JSON object
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
      Serial.print(F("deserializeJson() returned "));
      Serial.println(err.c_str());
      return;
    }

    // Decode JSON/Extraer valores

    //int Json_IdChofer = doc[0];
    int Json_Estado = doc["Estado"];

    switch (Json_Estado) {
      //Autorizado
      case 7:
        {
          Json_IdVehiculo = doc["Id_Vehiculo"];
          Json_IdChofer = doc["Id_Chofer"];
          Json_Odometro = doc["Odometro"];
          Json_RegDatosTarjeta = doc["Id_RegDatosTarjeta"];
          Serial.print("Id_Chofer:");
          Serial.println (Json_IdChofer);
          Serial.print("Id_Vehiculo:");
          Serial.println (Json_IdVehiculo);
          Serial.print("Odometro:");
          Serial.println (Json_Odometro);
          Serial.print("Id_RegDatosTarjeta:");
          Serial.println (Json_RegDatosTarjeta);

          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Carga");
          lcd.setCursor(0, 1);
          lcd.print("Autorizada");
          autorizado = true;
          delay(5);
          break;
        }

      case 6:
        {
          Serial.println("Llavero no registrado");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Llavero no");
          lcd.setCursor(0, 1);
          lcd.print("Registrado");
          delay(5000);
          resetLCD();
          break;
        }

      case 5:
        {
          Serial.println("Chofer No Autorizado");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("CHOFER");
          lcd.setCursor(0, 1);
          lcd.print("NO AUTORIZADO");
          delay(5000);
          resetLCD();
          break;
        }

      case 4:
        {
          Serial.println("Cargar gasoil previamente");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Realizar Carga");
          lcd.setCursor(0, 1);
          lcd.print("de GASOIL");
          delay(5000);
          resetLCD();
          break;
        }

      case 3:
        {
          Serial.println("Error consulta Vehiculo");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Carga");
          lcd.setCursor(0, 1);
          lcd.print("Autorizada");
          delay(5000);
          resetLCD();
          break;
        }

      case 2:
        {
          Serial.println("Error consulta RegChofer");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Error conexión");
          lcd.setCursor(0, 1);
          lcd.print("BD Vehiculo");
          delay(5000);
          resetLCD();
          break;
        }

      case 1:
        {
          Serial.println("Error consulta Autorización");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Error conexión");
          lcd.setCursor(0, 1);
          lcd.print("BD Autorizacion");
          delay(5000);
          resetLCD();
          break;
        }

      case 0:
        {
          Serial.println("Error consulta Chofer");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Error conexión");
          lcd.setCursor(0, 1);
          lcd.print("BD Chofer");
          delay(5000);
          resetLCD();
          break;
        }
    }

    //Serial.println(Json_IdChofer);
    //Serial.println(Json_IdVehiculo);
    //Serial.println(Json_IdEstado);

    //lcd.setCursor(0, 1);
    //lcd.print(payload);
  }
  else
  {
    Serial.println("Error de conexión");
  }

  http.end();  //Cierro la conexión

  delay(5000);  //Demoro 5 segundos antes de enviar una nueva petición


  //Fin agregado Autoriza.PHP
}

void setup(void) {

  litros = 0;

  Serial.begin(9600);

  WiFi.begin(ssid, password);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  //Serial.println("");
  //Serial.println(WiFi.localIP());
  // Definimos dos routeos
  server.on("/led", HTTP_GET, getLED);
  server.on("/led", HTTP_POST, setLED);

  server.onNotFound(handleNotFound);

  server.begin();
  //Serial.println("HTTP server started");

  lcd.begin(16, 2);

  //pinMode(LED_HABI,OUTPUT);
  //digitalWrite(LED_HABI,LOW);
  pinMode(CAUDALIMETRO, INPUT_PULLUP);
  pinMode(PALANCA, INPUT_PULLUP);
  pinMode(HABILITA_B, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(CAUDALIMETRO), parpadeo, FALLING);
  attachInterrupt(digitalPinToInterrupt(PALANCA), reseteo, CHANGE);

  lcd.createChar(1, flechaAbajo);

  resetLCD();

  //Agregado de guardado en memoria interna

  //Initialize File System
  if (SPIFFS.begin())
  {
    Serial.println("SPIFFS Inicializado");
  }
  else
  {
    Serial.println("Error SPIFFS");
  }

  //Format File System
  if (SPIFFS.format())
  {
    Serial.println("SPIFFS File System Formated");
  }
  else
  {
    Serial.println("SPIFFS File System Formatting Error");
  }

  int i;

  //Read File data
  File f = SPIFFS.open(filename, "r");

  if (!f) {
    Serial.println("file open failed");
  }
  else
  {
    Serial.println("Reading Data from File:");
    //Data from file
    for (i = 0; i < f.size(); i++) //Read upto complete file size
    {
      if (f.size() != 0) {
        Serial.print((char)f.read());
      }

    }
    f.close();  //Close file
    Serial.println("File Closed");


    //Fin de agregado guardado en memoria interna
  }
}

void loop() {
  HTTPClient http;
  WiFiClient cliente;
  server.handleClient();
  //HTTPClient http;

  if (habilita == 0) {
    while (Serial.available() > 0) {
      delay(5); //================================================ without this delay it dosn't  work!===============
      c = Serial.read();
      msg += c;

    }
    msg = msg.substring(5, 11);
    if (msg.length() > 5) {
      verifica();
      msg = "";
    }
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
