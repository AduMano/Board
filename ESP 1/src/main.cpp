// Modern WebSocket Client for ESP8266 NodeMCU (Client-side)
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include "wifi_setup.h"
#include "lcd_manager.h"
#include "buzzer_manager.h"
#include "supabase_fetch.h"

SupabaseFetcher supabase;


void setup() {
    Serial.begin(74880);
    
    connectToWiFi();

    // ✅ Fetch the server URL from Supabase
    supabase.fetchServerUrl();
}

void loop() {
     // ✅ Poll Supabase for locker updates
    int lockerIds[] = {1, 2, 3};  // List of lockers to check
    
    supabase.checkForUpdates(lockerIds);
    supabase.checkForState();
    
    delay(3000);  // Poll every 5 seconds
}