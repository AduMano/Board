// fingerprint_scanner.cpp
// Implementation file for handling fingerprint scanner on Arduino Uno R3

#include "fingerprint_scanner.h"

FingerprintScanner::FingerprintScanner()
    : mySerial(FINGERPRINT_TX, FINGERPRINT_RX), finger(&mySerial) {}

void FingerprintScanner::begin()
{
  Serial.begin(115200);
  mySerial.begin(57600);

  Serial.println("Initializing fingerprint scanner...");
  finger.begin(57600);

  if (finger.verifyPassword())
  {
    Serial.println("Fingerprint scanner found!");
  }
  else
  {
    Serial.println("Fingerprint scanner NOT found. Check wiring.");
    while (1)
      ;
  }
}

bool FingerprintScanner::captureFingerprint()
{
  Serial.println("Place your finger on the scanner...");
  delay(1000);

  if (finger.getImage() != FINGERPRINT_OK)
  {
    Serial.println("No valid fingerprint detected.");
    return false;
  }

  Serial.println("Fingerprint image taken!");
  if (finger.image2Tz(1) != FINGERPRINT_OK)
  {
    Serial.println("Error converting image to template.");
    return false;
  }

  Serial.println("Fingerprint converted to template.");
  if (finger.storeModel(1) == FINGERPRINT_OK)
  {
    Serial.println("Fingerprint template stored successfully!");
    return true;
  }

  Serial.println("Error storing template.");
  return false;
}

void FingerprintScanner::sendFingerprintData()
{
  Serial.println("Retrieving stored fingerprint template...");
  if (finger.loadModel(1) != FINGERPRINT_OK)
  {
    Serial.println("Error loading fingerprint template.");
    return;
  }

  if (finger.getModel() != FINGERPRINT_OK)
  {
    Serial.println("Error retrieving model data.");
    return;
  }

  uint8_t packet[2] = {static_cast<uint8_t>(finger.fingerID & 0xFF), static_cast<uint8_t>((finger.fingerID >> 8) & 0xFF)};
  Serial.write(packet, sizeof(packet));
  Serial.println("Fingerprint data sent.");
}
