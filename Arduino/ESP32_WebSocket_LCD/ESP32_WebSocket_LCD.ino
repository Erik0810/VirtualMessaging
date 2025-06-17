#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "config.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
WebSocketsClient webSocket;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Socket.IO Client Starting");
  
  // Initialize LCD
  Wire.begin(21, 22);
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();
  lcd.print("Connecting...");
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
  
  // Connect to WebSocket
  webSocket.begin(websocket_server, websocket_port, "/socket.io/?EIO=4&transport=websocket");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
  
  lcd.clear();
  lcd.print("Ready");
}

void loop() {
  webSocket.loop();
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  String message = String((char*)payload);
  
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("Disconnected");
      lcd.clear();
      lcd.print("Disconnected");
      break;
      
    case WStype_CONNECTED:
      Serial.println("Connected to server");
      lcd.clear();
      lcd.print("Connected");
      // Send Socket.IO connection message
      webSocket.sendTXT("40");
      break;
      
    case WStype_TEXT:
      Serial.print("Received: ");
      Serial.println(message);
      
      // Handle Socket.IO messages
      if (message.startsWith("2")) {
        // Engine.IO ping - respond with pong
        webSocket.sendTXT("3");
      }
      else if (message.startsWith("40")) {
        // Socket.IO connection confirmed
        Serial.println("Socket.IO connected");
      }
      else if (message.startsWith("42")) {
        // Event message - parse it
        String jsonData = message.substring(2);
        Serial.print("JSON Data: ");
        Serial.println(jsonData);
        
        DynamicJsonDocument doc(512);
        if (deserializeJson(doc, jsonData) == DeserializationError::Ok) {
          if (doc.is<JsonArray>() && doc.size() >= 2) {
            String eventName = doc[0];
            if (eventName == "display_message") {
              String displayText = doc[1]["message"];
              Serial.print("Display: ");
              Serial.println(displayText);
              
              // Update LCD
              lcd.clear();
              if (displayText.length() > 16) {
                lcd.setCursor(0, 0);
                lcd.print(displayText.substring(0, 16));
                lcd.setCursor(0, 1);
                lcd.print(displayText.substring(16, 32));
              } else {
                lcd.print(displayText);
              }
            }
          }
        }
      }
      break;
  }
}