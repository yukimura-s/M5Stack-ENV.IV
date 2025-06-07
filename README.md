# M5Stack CoreS3 温湿度センサープロジェクト完全ガイド

本ガイドでは、M5Stack CoreS3とUnit-ENVセンサーを使用して温湿度モニターを作成する手順を説明します。

## 必要な機材

- M5Stack CoreS3開発ボード
- M5Stack Unit-ENV温湿度センサー (U001-D)
- Grove接続ケーブル（Unit-ENVに付属）
- USB Type-Cケーブル
- Windows PC

## 開発環境のセットアップ

### 1. Arduino IDEのインストール

1. [Arduino公式サイト](https://www.arduino.cc/en/software)からArduino IDEをダウンロード
2. インストーラーを実行してデフォルト設定でインストール

### 2. M5Stackボードパッケージの追加

1. Arduino IDEを起動
2. `ファイル` → `環境設定`を開く
3. 追加のボードマネージャーのURLに以下を追加：
   ```
   https://static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json
   ```
4. `OK`をクリック

### 3. M5Stackボードのインストール

1. `ツール` → `ボード` → `ボードマネージャー`を開く
2. 検索欄に「M5Stack」と入力
3. 「M5Stack by M5Stack」を見つけてインストール

### 4. M5CoreS3ライブラリのインストール

1. `ツール` → `ライブラリを管理`を開く
2. 検索欄に「M5CoreS3」と入力
3. 「M5CoreS3」ライブラリをインストール

### 5. ボード設定

1. `ツール` → `ボード` → `M5Stack-CoreS3`を選択
2. 以下の設定を確認：
   - Flash Mode: QIO
   - Flash Size: 16MB (128Mb)
   - Partition Scheme: Default 4MB with spiffs

## ハードウェア接続

### Unit-ENVセンサーの接続

1. CoreS3の電源を切る
2. Unit-ENVセンサーを付属のGroveケーブルでCoreS3の**赤いGroveポート**（I2C用）に接続
3. ケーブルがしっかり挿入されていることを確認

### USB接続

1. USB Type-CケーブルでCoreS3をPCに接続
2. ドライバーが自動的にインストールされることを確認
3. `ツール` → `シリアルポート`でCOMポートを選択

## センサー動作確認

まず、センサーが正しく接続されているかを確認します。

### I2Cスキャンコード

```cpp
#include <M5CoreS3.h>
#include <Wire.h>

TwoWire Wire1 = TwoWire(1);

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  M5.begin();
  Wire1.begin(2, 1);  // SDA: GPIO2, SCL: GPIO1
  
  Serial.println("I2Cスキャン開始");
}

void loop() {
  Serial.println("Grove I2Cスキャン中...");
  
  int deviceCount = 0;
  for(int address = 1; address < 127; address++) {
    Wire1.beginTransmission(address);
    uint8_t error = Wire1.endTransmission();
    
    if (error == 0) {
      Serial.printf("デバイス発見: 0x%02X", address);
      if (address == 0x44) {
        Serial.println(" (SHT4x温湿度センサー)");
      } else if (address == 0x76) {
        Serial.println(" (BMP280気圧センサー)");
      } else {
        Serial.println(" (不明)");
      }
      deviceCount++;
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("デバイスが見つかりません - 配線を確認してください");
  } else {
    Serial.printf("合計 %d 個のデバイスが見つかりました\n", deviceCount);
  }
  
  delay(5000);
}
```

### 期待される結果

シリアルモニタに以下が表示されれば接続成功：
```
デバイス発見: 0x44 (SHT4x温湿度センサー)
デバイス発見: 0x76 (BMP280気圧センサー)
合計 2 個のデバイスが見つかりました
```

## 基本的な温湿度取得

センサーが認識されたら、温湿度データを取得してみます。

### 温湿度読み取りコード

```cpp
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
```

## プログラムのアップロード

**重要**: 新しいコードをアップロードする際は、毎回以下の手順を実行してください。

1. **ダウンロードモードに入る**：CoreS3のリセットボタンを3秒間長押しして緑色LEDが点灯するまで待つ
2. Arduino IDEで`ツール` → `シリアルポート`からCOMポートを選択
3. 左上の矢印ボタン（アップロード）をクリック
4. コンパイルとアップロードが完了するまで待つ
5. アップロード完了後、CoreS3のリセットボタンを短押ししてプログラムを開始

**注意**: コードを変更してアップロードする度に、手順1のダウンロードモード操作が必要です。

## 動作確認

正常に動作すると以下が確認できます：

1. **シリアルモニタ**：温度と湿度が3秒間隔で表示される
2. **画面表示**：
   - 温度が水色の大きな文字で表示
   - 湿度が黄色の大きな文字で表示
   - 更新時刻が表示される

### 期待される出力例

**シリアルモニタ**：
```
温度: 23.45 °C
湿度: 45.20 %RH
```

**画面表示**：
```
Unit-ENV

23.5C

45%

Update: 123 sec
```

## トラブルシューティング

### センサーが認識されない場合

1. Grove接続の確認：赤いポート（I2C用）に接続されているか
2. ケーブルの確認：しっかり挿入されているか
3. I2Cスキャンコードで0x44、0x76が検出されるか確認

### コンパイルエラーの場合

1. M5CoreS3ライブラリが正しくインストールされているか確認
2. ボード設定がM5Stack-CoreS3になっているか確認
3. Arduino IDEを再起動

### アップロードエラーの場合

1. COMポートが正しく選択されているか確認
2. CoreS3がダウンロードモード（緑色LED点灯）になっているか確認
3. USB Type-Cケーブルを別のものに交換

## まとめ

このプロジェクトでは、M5Stack CoreS3とUnit-ENVセンサーを使用して温湿度モニターを作成しました。基本的なセンサー読み取りから始まり、画面表示まで段階的に機能を追加しました。

このベースコードを参考に、データロギング、Wi-Fi通信、アラート機能など、さらなる機能拡張も可能です。
