#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>

#define ledRed A4
#define ledGreen A5
#define SS_PIN 10
#define RST_PIN 9

const int SW_pin = 8; // digital pin connected to switch output
const int X_pin = 1; // analog pin connected to X output
const int Y_pin = 2; // analog pin connected to Y output

unsigned long organizerStartTime; // Start time organizer has been at work
unsigned long organizerEndTime; // Time organizer left work
unsigned long workerStartTime; // Start time worker has been at work
unsigned long workerEndTime; // Time worker left work

// Selected Task
String taskDescription = "Auftrag 11104";
//IDs of Worker and Organizer
String worker = "E0C1211A";
String organizer = "860F2325";

// Helper boolean
bool workerOnWork = false;
bool organizerOnWork = false;
bool registerPossible = false;

// Trigger if oragnizer has unlocked the terminal
bool eventTriggered = true;

byte nuidPICC[4];

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

// Settings at wich terminal is started
void setup() {
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);

  analogWrite(ledRed, 255);

  // Setup Button input pin
  pinMode(SW_pin, INPUT);
  digitalWrite(SW_pin, HIGH);

  // Setup Display
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Auftrag 11104 x");
  lcd.setCursor(0, 1);
  lcd.print("Auftrag 11105");

  // Setup RFID-reader
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.begin(9600);

}

void loop()
{
  if (  rfid.PICC_IsNewCardPresent())
  {
    readRFID();
  }
  handleInput();
}

void handleInput()
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

  // Change selected Task on display depending on joystick movements
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

    lcd.setCursor(0, 1);
    lcd.print("Auftrag 11105  ");

    taskDescription = "Auftrag 11104";
  }

  //Check if joystick is moved right and if task wasn't already choosen.
  if (y > 800 && !eventTriggered)
  {
    Serial.println("Ausgewählt: " + taskDescription);
    Serial.println("Vorarbeiter hat seine Arbeit an Auftrag " + taskDescription + " gestartet.");
    Serial.println("Anmeldung für weitere Arbeiter nun möglich");
    Serial.println("============================");

    organizerStartTime = millis();
    analogWrite(ledGreen, 255);
    tone(A3, 2093, 200);
    eventTriggered = true;
  }

  if (digitalRead(SW_pin) == 0)
  {
    Serial.println("Ausgewählt: " + taskDescription);
    Serial.println("Vorarbeiter hat seine Arbeit an Auftrag " + taskDescription + " gestartet.");
    tone(A3, 2093, 200);
    eventTriggered = true;
  }
}

// Method to sign in and logout employees.
void readRFID()
{
  rfid.PICC_ReadCardSerial();
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

  //Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Tag entspricht nicht den genutzten MIFARE Classic RFID-Chips."));
    return;
  }

  //Read NUID of Chip
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }

  //Convert NUID-byte-array to hex-value and write result to string array
  byte array[4] = {0xAB, 0xCD, 0xEF, 0x99};
  char str[8] = "";
  array_to_string(nuidPICC, 4, str);


  //Check if read result equals known id of worker or organizer
  if (worker.equals(str))
  {
    tone(A3, 1047, 200);
    String tmpKey = "Arbeiter mit Kennung: ";

    // Check if boolean helper variable was true. If it was, invert value and logout. Start time recognition.
    if (workerOnWork == true )
    {
      tmpKey = tmpKey + str;
      tmpKey = tmpKey + " abgemeldet.";

      workerEndTime = (millis() - workerStartTime) / 1000;

      Serial.println(tmpKey);
      Serial.print("Arbeitszeit zwischen An- und Abmeldung: ");
      Serial.print(workerEndTime);
      Serial.print(" Sekunden.");
      Serial.println();
      Serial.println("============================");

      workerOnWork = !workerOnWork;
    } else if (workerOnWork == false && eventTriggered == true && registerPossible == true) {
      if (organizerOnWork == true )
      {
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
  if (organizer.equals(str))
  {
    tone(A3, 523, 200);
    String tmpKey = "Ingenieur mit Kennung: ";
    if (organizerOnWork == true)
    {
      tmpKey = tmpKey + str;
      tmpKey = tmpKey + " abgemeldet.";

      organizerEndTime = millis() - organizerStartTime;

      Serial.println(tmpKey);
      Serial.print("Arbeitszeit zwische An- und Abmeldung: ");
      Serial.print(organizerEndTime / 1000);
      Serial.print(" Sekunden.");
      Serial.println("Anmeldung für Arbeiter zurzeit nicht mehr möglich.");

      registerPossible = !registerPossible;
      organizerOnWork = !organizerOnWork;
      eventTriggered = true;
      if (workerOnWork == true)
      {
        workerOnWork = !workerOnWork;
        Serial.println("Verbliebene Arbeiter abgemeldet.");
      }
    } else if (organizerOnWork == false)
    {
      tmpKey = tmpKey + str;
      tmpKey = tmpKey + " angemeldet.";

      Serial.println(tmpKey);

      organizerOnWork = !organizerOnWork;
      registerPossible = !registerPossible;
      eventTriggered = false;
    }
    if (organizerOnWork == false)
    {
      Serial.println("Anmeldung gesperrt.");
      analogWrite(ledRed, 255);
      analogWrite(ledGreen, 0);
    } else if (organizerOnWork == true)
    {
      Serial.println("Anmeldung offen.");
      analogWrite(ledRed, 0);

    }
    delay(1000);
    Serial.println("============================");

    //Delete possible value of result array to prevent side effects at next iteration.
    for (int i = 0; i < 8; i++)
    {
      str[i] = i ;
    }

    // Halt PICC
    rfid.PICC_HaltA();

    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();

    lcd.setCursor(0, 0);
    lcd.print("Auftrag 11104 x");

    lcd.setCursor(0, 1);
    lcd.print("Auftrag 11105  ");

    taskDescription = "Auftrag 11104";
  }
}

// Method to Convert byte array to char array.
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
