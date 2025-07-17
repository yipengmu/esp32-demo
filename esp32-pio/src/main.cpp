#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

// WiFi 配置
const char* ssid = "tatahome";
const char* password = "m1234567";

// 天气API配置
const char* apiURL = "https://nu33jntuuc.re.qweatherapi.com/v7/weather/now?location=101210101&key=d48d7aaeea044c23b127a8661c74275d";
const char* apiKey = "d48d7aaeea044c23b127a8661c74275d";

// 核心函数声明
bool connectWiFi();
void getWeatherData();

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("ESP32 天气API测试程序启动...");
  Serial.println("=============================");

  // 核心功能1：连接WiFi
  if (connectWiFi()) {
    Serial.println("WiFi连接成功！");

    // 核心功能2：获取天气数据
    getWeatherData();
  } else {
    Serial.println("WiFi连接失败！");
  }
}

void loop() {
  // 每30秒获取一次天气数据
  delay(8000);

  if (WiFi.status() == WL_CONNECTED) {
    getWeatherData();
  } else {
    Serial.println("WiFi连接断开，尝试重新连接...");
    connectWiFi();
  }
}

// 核心功能1：WiFi连接函数
bool connectWiFi() {
  Serial.println("开始连接WiFi...");
  Serial.print("网络名称: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int attempts = 0;
  const int maxAttempts = 20;

  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi连接成功！");
    Serial.print("IP地址: ");
    Serial.println(WiFi.localIP());
    Serial.print("信号强度: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    return true;
  } else {
    Serial.println("WiFi连接失败！");
    return false;
  }
}

// 核心功能2：获取天气数据函数
void getWeatherData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi未连接，无法获取天气数据");
    return;
  }

  Serial.println();
  Serial.println("正在获取天气数据...");
  Serial.print("API地址: ");
  Serial.println(apiURL);

  HTTPClient http;
  http.begin(apiURL);

  // 设置请求头
  http.addHeader("User-Agent", "ESP32-Weather-Client/1.0");
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");

  Serial.println("发送HTTP GET请求...");
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    Serial.print("HTTP响应代码: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200) {
      String payload = http.getString();

      Serial.println();
      Serial.println("========== 天气API返回的JSON数据 ==========");
      Serial.println(payload);
      Serial.println("==========================================");

    } else {
      Serial.print("HTTP请求失败，响应代码: ");
      Serial.println(httpResponseCode);
    }
  } else {
    Serial.print("HTTP请求错误: ");
    Serial.println(http.errorToString(httpResponseCode));
  }

  http.end();
  Serial.println("天气数据获取完成");
  Serial.println();
}

