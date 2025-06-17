#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "config.h"

// WebSocket path for Socket.IO with namespace (version 4)
const char* websocket_path = "/socket.io/?EIO=4&transport=websocket";

// Debug flag
#define DEBUG_WEBSOCKET true

// LCD setup (address and size needs to be adjusted if you are using a different LCD)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // (address, columns, rows)
WebSocketsClient webSocket;

void setup() {
  // Init Serial for debugging
  Serial.begin(115200);
  
  // Init LCD
  Wire.begin(21, 22);  // SDA = 21, SCL = 22
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
  Serial.println("\nConnected to WiFi");
  
  // Display IP on LCD
  lcd.clear();
  lcd.print("WiFi Connected");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(2000);
  
  // Configure WebSocket with ping interval
  Serial.print("Connecting to server: ");
  Serial.println(websocket_server);
  webSocket.setReconnectInterval(5000);
  webSocket.enableHeartbeat(25000, 20000, 2);
  webSocket.begin(websocket_server, websocket_port, websocket_path);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("Disconnected from WebSocket server");
      lcd.clear();
      lcd.print("Disconnected");
      break;
      
    case WStype_CONNECTED:
      Serial.println("Connected to WebSocket server");
      lcd.clear();
      lcd.print("Connected");
      break;
      
    case WStype_TEXT: {
      String rawMessage = String((char*)payload);
      Serial.println("\n--- New Message ---");
      Serial.print("Raw message length: ");
      Serial.println(length);
      Serial.print("Raw message: ");
      Serial.println(rawMessage);
      
      // Print first few bytes for debugging
      Serial.print("First bytes: ");
      for(size_t i = 0; i < min(length, (size_t)10); i++) {
        Serial.print((char)payload[i]);
      }
      Serial.println();
      
      if (rawMessage.startsWith("0")) {
        Serial.println("Type: Socket.IO handshake");
      }
      else if (rawMessage.startsWith("2")) {
        Serial.println("Type: Socket.IO ping/pong");
        // Send pong response
        webSocket.sendTXT("3");
      }
      else if (rawMessage.startsWith("42")) {
        Serial.println("Type: Socket.IO event message");
        String messageContent = rawMessage.substring(2);
        Serial.print("Message content: ");
        Serial.println(messageContent);
        
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, messageContent);
        
        if (!error) {
          const char* eventName = doc[0];
          Serial.print("Event name: ");
          Serial.println(eventName);
          
          if (strcmp(eventName, "display_message") == 0) {
            Serial.println("Found display_message event");
            Serial.println("Parsing message object...");
            JsonVariant messageObj = doc[1];
            if (messageObj.containsKey("message")) {
              const char* displayText = messageObj["message"];
              Serial.print("Message to display: ");
              Serial.println(displayText);
              updateDisplay(displayText);
            } else {
              Serial.println("No message field found in event data");
            }
          }
        } else {
          Serial.print("JSON parsing failed: ");
          Serial.println(error.c_str());
        }
      }
      else {
        Serial.println("Type: Unknown message format");
      }
      Serial.println("------------------");
      break;
    }
  }
}

void updateDisplay(const char* text) {
  if (!text) {
    Serial.println("Error: Null message received");
    return;
  }
  
  Serial.println("Updating LCD display:");
  Serial.print("Message: ");
  Serial.println(text);
  
  lcd.clear();
  if (strlen(text) > 16) {
    // First line
    String firstLine = String(text).substring(0, 16);
    Serial.print("Line 1: ");
    Serial.println(firstLine);
    lcd.setCursor(0, 0);
    lcd.print(firstLine);
    
    // Second line if message is longer
    if (strlen(text) > 16) {
      String secondLine = String(text).substring(16, 32);
      Serial.print("Line 2: ");
      Serial.println(secondLine);
      lcd.setCursor(0, 1);
      lcd.print(secondLine);
    }
  } else {
    Serial.println("Single line message");
    lcd.print(text);
  }
  
  Serial.println("Display update complete");
}