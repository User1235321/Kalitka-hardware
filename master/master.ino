#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

uint32_t rebootTimer;
long mode;
long numOfDoor;

void setup()
{
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
  mode = 0;
  numOfDoor = 0;
}

void loop()
{
  //rfid reboot
  if ((millis() - rebootTimer) >= 1000)
  {
    rebootTimer = millis();
    digitalWrite(RST_PIN, HIGH);
    delayMicroseconds(2);
    digitalWrite(RST_PIN, LOW);
    rfid.PCD_Init();
  }

  mode = (mode == 0) ? Serial.parseInt() : mode;
  if ((mode == 3) || (mode == 2))
  {
    numOfDoor = (numOfDoor == 0) ? Serial.parseInt() : numOfDoor;

    if (numOfDoor == 0) return; //Number of door isn't read
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

    if (mode == 3)
    {
      //reading
      Serial.println(dataBlock[(numOfDoor % 16)]);
      rfid.PICC_HaltA();                              // Завершаем работу с меткой
      rfid.PCD_StopCrypto1();
      mode = 0;
      numOfDoor = 0;
    }
    else if (mode == 2)
    {
      //writing
      uint8_t doorMode = Serial.parseInt();
      dataBlock[numOfDoor % 16] = doorMode;
      if (rfid.MIFARE_Write(blockNum, dataBlock, 16) != MFRC522::STATUS_OK)
      {
        Serial.println("0");
      }
      else
      {
        Serial.println(doorMode + numOfDoor + mode);
        rfid.PICC_HaltA();
        mode = 0;
        numOfDoor = 0;
      }
    }
  }
  else if (mode == 1)
  {
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return; // Can't read cart
    for (uint8_t i = 0; i < 4; i++)
    {
      Serial.print(rfid.uid.uidByte[i], HEX);
    }
    Serial.println();
    mode = 0;
  }
}