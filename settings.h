/*
 * settings.h
 * 設定情報
*/

// WiFiの資格情報 (2.4GHzのみ)
#define WIFI_SSID         "xxxxxx-G-xxxx" // 2.4Ghzを指定する
#define WIFI_PASS         "wifipassword" // wifiのパスワードを設定する

// LINEの資格情報
#define ACCESS_TOKEN      "xxxxxxxxxxxxxxxxxxxx" // line notify のアクセストークンをセット

// 設定情報
#define CAMERA_VFLIP      1             // カメラの設置向き（本体のM5文字の向き） 1:正常 0:逆さ
//#define USE_PIR                         // タイマーを使う場合はコメントにする

#ifdef USE_PIR
  // PIR用の設定
  #define PIR_WAIT_TIME   10            // 連続送信防止用の待ち時間（秒）
  #define MESSAGE         "検知しました"  // 写真と共に送られるメッセージ
#else
  // タイマー用の設定
  #define INTERVAL_TIME   1            // 送信間隔（分）
  #define MESSAGE         "通知しましたよ！"      // 写真と共に送られるメッセージ
#endif
