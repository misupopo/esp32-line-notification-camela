#include <WiFiClientSecure.h>           // セキュアコネクションTLS(SSL)ライブラリ
#include <esp_camera.h>                 // ESPカメラライブラリ
#include "settings.h"                   // 設定情報

// 通知メッセージ作成用の部品
#define BOUNDARY        "--------------------------133747188241686651551404"  
#define BODY_MESSAGE    "\r\n--" BOUNDARY "\r\nContent-Disposition: form-data; name=\"message\"\r\n\r\n" MESSAGE
#define BODY_IMAGEFILE  "\r\n--" BOUNDARY "\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"picture.jpg\"\r\n\r\n"
#define BODY_END        "\r\n--" BOUNDARY "--\r\n"

// グローバル変数・オブジェクト
camera_fb_t *fb;          // 写真用
WiFiClientSecure client;  // セキュアコネクション用

// カメラの初期化（CameraWebServerのサンプルコードを改良）
void initCamera() {
  camera_config_t config;
  config.pin_sscb_scl = 23;             // SIOC_GPIO_NUM
  config.pin_sscb_sda = 25;             // SIOD_GPIO_NUM
  config.pin_xclk =     27;             // XCLK_GPIO_NUM
  config.pin_vsync =    22;             // VSYNC_GPIO_NUM
  config.pin_href =     26;             // HREF_GPIO_NUM
  config.pin_pclk =     21;             // PCLK_GPIO_NUM
  config.pin_d0 =       32;             // Y2_GPIO_NUM
  config.pin_d1 =       35;             // Y3_GPIO_NUM
  config.pin_d2 =       34;             // Y4_GPIO_NUM
  config.pin_d3 =        5;             // Y5_GPIO_NUM
  config.pin_d4 =       39;             // Y6_GPIO_NUM
  config.pin_d5 =       18;             // Y7_GPIO_NUM
  config.pin_d6 =       36;             // Y8_GPIO_NUM
  config.pin_d7 =       19;             // Y9_GPIO_NUM
  config.pin_reset =    15;             // RESET_GPIO_NUM
  config.pin_pwdn =     -1;             // PWDN_GPIO_NUM
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer =   LEDC_TIMER_0;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size =   FRAMESIZE_UXGA;
  config.jpeg_quality = 10;             // 0-63 値が小さいほど品質が高い
  config.fb_count =      1;             // 1より大きい場合は連続モード
  esp_camera_init(&config);
  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, CAMERA_VFLIP);        // 写真の上下反転設定
  s->set_hmirror(s, CAMERA_VFLIP);      // 写真の左右反転設定
  delay(1000);
}

// WiFi接続
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);       // WiFiを実行
  Serial.print(WIFI_SSID "に接続中");
  while (WiFi.status() != WL_CONNECTED) { // 繋がるまでループ
    Serial.print(".");
    delay(1000);
  }
  Serial.println("接続完了");
}

// 写真をLINEに送信する
void sendPhoto() {
  int content_len = strlen(BODY_MESSAGE) + strlen(BODY_IMAGEFILE) + fb->len + strlen(BODY_END); // 通知メッセージのサイズを計算
  client.setInsecure();
  client.connect("notify-api.line.me", 443);   // 通知用APIサーバーに接続
  client.println("POST /api/notify HTTP/1.0"); // HTTP POSTリクエスト出力
  client.println("Authorization: Bearer " ACCESS_TOKEN); // 認証用ヘッダー出力 
  client.println("Content-Type: multipart/form-data; boundary=" BOUNDARY); // コンテントタイプヘッダー出力
  client.println(String("Content-Length: ") + content_len); // コンテントレングスヘッダー出力
  client.println();                            // 空行を出力
  client.write((uint8_t *)BODY_MESSAGE, strlen(BODY_MESSAGE));     // メッセージパートの出力
  client.write((uint8_t *)BODY_IMAGEFILE, strlen(BODY_IMAGEFILE)); // イメージファイルパートのヘッダー出力
  uint8_t *send_pointer = fb->buf;             // 写真を分割して出力する処理
  for (int remaining = fb->len; remaining > 0; remaining -= 10240) {
    client.write(send_pointer, remaining > 10240 ? 10240 : remaining);
    send_pointer += 10240;
  }
  client.write((uint8_t *)BODY_END, strlen(BODY_END)); // マルチパートのメッセージ終了を出力
  while (!client.available()) {                // サーバーからのレスポンスを待つ
    delay(10);
  }
  Serial.print("LINEに写真を送信しました。\nサーバーからのレスポンス : ");
  Serial.println(client.readStringUntil('\n')); // レスポンスを表示
  client.stop();                                // セキュアコネクション接続終了
}

void setup() {
  Serial.begin(115200);                   // シリアルモニタの通信速度設定
  pinMode(2, OUTPUT);                     // LED接続GPIOを出力に設定
  digitalWrite(2, HIGH);                  // LED点灯
  initCamera();                           // カメラの初期化
  fb = esp_camera_fb_get();               // 写真を撮る
  connectWiFi();                          // WiFi接続
  sendPhoto();                            // 写真をLINEに送信する
  WiFi.disconnect(true);                  // WiFi切断

  //スリープ処理
#ifdef USE_PIR
  // PIRモーションセンサーを使う場合
  delay(PIR_WAIT_TIME*1000);              // 連続送信防止用の待ち時間（ミリ秒）
  // GPIO13がHIGHになったらスリープから復帰するように設定
  Serial.println("PIRが検知するまでスリープします。");
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, HIGH);
#else
  // ディープスリープタイマーを使う場合
  Serial.printf("%d分間スリープします。", INTERVAL_TIME);
  esp_sleep_enable_timer_wakeup(INTERVAL_TIME*60*1000*1000UL); // ディープスリープタイマーを設定（マイクロ秒）
#endif
  esp_deep_sleep_start();                 // スリープ実行
}

void loop() {
}
