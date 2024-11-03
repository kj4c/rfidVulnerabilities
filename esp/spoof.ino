#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define SS_PIN 5
#define RST_PIN 22

MFRC522 rfid(SS_PIN, RST_PIN);

const char* ssid = "Low Family 5G";
const char* password = "lowisthenewhigh";
const char* serverName = "http://192.168.0.243:5000/handle_rfid";

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  
  Serial.println("Connected to WiFi.");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  // read data from block 4
  String payload = readDataBlock(4);

  if (payload == "-1") {
    Serial.println("read failed");
    delay(1000);
    return;
  }

  Serial.print("sendin dat payload: ");
  Serial.println(payload);
  sendToServer(payload);
  
  delay(5000); 
}

String readDataBlock(byte blockNumber) {
    byte buffer[18];
    byte size = sizeof(buffer);

    // default key for Key A
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
    }

    // authentication for key A
    MFRC522::StatusCode status = rfid.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNumber, &key, &(rfid.uid)
    );

    if (status != MFRC522::STATUS_OK) {
        Serial.print("Authentication failed: ");
        Serial.println(rfid.GetStatusCodeName(status));
        return "-1";
    }

    // read dat boy
    status = rfid.MIFARE_Read(blockNumber, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Read failed: ");
        Serial.println(rfid.GetStatusCodeName(status));
        return "-1";
    }

    String data = "";
    for (byte i = 0; i < 16; i++) {
      if (buffer[i] != '\0') {
        data += (char)buffer[i];
      }
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return data;
}


void sendToServer(String data) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "username=" + data;

    // Send the POST request
    int httpResponseCode = http.POST(postData);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response received: " + response);
    } else {
      Serial.print("Error on sending POST request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}
