#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H

#include <ESP8266WiFi.h>
#include <lcd_manager.h>
#include <buzzer_manager.h>

BuzzerManager buzzer(D0);

const char *ssid = "Adu";
const char *password = "aldospot";

void connectToWiFi()
{
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);

  Serial.print("Connecting to WiFi");
  displayMessage(" Connecting  to ", "    WiFi....    ");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED)
  {
    if (millis() - startAttemptTime >= 35000)
    {
      buzzer.beepError();
      Serial.println("\n[ERROR] WiFi Connection Failed! Retrying in 30 seconds...");
      displayMessage("  WiFi  Failed  ", "  Restarting... ");
      delay(2500);

      ESP.restart();
    }

    delay(1000);
    Serial.print(".");
    buzzer.beepSuccess();

    yield();
  }

  Serial.println("\n[INFO] WiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  buzzer.beepSuccess();
  displayMessage("  Successfully  ", "   Connected!   ");
  delay(2500);
}

// Function to monitor WiFi connection & reconnect if needed
void checkWiFiConnection()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    buzzer.beepError();

    Serial.println("\n[WARNING] WiFi Disconnected! Reconnecting...");
    displayMessage("WiFi signal lost", "Reconnecting... ");
    WiFi.disconnect();

    delay(2000);
    connectToWiFi();
  }
}

#endif
