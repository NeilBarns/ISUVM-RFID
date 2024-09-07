#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

// Replace with your network credentials
const char* ssid = "YOUR_ISP_SSID";
const char* password = "YOUR_ISP_PASSWORD";

// Replace with your IP
const String inoutAPIEndpoint = "http://YOUR_IP:8000/api/owner/inout/";
const String sendSMSAPIEndpoint = "http://YOUR_IP:8000/api/owner/sendsms/";

#define SS_PIN  D8 
#define RST_PIN D2 
#define BUZZER_PIN D3

MFRC522 mfrc522(SS_PIN, RST_PIN);
void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init(); 
  pinMode(BUZZER_PIN, OUTPUT);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.println("Tap an RFID/NFC tag on the MFRC522 reader");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent()) { 
    if (mfrc522.PICC_ReadCardSerial()) { 
   
      unsigned long uidDec = 0;
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        uidDec += mfrc522.uid.uidByte[i] << (8 * i);
      }
      
      Serial.print("UID: ");
      Serial.println(uidDec);

      String inoutFullUrl = inoutAPIEndpoint + uidDec;
      String sendSMSFullUrl = sendSMSAPIEndpoint + uidDec;
      
      Serial.print("URL: ");
      Serial.println(inoutFullUrl);
      Serial.println(sendSMSFullUrl);

      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        WiFiClient wifiClient;

        http.begin(wifiClient, inoutFullUrl); 
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0) {
          String response = http.getString(); 
          Serial.println(httpResponseCode); 
          Serial.println(response); 

          if (response.indexOf("\"isRegistered\":true") != -1) {
            buzz(100); 
            
            // Send SMS
            http.begin(wifiClient, sendSMSFullUrl);
            httpResponseCode = http.GET();
            if (httpResponseCode > 0) {
              String smsResponse = http.getString(); 
              Serial.println(httpResponseCode); 
              Serial.println(smsResponse); 
            } else {
              Serial.println("Error sending SMS");
              Serial.println(httpResponseCode);
            }
            http.end();
          } else {
            buzz(3000); 
          }
        } else {
          Serial.print("Error on sending request");
          Serial.println(httpResponseCode);
        }

        http.end();
      }
      
      mfrc522.PICC_HaltA(); 
      mfrc522.PCD_StopCrypto1();
    }
  }
}

void buzz(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration); 
  digitalWrite(BUZZER_PIN, LOW); 
}
