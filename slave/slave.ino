#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN 9        // Пин rfid модуля RST
#define SS_PIN 10       // Пин rfid модуля SS
MFRC522 rfid(SS_PIN, RST_PIN);   // Объект rfid модуля
MFRC522::MIFARE_Key key;         // Объект ключа

void setup() {
  Serial.begin(9600);              // Инициализация Serial
  SPI.begin();                     // Инициализация SPI
  rfid.PCD_Init();                 // Инициализация модуля

  rfid.PCD_SetAntennaGain(rfid.RxGain_max);  // Установка усиления антенны

  rfid.PCD_AntennaOff();           // Перезагружаем антенну
  rfid.PCD_AntennaOn();            // Включаем антенну

  for (byte i = 0; i < 6; i++) {   // Наполняем ключ
    key.keyByte[i] = 0xFF;         // Ключ по умолчанию 0xFFFFFFFFFFFF
  }
}

int numOfDoor = 0;
uint32_t rebootTimer = millis();

void loop() {
  if (numOfDoor == 0)
  {
    numOfDoor = Serial.parseInt();
  }

  if ((millis() - rebootTimer) >= 1000)
  {
    rebootTimer = millis();

    digitalWrite(RST_PIN, HIGH);          // Сбрасываем модуль
    delayMicroseconds(2);                 // Ждем 2 мкс
    digitalWrite(RST_PIN, LOW);           // Отпускаем сброс
    rfid.PCD_Init();                      // Инициализируем заного
  }

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())  // метка поднесена и читается
  {
    if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 7, &key, &(rfid.uid)) != MFRC522::STATUS_OK)
    {
      return;
    }

    uint8_t dataToWrite[16];
    for (int i = 0; i < 16; ++i)
    {
      dataToWrite[0] = 1;
    }

    if (rfid.MIFARE_Write(6, dataToWrite, 16) != MFRC522::STATUS_OK)
    {
      Serial.println("Write error");
      return;
    }

    Serial.println("Write OK");
    rfid.PICC_HaltA();
  }
}                              