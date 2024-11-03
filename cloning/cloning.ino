#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 22   
#define SS_PIN 5     

MFRC522 mfrc522(SS_PIN, RST_PIN);

byte buffer[18];
byte block;
byte allblocks[64][16];
MFRC522::StatusCode status;
    
MFRC522::MIFARE_Key key;
char choice;

void setup() {
    Serial.begin(9600);         
    while (!Serial);            
    SPI.begin();              
    mfrc522.PCD_Init();         
    Serial.println("1.Read card \n2.Copy the data.");

    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }
}

// read the bytes in hexadecimal format
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

// convert the data to ascii
void dump_byte_array1(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.write(buffer[i]);
  }
}

bool read_blocks(MFRC522::MIFARE_Key *key)
{
    bool result = false;
    
    for(byte block = 0; block < 64; block++){
      
    // authenticate using key A
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    // Read block
    byte byteCount = sizeof(buffer);
    status = mfrc522.MIFARE_Read(block, buffer, &byteCount);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    else {
        // Successful read
        result = true;
        
        // Dump block data
        Serial.print(F("Block ")); Serial.print(block); Serial.print(F(":"));
        dump_byte_array1(buffer, 16); // Convert from hex to ASCII
        Serial.println();
        
        for (int p = 0; p < 16; p++) // Read the 16 bytes from the block
        {
          allblocks [block][p] = buffer[p];
          Serial.print(allblocks[block][p]);
          Serial.print(" ");
        }
        
        }
    }
    Serial.println();
    
    Serial.println("1.Read card \n2.Copy the data.");

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return result;
    
    start();
}

void loop() {
  start();
}

void start(){
  choice = Serial.read();
  
  if(choice == '1')
  {
    Serial.println("reading the card");
    read();
      
    }
    else if(choice == '2')
    {
      Serial.println("copying data read to new card");
      clone();
    }
}

void clone(){
Serial.println("insert new card");
    if ( ! mfrc522.PICC_IsNewCardPresent())
        return;

    if ( ! mfrc522.PICC_ReadCardSerial())
        return;

    // Show some details of the PICC (that is: the tag/card)
    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();
    
    // use default key
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }

    // copy the blocks excluding those that are unwritable
    for(int i = 4; i <= 62; i++) { 
        if((i + 1) % 4 == 0){
          i++;
        }
        block = i;
        
        // use key a to authenticate
        Serial.println(F("Authenticating using key A..."));
        status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
        if (status != MFRC522::STATUS_OK) {
            Serial.print(F("PCD_Authenticate() failed: "));
            Serial.println(mfrc522.GetStatusCodeName(status));
            return;
        }
        
        // Write data to the block
        Serial.print(F("Writing data into block "));
        Serial.print(block);
        Serial.println("\n");
              
        dump_byte_array(allblocks[block], 16); 
        
        status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(block, allblocks[block], 16);
        if (status != MFRC522::STATUS_OK) {
            Serial.print(F("MIFARE_Write() failed: "));
            Serial.println(mfrc522.GetStatusCodeName(status));
        }
        
        Serial.println("\n");
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1(); 
    
    Serial.println("1. Read card \n2.Copy the data.");
    start();
}

void read() { 
    Serial.println("Insert card...");

    if (!mfrc522.PICC_IsNewCardPresent())
        return;

    if (!mfrc522.PICC_ReadCardSerial())
        return;

    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();

    read_blocks(&key);
}

