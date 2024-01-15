#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 rfid(SS_PIN, RST_PIN);   // Объект rfid модуля
MFRC522::MIFARE_Key key;         // Объект ключа

int numOfDoor = 0;
uint32_t rebootTimer = millis();

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  rfid.PCD_SetAntennaGain(rfid.RxGain_max);  // Установка усиления антенны

  rfid.PCD_AntennaOff();
  rfid.PCD_AntennaOn();

  for (byte i = 0; i < 6; ++i)
  {
    key.keyByte[i] = 0xFF;
  }
}

void loop() {
  if (numOfDoor == -1)
  {
    numOfDoor = Serial.parseInt();
  }

  if ((millis() - rebootTimer) >= 1000)
  {
    rebootTimer = millis();

    digitalWrite(RST_PIN, HIGH);          // Сбрасываем модуль
    delayMicroseconds(2);
    digitalWrite(RST_PIN, LOW);           // Отпускаем сброс
    rfid.PCD_Init();
  }

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())  // метка поднесена и читается
  {
    if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, ((((numOfDoor / 4) + 1) * 4) - 1), &key, &(rfid.uid))
        == MFRC522::STATUS_OK)
    {
      uint8_t dataToWrite[16];
      for (int i = 0; i < 16; ++i)
      {
        dataToWrite[0] = 1;
      }

      if (rfid.MIFARE_Write(numOfDoor, dataToWrite, 16) != MFRC522::STATUS_OK)
      {
        Serial.println("Bad write");
      }
      else
      {
        Serial.println("Good write");
        rfid.PICC_HaltA();
        numOfDoor = -1;
      }
    }
  }
}
