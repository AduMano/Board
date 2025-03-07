// fingerprint_scanner.h
// Header file for handling fingerprint scanner on Arduino Uno R3

#ifndef FINGERPRINT_SCANNER_H
#define FINGERPRINT_SCANNER_H

#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

#define FINGERPRINT_RX 2 // Fingerprint Scanner TX → Arduino D2
#define FINGERPRINT_TX 3 // Fingerprint Scanner RX → Arduino D3

class FingerprintScanner
{
public:
  FingerprintScanner();
  void begin();
  bool captureFingerprint();
  void sendFingerprintData();

private:
  SoftwareSerial mySerial;
  Adafruit_Fingerprint finger;
  uint8_t templateData[512];
  uint16_t templateLength;
};

#endif
