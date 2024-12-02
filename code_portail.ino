#include <SR04.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <SPI.h>
#include <MFRC522.h>
#define fc1  22
#define fc2  23
#define moteur_sens1  24
#define moteur_sens2  25
#define recepteur  8
#define SDA_PIN 53
#define RESET_PIN 5
#define ledjaune 27 //led qui clignote quand le portail est ouvert
#define ledrouge 49 //qui indique que le badge est non valide
#define ledverte 28 //qui indique que le badge eest bon
#define max_test 3
#define TRIG_PIN 12
#define ECHO_PIN 11
SR04 sr04 = SR04(ECHO_PIN, TRIG_PIN);
long a;
const int ir_sensor = 9;
#define UID {0xB7, 0xDA, 0x95, 0x53}
byte badge_lu = 0;
byte verifcode = 0;
int buzzer = 13;
byte compteur_acces = 0;
byte nuidPICC[4];
volatile int val = LOW;
MFRC522 mfrc522(SDA_PIN, RESET_PIN);
LiquidCrystal_I2C LCD(0x27, 16, 2);
byte NEW_UID[4] = {0xB7, 0xDA, 0x95, 0x53};
MFRC522::MIFARE_Key key;
IRrecv irrecv(recepteur) ;
decode_results resultat;


void setup() {
  // put your setup code here, to run once:
  pinMode(moteur_sens1, OUTPUT);
  pinMode(moteur_sens2, OUTPUT);
  pinMode(fc1, INPUT_PULLUP);
  pinMode(fc2, INPUT_PULLUP);
  pinMode( ir_sensor, INPUT);
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(ledverte, OUTPUT);
  pinMode(ledjaune, OUTPUT);
  pinMode(ledrouge, OUTPUT);
  digitalWrite(ledjaune, LOW);
  digitalWrite(ledrouge, LOW);
  digitalWrite(ledverte, LOW);
  pinMode(buzzer, OUTPUT);
  // Init SPI bus
  SPI.begin();
  while (!Serial);
  // Init MFRC522
  mfrc522.PCD_Init();
  for (byte i = 0; i < 4; i++)
  {
    key.keyByte[i] = 0xFF;
  }
  LCD.init();
  LCD.backlight();
  LCD.setCursor(3, 0);
  LCD.print("BIENVENUE");
}
void telecommande_IR()
{
  switch (resultat.value)
  {
    case 0xFFA25D: Serial.println("Demarrage du moteur");
      digitalWrite(ledverte, HIGH);
      delay(2000);
      digitalWrite(ledverte, LOW);
      moteur_sensnormal();
      moteur_sensinverse();
      moteur_stop();
      porte_completement_ouverte();
      ouvrir_porte();
      porte_completement_fermee();
      ferme_porte();
  }
}

void affichage()
{

  if (!mfrc522.PICC_IsNewCardPresent())
    return;
  if (!mfrc522.PICC_ReadCardSerial())
    return;

  for (byte i = 0; i < 6; i++) {
    nuidPICC[i] = mfrc522.uid.uidByte[i];
  }
  //verification du code
  verifcode = GetAccesState(NEW_UID, nuidPICC);
  if (verifcode != 1) {
    compteur_acces += 1;
    if (compteur_acces == max_test)
    {
      //depassement des tentatives(clignotement infinie)
      while (1) {
        digitalWrite(ledrouge, HIGH);
        delay(200);
        digitalWrite(ledrouge, LOW);
        delay(200);
        //affichage
        Serial.println("alarme!");
      }

    }
    else
    {
      //affichage
      Serial.println("code érroné");
      digitalWrite(ledrouge, HIGH);
      delay(1000);
      digitalWrite(ledrouge, LOW);
    }
    LCD.clear();
    LCD.setCursor(4, 0);
    LCD.print("MAUVAIS");
    LCD.setCursor(5, 1);
    LCD.print("BADGE");
    delay(3000);
    LCD.clear();
  }

  else {
    // ouverture de laa porte et initialisation
    digitalWrite(ledverte, HIGH);
    delay(2000);
    digitalWrite(ledverte, LOW);
    moteur_sensnormal();
    moteur_sensinverse();
    moteur_stop();
    porte_completement_ouverte();
    ouvrir_porte();
    porte_completement_fermee();
    ferme_porte();

    compteur_acces = 0;
  }

  Serial.println("L'UID du tag est: ");
  for (byte i = 0; i < 6; i++)
  {
    Serial.print(nuidPICC[i], i++);
    Serial.print("");
  }
  Serial.println();
  //re-init RFID
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

byte GetAccesState(byte *codeacces, byte *newcode)
{
  byte StateAcces = 0;
  if ((codeacces[0] == newcode[0]) && (codeacces[1] == newcode[1]) && (codeacces[2] == newcode[2]) && (codeacces[3] == newcode[3]))
    return StateAcces = 1;
  else
    return StateAcces = 0;
}


void module_rfid() {
  // Look for new cards, and select one if present
  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
    delay(50);
    return;
  }

  // Dump UID
  Serial.print(F("Card UID:"));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  byte newUid[] = UID;
  if ( mfrc522.MIFARE_SetUid(newUid, (byte)4, true) ) {
    Serial.println(F("Wrote new UID to card."));
  }
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void loop() {
  //int fc = digitalRead(fc2);
  //Serial.println(fc);
  int nouveletat = digitalRead(ir_sensor);
  Serial.println(nouveletat);

  module_rfid();
  affichage();
  if (irrecv.decode(&resultat)) // have we received an IR signal?
  {
    telecommande_IR();
    irrecv.resume(); // receive the next value
  }
  sortir();
}

void moteur_sensnormal()
{
  // digitalWrite(ledjaune, HIGH);
  digitalWrite(moteur_sens1, HIGH);
  digitalWrite(moteur_sens2, LOW);
}
void moteur_sensinverse()
{
  digitalWrite(moteur_sens1, LOW);
  digitalWrite(moteur_sens2, HIGH);
  // digitalWrite(ledjaune, HIGH);
}
void moteur_stop()
{
  digitalWrite(moteur_sens1, LOW);
  digitalWrite(moteur_sens2, LOW);
  digitalWrite(ledjaune, LOW);
}
//fonction qui dit si le portail est completement ouvert en lisant le switch fin de course
bool porte_completement_ouverte()
{

  if (digitalRead(fc1) == HIGH)
    return true;//oui
  else
    return false;//non voir la porte completement fermee
}
// fonction qui va ouvrir la porte
void ouvrir_porte()
{
  Serial.println("ouverture du portail");
  moteur_sensnormal();
  while (!porte_completement_ouverte()) {
    //on attend la fin de la course
    LCD.clear();
    LCD.setCursor(2, 0);
    LCD.print("OUVERTURE DU");
    LCD.setCursor(4, 1);
    LCD.print("PORTAIL");

    if (!porte_completement_ouverte() ) {
      //on attend la fin de la course
      digitalWrite(ledjaune, HIGH);
      delay(1000);
      digitalWrite(ledjaune, LOW);
      digitalWrite(buzzer, HIGH);
      delay(1000);
      digitalWrite(buzzer, LOW);
    }
  }

  moteur_stop();

}
// voir portail ferme

bool porte_completement_fermee()
{
  if (digitalRead(fc2) == HIGH)
    return true;//oui
  else
    return false;//non voir porte completement ouverte
}

void ferme_porte()
{
  LCD.clear();
  LCD.setCursor(1, 0);
  LCD.print("PORTAIL OUVERT");
  LCD.setCursor(3, 1);
  LCD.print("TRAVERSEZ");
  delay(4000);
  Serial.println("fermeture du portail");
  moteur_sensinverse();
  while (!porte_completement_fermee()) {
    //on attend la fin de course
    int nouveletat = digitalRead(ir_sensor);
    Serial.println(nouveletat);
    val = nouveletat;
    if (nouveletat == LOW) {
      Serial.println("obstacle détecté");
      LCD.clear();
      LCD.setCursor(3, 0);
      LCD.print("OBSTACLE");
      LCD.setCursor(3, 1);
      LCD.print("DETECTE");
      delay(500);
      moteur_sensnormal();
      moteur_sensinverse();
      moteur_stop();
      porte_completement_ouverte();
      ouvrir_porte();
      porte_completement_fermee();
      ferme_porte();
    }
    if (!porte_completement_fermee()) {
      //on attend la fin de la course
      digitalWrite(ledjaune, HIGH);
      delay(1000);
      digitalWrite(ledjaune, LOW);
      digitalWrite(buzzer, HIGH);
      delay(1000);
      digitalWrite(buzzer, LOW);
    }
    LCD.clear();
    LCD.setCursor(2, 0);
    LCD.print("FERMETURE DU");
    LCD.setCursor(4, 1);
    LCD.print("PORTAIL");
  }
  moteur_stop();
  LCD.clear();
  LCD.setCursor(1, 0);
  LCD.print("PORTAIL FERME");
  delay(2000);
  LCD.clear();
}
void sortir() {
  a = sr04.Distance();
  Serial.print(a);
  Serial.println("cm");
  delay(1000);
  if ( a < 5) {
    digitalWrite(ledverte, HIGH);
    delay(2000);
    digitalWrite(ledverte, LOW);
    moteur_sensnormal();
    moteur_sensinverse();
    moteur_stop();
    porte_completement_ouverte();
    ouvrir_porte();
    porte_completement_fermee();
    ferme_porte();
  }
}
