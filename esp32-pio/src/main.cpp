#include <Arduino.h>
#include <WiFi.h>

// WiFi credentials
const char* ssid = "tatahome";
const char* password = "m1234567";

void setup() {
  Serial.begin(115200);
  delay(3000);  // 增加延迟

  Serial.println();
  Serial.println("ESP32 WiFi Test Starting...");
  Serial.println("===========================");
  Serial.flush();

  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  Serial.flush();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  Serial.flush();

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000);
    Serial.print(".");
    Serial.flush();
    attempts++;

    if (attempts % 5 == 0) {
      Serial.println();
      Serial.print("Status: ");
      Serial.println(WiFi.status());
      Serial.print("Attempt ");
      Serial.print(attempts);
      Serial.print("/20");
      Serial.flush();
    }
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi connection failed!");
    Serial.print("Final status: ");
    Serial.println(WiFi.status());
  }
  Serial.flush();
}

void loop() {
  Serial.println("Loop running...");
  Serial.flush();
  delay(5000);
}
