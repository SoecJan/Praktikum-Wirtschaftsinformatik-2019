#include <SPI.h>
#include <MFRC522.h>

#include <LiquidCrystal.h>//Don't forget to enter this library

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

String taskDescription = "Auftrag 11104"; // Selected Task
const int SW_pin = 8; // digital pin connected to switch output
const int X_pin = 1; // analog pin connected to X output
const int Y_pin = 2; // analog pin connected to Y output

unsigned long organizerStartTime; // Time Gerd has been at work
unsigned long organizerEndTime;
unsigned long workerStartTime; // Time manni has been at work
unsigned long workerEndTime;

bool eventTriggered = true; // Trigger if Gerd has unlocked the terminal
#define ledRed A4
#define ledGreen A5
#define SS_PIN 10
#define RST_PIN 9
byte nuidPICC[4];
//String worker = "3DD45452";
//String organizer = "E0C1211A";
String worker = "E0C1211A";
//String organizer = "3DD45452";
String organizer = "860F2325";

bool workerOnWork = false;
bool organizerOnWork = false;
bool registerPossible = false;

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;

int code[] = {32, 154, 149, 117}; //This is the stored UID (Unlock Card)
int codeRead = 0;
String uidString;
void setup() {
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  analogWrite(ledRed, 255);
  lcd.begin(16, 2); // Setup Display
  pinMode(SW_pin, INPUT); // Setup Button input pin
  digitalWrite(SW_pin, HIGH);
  Serial.begin(115200); // Setup Serial Port for Debugging

  lcd.setCursor(0, 0);
  lcd.print("Auftrag 11104 x");

  lcd.setCursor(0, 1); // print the number of seconds since reset:
  lcd.print("Auftrag 11105");

  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

}

void loop() {
  if (  rfid.PICC_IsNewCardPresent())
  {
    readRFID();
  }

  //delay(100);
  HandleInput();
}

void HandleInput()
{
  if (eventTriggered)
  {

    lcd.setCursor(0, 0);
    lcd.print(taskDescription + "  ");
    lcd.setCursor(0, 1);
    lcd.print("               ");
    return;
  }
  int y = analogRead(Y_pin);
  int x = analogRead(X_pin);
  if (x < 300)
  {
    lcd.setCursor(0, 0);
    lcd.print("Auftrag 11104  ");

    lcd.setCursor(0, 1); // print the number of seconds since reset:
    lcd.print("Auftrag 11105 x");

    taskDescription = "Auftrag 11105";
  }
  else if (x > 800)
  {
    lcd.setCursor(0, 0);
    lcd.print("Auftrag 11104 x");

    lcd.setCursor(0, 1); // print the number of seconds since reset:
    lcd.print("Auftrag 11105  ");

    taskDescription = "Auftrag 11104";
  }

  if (y > 800 && !eventTriggered) {
    Serial.println("Selected: " + taskDescription);
    Serial.println("Gerd hat seine Arbeit an Auftrag " + taskDescription + " gestartet.");
    organizerStartTime = millis();
    Serial.println("Anmeldung für weitere Arbeiter nun möglich");
    Serial.println("============================");

    analogWrite(ledGreen, 255);
    tone(A3, 2093, 200);
    eventTriggered = true;
  }
  // Serial.println(digitalRead(SW_pin));
  if (digitalRead(SW_pin) == 0)
  {
    //Serial.println(digitalRead(SW_pin));
    Serial.println("Selected: " + taskDescription);
    Serial.println("Gerd hat seine Arbeit an Auftrag " + taskDescription + " gestartet.");
    tone(A3, 2093, 200);
    //tone(A3, 2093, 400);
    //delay(400);
    //tone(A3, 2637, 400);
    //delay(400);
    //tone(A3, 3136, 400);
    //delay(400);
    //tone(A3, 2637, 250);
    //delay(250);
    //tone(A3, 3136, 500);
    eventTriggered = true;
  }
}

void readRFID()
{

  rfid.PICC_ReadCardSerial();
  //Serial.print(F("\nPICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  //Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }


  //Serial.println("Scanned PICC's UID:");
  //printDec(rfid.uid.uidByte, rfid.uid.size);

  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }
  /*
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  */
  byte array[4] = {0xAB, 0xCD, 0xEF, 0x99};
  char str[8] = "";
  //array_to_string(rfid.uid.uidByte, rfid.uid.size, str);
  array_to_string(nuidPICC, 4, str);
  //Serial.println(str);


  if (worker.equals(str)) {

    tone(A3, 1047, 200);

    String tmpKey = "Arbeiter mit Kennung: ";
    if (workerOnWork == true ) {
      tmpKey = tmpKey + str;
      tmpKey = tmpKey + " abgemeldet.";
      workerOnWork = !workerOnWork;
      workerEndTime = (millis() - workerStartTime)/1000;
      Serial.println(tmpKey);
      Serial.print("Arbeitszeit zwischen An- und Abmeldung: ");
      Serial.print(workerEndTime);
      Serial.print(" Sekunden.");
      Serial.println();
      Serial.println("============================");
    } else if (workerOnWork == false && eventTriggered == true && registerPossible == true) {
      if (organizerOnWork == true ) {
        tmpKey = tmpKey + str;
        tmpKey = tmpKey + " angemeldet.";
        Serial.println(tmpKey);
        Serial.println("============================");
        workerOnWork = !workerOnWork;
        workerStartTime = millis();
      } else {
        tone(A3, 523, 200);
        Serial.println("Anmeldung von Arbeiter verweigert. Terminal nicht von Vorarbeiter freigegeben.");
      }

    }
    delay(1000);
  }
  if (organizer.equals(str)) {

    tone(A3, 523, 200);

    String tmpKey = "Ingenieur mit Kennung: ";
    if (organizerOnWork == true) {
      tmpKey = tmpKey + str;
      tmpKey = tmpKey + " abgemeldet.";
      organizerOnWork = !organizerOnWork;
      eventTriggered = true;

      Serial.println(tmpKey);
      organizerEndTime = millis() - organizerStartTime;
      Serial.print("Arbeitszeit zwische An- und Abmeldung: ");
      Serial.print(organizerEndTime/1000);
      Serial.print(" Sekunden.");
      registerPossible = !registerPossible;
      Serial.println("Anmeldung für Arbeiter zurzeit nicht mehr möglich.");
      if (workerOnWork == true) {
        workerOnWork = !workerOnWork;
        Serial.println("Verbliebene Arbeiter abgemeldet.");
      }
    } else if (organizerOnWork == false) {

      eventTriggered = false;
      tmpKey = tmpKey + str;
      tmpKey = tmpKey + " angemeldet.";
      Serial.println(tmpKey);
      organizerOnWork = !organizerOnWork;
      registerPossible = !registerPossible;
      
    }
    if (organizerOnWork == false) {
      Serial.println("Anmeldung gesperrt.");
      analogWrite(ledRed, 255);
      analogWrite(ledGreen, 0);
    } else if (organizerOnWork == true) {
      Serial.println("Anmeldung offen.");
      analogWrite(ledRed, 0);
      
    }
    delay(1000);
    Serial.println("============================");
   

    for (int i = 0; i < 8; i++) {
      str[i] = i ;
    }
    // Halt PICC
    rfid.PICC_HaltA();

    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();

    lcd.setCursor(0, 0);
    lcd.print("Auftrag 11104 x");

    lcd.setCursor(0, 1); // print the number of seconds since reset:
    lcd.print("Auftrag 11105  ");

    taskDescription = "Auftrag 11104";
  }
}
void printDec(byte * buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
void printHex(byte * buffer, byte bufferSize) {

  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
void array_to_string(byte array[], unsigned int len, char buffer[])
{
  for (unsigned int i = 0; i < len; i++)
  {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i * 2 + 0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i * 2 + 1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  buffer[len * 2] = '\0';
}

void setupRFIDReader() {
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}
