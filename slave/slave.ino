#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10
#define numOfDoor 200

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

uint32_t rebootTimer;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);
  rfid.PCD_AntennaOff();
  rfid.PCD_AntennaOn();

  for (byte i = 0; i < 6; ++i)
  {
    key.keyByte[i] = 0xFF;
  }

  rebootTimer = millis();
}

void loop() {
  if ((millis() - rebootTimer) >= 1000)
  {
    rebootTimer = millis();

    digitalWrite(RST_PIN, HIGH);
    delayMicroseconds(2);
    digitalWrite(RST_PIN, LOW);
    rfid.PCD_Init();
  }

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return; // Can't read cart
    long blockNum = (1 + numOfDoor / 16) + ((1 + numOfDoor / 16) / 3);
    if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, ((((blockNum / 4) + 1) * 4) - 1), &key, &(rfid.uid))
      != MFRC522::STATUS_OK)
    {
      Serial.println("Key error!");
      return;
    }

    uint8_t dataBlock[18];
    uint8_t size = sizeof(dataBlock);
    if (rfid.MIFARE_Read(blockNum, dataBlock, &size) != MFRC522::STATUS_OK)
    {
      Serial.println("Read error");
      return;
    }

    if (dataBlock[(numOfDoor % 16)] == 1)
    {
      Serial.println("Door open");
    }
    else
    {
      Serial.println("Door close");
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}
