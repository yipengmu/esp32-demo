#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <zlib_turbo.h>  // 引入zlib_turbo库

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

  // 打印内存使用情况
  Serial.printf("开始获取天气数据 - 可用内存: %d 字节\n", ESP.getFreeHeap());
  Serial.println();
  Serial.println("正在获取天气数据...");
  Serial.print("API地址: ");
  Serial.println(apiURL);

  HTTPClient http;
  http.begin(apiURL);

  // 设置请求头 - 暂时禁用gzip以避免堆栈问题
  http.addHeader("User-Agent", "ESP32-Weather-Client/1.0");
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept-Encoding", "identity"); // 只接受未压缩数据

  Serial.println("发送HTTP GET请求...");
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    Serial.print("HTTP响应代码: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200) {
      Serial.println("HTTP请求成功，获取响应数据...");

      // 获取响应流
      WiFiClient *stream = http.getStreamPtr();
      int contentLength = http.getSize();

      Serial.printf("内容长度: %d 字节\n", contentLength);

      String payload = "";

      if (contentLength > 0 && contentLength < 4096) {
        // 读取前几个字节检查是否为gzip
        uint8_t header[10];
        int headerRead = 0;

        // 读取头部数据
        while (headerRead < 10 && stream->available()) {
          header[headerRead++] = stream->read();
        }

        // 打印头部字节用于调试
        Serial.print("头部字节: ");
        for (int i = 0; i < headerRead; i++) {
          Serial.printf("0x%02X ", header[i]);
        }
        Serial.println();

        // 检查gzip魔数 (0x1f, 0x8b)
        if (headerRead >= 2 && header[0] == 0x1f && header[1] == 0x8b) {
          Serial.println("检测到gzip压缩数据！");
          Serial.println("服务器忽略了Accept-Encoding头，强制返回压缩数据");

          // 分配缓冲区读取完整数据
          uint8_t *buffer = (uint8_t*)malloc(contentLength + 16);
          if (buffer) {
            // 复制已读取的头部
            memcpy(buffer, header, headerRead);

            // 读取剩余数据
            int totalRead = headerRead;
            while (totalRead < contentLength && stream->available()) {
              int bytesToRead = min(contentLength - totalRead, stream->available());
              int actualRead = stream->readBytes(buffer + totalRead, bytesToRead);
              totalRead += actualRead;
            }

            Serial.printf("读取完成，总共 %d 字节\n", totalRead);

            // 使用zlib_turbo解压
            static zlib_turbo zt;
            uint32_t uncompSize = zt.gzip_info(buffer, totalRead);

            if (uncompSize > 0 && uncompSize < 8192) {
              uint8_t *uncompressed = (uint8_t*)malloc(uncompSize + 16);
              if (uncompressed) {
                int result = zt.gunzip(buffer, totalRead, uncompressed);
                if (result == ZT_SUCCESS) {
                  uncompressed[uncompSize] = 0;
                  payload = String((char*)uncompressed);
                  Serial.println("gzip解压成功！");
                } else {
                  Serial.printf("解压失败，错误代码: %d\n", result);
                  payload = "解压失败";
                }
                free(uncompressed);
              } else {
                payload = "内存不足";
              }
            } else {
              Serial.printf("解压大小异常: %d\n", uncompSize);
              payload = "数据异常";
            }

            free(buffer);
          } else {
            payload = "内存分配失败";
          }
        } else {
          Serial.println("数据未压缩");
          // 重新构建完整响应
          payload = String((char*)header, headerRead);
          payload += http.getString(); // 读取剩余部分
        }
      } else {
        // 备选方案
        payload = http.getString();
        Serial.println("使用getString()备选方案");
      }

      Serial.printf("最终数据长度: %d 字节\n", payload.length());

      // 打印最终的JSON数据
      Serial.println();
      Serial.println("========== 天气API返回的JSON数据 ==========");
      Serial.println(payload);
      Serial.println("==========================================");
    }else {
      Serial.print("HTTP请求失败，响应代码: ");
      Serial.println(httpResponseCode);
    }
  } else {
    Serial.print("HTTP请求错误: ");
    Serial.println(http.errorToString(httpResponseCode));
  }

  http.end();
  Serial.println("天气数据获取完成");
  Serial.printf("结束获取天气数据 - 可用内存: %d 字节\n", ESP.getFreeHeap());
  Serial.println();
}

