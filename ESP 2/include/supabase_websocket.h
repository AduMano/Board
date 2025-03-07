// #ifndef SUPABASE_WEBSOCKET_H
// #define SUPABASE_WEBSOCKET_H

// #include <ESP8266WiFi.h>
// #include <wifi_setup.h>
// #include <ArduinoJson.h> // for handleUpdate JSON parse if needed
// #include <ArduinoWebsockets.h>

// const char *sssid = "Adu";
// const char *passsword = "aldospot";

// using namespace websockets;

// // The global WebSocketsClient instance
// WebsocketsClient wsClient;

// static const char ROOT_CERT[] PROGMEM = R"EOF(
// -----BEGIN CERTIFICATE-----
// MIIDpjCCA0ugAwIBAgIRAO5Fh/8Iqw0JEUcwTfjY/i8wCgYIKoZIzj0EAwIwOzEL
// MAkGA1UEBhMCVVMxHjAcBgNVBAoTFUdvb2dsZSBUcnVzdCBTZXJ2aWNlczEMMAoG
// A1UEAxMDV0UxMB4XDTI1MDExMzAxMjgwMloXDTI1MDQxMzAyMjc0NlowFjEUMBIG
// A1UEAxMLc3VwYWJhc2UuY28wWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAAQyqdY7
// SWd3d9aWXbeOehpaBPUzrJFo4U6PjUk+BjmioBpyJ/QLzRTyPbyei+cdGKN1kb/Q
// gMHcv/jSFAytFW2ro4ICUzCCAk8wDgYDVR0PAQH/BAQDAgeAMBMGA1UdJQQMMAoG
// CCsGAQUFBwMBMAwGA1UdEwEB/wQCMAAwHQYDVR0OBBYEFBXV8ZueMZ1e4ZljSIAS
// mHfUgEMNMB8GA1UdIwQYMBaAFJB3kjVnxP+ozKnme9mAeXvMk/k4MF4GCCsGAQUF
// BwEBBFIwUDAnBggrBgEFBQcwAYYbaHR0cDovL28ucGtpLmdvb2cvcy93ZTEvN2tV
// MCUGCCsGAQUFBzAChhlodHRwOi8vaS5wa2kuZ29vZy93ZTEuY3J0MCUGA1UdEQQe
// MByCC3N1cGFiYXNlLmNvgg0qLnN1cGFiYXNlLmNvMBMGA1UdIAQMMAowCAYGZ4EM
// AQIBMDYGA1UdHwQvMC0wK6ApoCeGJWh0dHA6Ly9jLnBraS5nb29nL3dlMS9UdE9U
// VHJDOHZSUS5jcmwwggEEBgorBgEEAdZ5AgQCBIH1BIHyAPAAdgDPEVbu1S58r/OH
// W9lpLpvpGnFnSrAX7KwB0lt3zsw7CAAAAZRdfVZBAAAEAwBHMEUCIQDk9SN8OoT5
// SQGhQmw4CGi5d0MRsyQ3I30BLby+cpSo9AIgBMGjHPCUnAMvM/bJQYkj//G6sftb
// xwZ/A43J1Qnlw9YAdgDm0jFjQHeMwRBBBtdxuc7B0kD2loSG+7qHMh39HjeOUAAA
// AZRdfVYbAAAEAwBHMEUCIQCmW2XnBOi2B1B4WOFVCq6nkDCtpnnZ2nG0K/lq+fZq
// gQIgALFuMNTUHpbjEoBtD2YQiXgdCLJlNzjEVhAJlyYktp4wCgYIKoZIzj0EAwID
// SQAwRgIhAO+T0GngMAL8j1hi3WOhnM8GKclfM7HBLEY65kNWlgctAiEAsmqGvk8y
// ENlfeHA9h5mpTebxdbBMSClP3RwUvgkzYxQ=
// -----END CERTIFICATE-----
// )EOF";

// // Called when we receive a TEXT message (case WStype_TEXT)
// void handleUpdate(uint8_t *payload)
// {
//   Serial.println("Update detected!");
//   String updateMessage = "Update received: ";
//   updateMessage += (char *)payload;
//   Serial.println(updateMessage);

//   StaticJsonDocument<256> doc;
//   DeserializationError error = deserializeJson(doc, payload);
//   if (!error)
//   {
//     // Access doc["key"] etc. if needed
//   }
// }

// // The revised setup function – uses SSL (port 443) & Bearer headers
// void realtime_setup(const char *supabaseUrl, uint16_t port, const char *path, const char *supabaseKey)
// {
//   Serial.print("Starting WebSocket connection to ");
//   Serial.print(supabaseUrl);
//   Serial.println(path);

//   // (B) Set Bearer token & apikey for Supabase Realtime
//   wsClient.addHeader("Authorization", String("Bearer ") + String(supabaseKey));
//   wsClient.addHeader("apikey", String(supabaseKey));
//   wsClient.addHeader("Accept", "*/*");
//   wsClient.addHeader("Content-Type", "application/json");

//   // (C) Tell the client to ignore certificate checks (insecure, for testing)
//   //     In your final code, you'd want to properly verify the certificate:
//   // Instead of setInsecure(), do:
//   // wsClient.set(ROOT_CERT);

//   // (D) Build the wss:// URL for Realtime
//   String realtimeUrl = String("wss://") + String(supabaseUrl) + "/realtime/v1";

//   // Now do connect. For Gil Maimon’s library:
//   bool connected = wsClient.connectSecure(String(supabaseUrl), port, String(path));
//   if (!connected)
//   {
//     Serial.println("Could NOT connect to Supabase Realtime via SSL...");
//     return;
//   }
//   Serial.println("Connected to Supabase Realtime with CA!");

//   // (F) Register callbacks
//   //    - onMessage: called when new data arrives
//   //    - onEvent: called when connection closes, pings, etc.
//   wsClient.onMessage([](WebsocketsClient &client, WebsocketsMessage message)
//                      {
//                        Serial.println("[Realtime] Got Message: " + message.data());
//                        // Here you could parse JSON, handle updates, etc.
//                      });

//   wsClient.onEvent([](WebsocketsClient &client, WebsocketsEvent event, String data)
//                    {
//     if (event == WebsocketsEvent::ConnectionOpened) {
//       Serial.println("[Realtime] ConnectionOpened event");
//       // (G) After connecting, send a subscription message
//       // If your table is `Table_Status` in the `public` schema:
//       String subscribeMessage = R"({
//         "topic": "realtime:public:Table_Status",
//         "event": "phx_join",
//         "payload": {},
//         "ref": "1"
//       })";
//       client.send(subscribeMessage);
//     } else if (event == WebsocketsEvent::ConnectionClosed) {
//       Serial.println("[Realtime] ConnectionClosed event");
//     } else if (event == WebsocketsEvent::GotPing) {
//       Serial.println("[Realtime] Got a Ping!");
//     } else if (event == WebsocketsEvent::GotPong) {
//       Serial.println("[Realtime] Got a Pong!");
//     } });
// }

// // The loop function to be called in main.cpp
// void realtime_loop()
// {
//   wsClient.poll();
//   delay(10); // small delay to avoid watchdog resets, etc.

//   yield(); // Let WiFi/other tasks run
// }

// // The event callback where we handle connect/disconnect/messages
// // void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
// // {
// //   Serial.println("[REALTIME] Events");
// //   switch (type)
// //   {
// //   case WStype_DISCONNECTED:
// //     Serial.println("Disconnected from WebSocket");
// //     // Optional: Attempt reconnect
// //     // webSocket.beginSSL(...);
// //     break;

// //   case WStype_CONNECTED:
// //     Serial.println("Connected to WebSocket");

// //     // 4) Send your subscription message after connecting
// //     //    Adjust your table name if needed: "realtime:public:Table_Status"
// //     {
// //       String subscribeMessage = R"RAW(
// //         {
// //           "topic": "realtime:public:Table_Status",
// //           "event": "phx_join",
// //           "payload": {},
// //           "ref": "1"
// //         }
// //         )RAW";
// //       webSocket.sendTXT(subscribeMessage);
// //     }
// //     break;

// //   case WStype_TEXT:
// //     Serial.printf("Received message: %s\n", payload);
// //     handleUpdate(payload);
// //     break;

// //   case WStype_ERROR:
// //     Serial.println("Error in WebSocket");
// //     // Possibly attempt to reconnect here as well
// //     break;

// //   default:
// //     break;
// //   }
// // }

// #endif // SUPABASE_WEBSOCKET_H
