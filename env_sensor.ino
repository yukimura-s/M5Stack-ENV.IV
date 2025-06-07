#include <M5CoreS3.h>
#include <Wire.h>

#define SHT4X_ADDRESS 0x44

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  M5.begin();
  Wire1.begin(2, 1);
  
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(50, 20);
  M5.Lcd.println("Unit-ENV");
  
  Serial.println("温湿度モニター開始");
}

void loop() {
  float temperature, humidity;
  
  if (readSHT4X(temperature, humidity)) {
    Serial.printf("温度: %.2f °C\n", temperature);
    Serial.printf("湿度: %.2f %%RH\n", humidity);
    
    // 画面表示
    M5.Lcd.fillRect(0, 80, 320, 160, BLACK);  // データエリアクリア
    
    // 温度表示
    M5.Lcd.setCursor(10, 90);
    M5.Lcd.setTextSize(4);
    M5.Lcd.setTextColor(CYAN);
    M5.Lcd.printf("%.1fC", temperature);
    
    // 湿度表示
    M5.Lcd.setCursor(10, 140);
    M5.Lcd.setTextSize(4);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.printf("%.0f%%", humidity);
    
    // 更新時刻
    M5.Lcd.setCursor(10, 200);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.printf("Update: %lu sec", millis()/1000);
    
  } else {
    Serial.println("センサー読み取りエラー");
    M5.Lcd.fillRect(0, 80, 320, 160, BLACK);
    M5.Lcd.setCursor(80, 120);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("ERROR!");
  }
  
  delay(3000);  // 3秒間隔で更新
}

bool readSHT4X(float &temperature, float &humidity) {
  Wire1.beginTransmission(SHT4X_ADDRESS);
  Wire1.write(0xFD);  // 高精度測定コマンド
  uint8_t error = Wire1.endTransmission();
  
  if (error != 0) return false;
  
  delay(10);  // 測定待ち
  
  Wire1.requestFrom(SHT4X_ADDRESS, 6);
  if (Wire1.available() < 6) return false;
  
  uint8_t data[6];
  for (int i = 0; i < 6; i++) {
    data[i] = Wire1.read();
  }
  
  // 温度計算
  uint16_t temp_raw = (data[0] << 8) | data[1];
  temperature = -45 + 175 * ((float)temp_raw / 65535.0);
  
  // 湿度計算
  uint16_t hum_raw = (data[3] << 8) | data[4];
  humidity = 100 * ((float)hum_raw / 65535.0);
  
  return true;
}
