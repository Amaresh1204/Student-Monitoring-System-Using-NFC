#include <Wire.h>
#include <Adafruit_PN532.h>
#include <SoftwareSerial.h>

#define SDA_PIN 21
#define SCL_PIN 22
#define RX_PIN 10
#define TX_PIN 11

SoftwareSerial sim900a(RX_PIN, TX_PIN);

Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);

struct NFCEntry {
  uint8_t uid[4];
  const char* name;
  const char* phoneNumber;
};

NFCEntry nfcEntries[] = {
  {{0x3, 0x32, 0xCE, 0x5B}, "Amaresh", "+919919914894"},
  {{0x33, 0x92, 0xAD, 0x4F}, "Srujan", "+919919914894"},
  {{0x4F 0xED 0x27 0x25}, "Srujan", "+919919914894"},
  // Add more entries as needed
};

size_t MAX_ENTRIES = sizeof(nfcEntries) / sizeof(nfcEntries[0]);

void setup(void) {
  Serial.begin(115200);
  sim900a.begin(9600);

  Serial.println("Hello!");

  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("Didn't find PN53x board");
    while (1);
  }
  nfc.SAMConfig();
  Serial.println("Waiting for an NFC card...");
}

void sendSMS(const char* phoneNumber, const char* message) {
  sim900a.println("AT+CMGF=1\r");
  delay(100);
  sim900a.print("AT+CMGS=\"");
  sim900a.print(phoneNumber);
  sim900a.print("\"\r");
  delay(100);
  sim900a.print(message);
  sim900a.write(26);
  delay(1000);

  while (sim900a.available()) {
    Serial.write(sim900a.read());
  }
  Serial.println("SMS sent!");
}

void loop(void) {
  uint8_t success;
  uint8_t uid[] = {0, 0, 0, 0};
  uint8_t uidLength;

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    Serial.println("Found an NFC card!");

    Serial.print("UID Length: ");
    Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.print(" 0x");
      Serial.print(uid[i], HEX);
    }
    Serial.println("");

    const char* foundName = NULL;
    const char* foundPhoneNumber = NULL;
    for (size_t i = 0; i < MAX_ENTRIES; i++) {
      if (memcmp(uid, nfcEntries[i].uid, uidLength) == 0) {
        foundName = nfcEntries[i].name;
        foundPhoneNumber = nfcEntries[i].phoneNumber;
        break;
      }
    }

    if (foundName != NULL && strlen(foundName) > 0) {
      Serial.print("Name: ");
      Serial.println(foundName);
      sendSMS(foundPhoneNumber, foundName);
    } else {
      Serial.println("Unknown card");
    }

    delay(1000);
  }
}
