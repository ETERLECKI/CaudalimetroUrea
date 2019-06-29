#include <LiquidCrystal.h>

// Inicialización de display LCD
// con los pines asignados (Ver al final disposición de pines)
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define CAUDALIMETRO 2 //Conexión de caudalímetro, con resistencia de 4K7 a 5v, el otro cable de caudalímetro a masa
#define LED_HABI 9 // Habilitación de bomba (Relé + led)
#define PALANCA 18 // Conectado con una resistencia a +5V y con la palanca de habilitación (NC) a masa
volatile int estado = HIGH; //estado inicial
volatile float pulsos=0;
int reset = 0;
int habilita = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_HABI,OUTPUT);
  digitalWrite(LED_HABI,LOW);
  pinMode(CAUDALIMETRO, INPUT_PULLUP);
  pinMode(PALANCA, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CAUDALIMETRO),parpadeo,FALLING);
  attachInterrupt(digitalPinToInterrupt(PALANCA),reseteo,CHANGE);
  delay(2000);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Pulsos:");
  lcd.setCursor(0, 1);
  lcd.print("Litros:");
}

void parpadeo() {

    if(habilita==1){
      lcd.setCursor(9,0);
      lcd.print("1");
      estado = !estado;
      pulsos=pulsos+1;
      lcd.setCursor(9,0);
      lcd.print(pulsos);
      lcd.setCursor(9,1);
      lcd.print(pulsos/88);
  }

}

void reseteo(){
  
    if(digitalRead(PALANCA)==HIGH){
      Serial.println("Reset LCD");
      lcd.setCursor(9,1);
      lcd.print(pulsos/88);
      habilita=1;
      Serial.println("Enciendo Led");
      digitalWrite(LED_HABI,HIGH);
    } else{
      pulsos=0;
      habilita=0;
      delay(2000);
      Serial.println("Apago Led");
      digitalWrite(LED_HABI,LOW);
    }
    
  }

void loop() {
 
}
/*

 * LCD RS pin 12
 * LCD Enable pin 11
 * LCD D4 pin 5
 * LCD D5 pin 4
 * LCD D6 pin 3
 * LCD D7 pin 2
 * LCD R/W a masa
 * LCD VSS a masa
 * LCD VCC a 5V
 * Potenciometro de 10K:
 * A +5V y masa
 * Centro a VO del LCD
*/
