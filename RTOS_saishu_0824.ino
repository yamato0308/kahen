#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219_A;
Adafruit_INA219 ina219_B(0x41);
Adafruit_INA219 ina219;              // INA219オブジェクトの生成(アドレス0x40)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD address to 0x27 for a 16 chars and 2 line display

const int pwmPin = 9;     // PWM出力に使うピン番号
const int buttonPin = 4;  // デジタル3番ピンを指定 (ボタンピン)

const int maxPWMValue = 240;      // PWMの最大値 (5V相当)
unsigned long totalTime = 12000;  // <===================================================================== timeモードの時、この数値を変えて合計時間（ミリ秒）を設定(初期値10秒)
float beta = 7.0;
const int pwmStartValue = 110;  //電流が流れ始めるpwm値
const int pwmEndValue = 240;    //電流が流れがとまるpwm値
unsigned long startTime;
int pwmValue;
int PCpwmValue = 0;

unsigned long currentMillis;
int t = 20;
float vout;
float v;
unsigned long previousMillisTime = 0;
unsigned long previousMillisPc = 0;
float intervalTime;                                    // time()関数での1秒間隔
const TickType_t delayTime = 20 / portTICK_PERIOD_MS;  // 10msのディレイ
const long intervalPc = 1000;                          // pc()関数での1秒間隔
float m;
float a;
unsigned long currentTime;

bool hasa = false;      // フラグ変数を定義
bool hasfull = false;   // フラグ変数を定義
bool JIKANOUT = false;  // フラグ変数を定義
bool progOUT = false;   // フラグ変数を定義


float voltage = 0;
int integerPart = 0;
int fractionalPart = 0;



void TaskJIKAN(void *pvParameters);
void Taskprog(void *pvParameters);

void setup() {

  Serial.begin(115200);
  lcd.begin();                       // LCDの初期化
  pinMode(pwmPin, OUTPUT);           // PWMピンを出力モードに設定
  pinMode(buttonPin, INPUT_PULLUP);  // 3番ピンをプルアップ付きの入力ピンとして設定
  ina219.begin();                    // INA219との通信を開始(初期化)
  ina219.setCalibration_32V_1A();    // 測定レンジの設定
                                     // lcd.setCursor(6, 0);
                                     //  lcd.print("START");  //  lcd.print("START");
                                     // delay(2000);
  startTime = millis();
zero:
  xTaskCreate(TaskJIKAN, "JIKAN", 256, NULL, 2, NULL);

  xTaskCreate(Taskprog, "prog", 256, NULL, 2, NULL);
}


void TaskJIKAN(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    if (digitalRead(buttonPin) == 1) {

    } else {
      if (!hasa) {
        a = millis();
        hasa = true;  // 処理が終わったらフラグをtrueにする
      }
      currentMillis = millis() - a;  // ミリ秒単位で経過時間を取得
                                     //  Serial.println(currentMillis);
      if (currentMillis < totalTime) {
        integerPart = currentMillis / 1000;             // 整数部の秒
        fractionalPart = (currentMillis % 1000) / 100;  // 小数部の1桁（ミリ秒を100で割る）
        lcd.setCursor(0, 0);
        lcd.print("      ");  // 表示領域をクリア
        lcd.setCursor(0, 0);
        lcd.print(integerPart);
        lcd.print(".");
        lcd.print(fractionalPart);  // 小数部を1桁表示
        m = (float)totalTime / 1000;
        lcd.setCursor(8, 0);
        lcd.print(m, 2);
        lcd.setCursor(4, 0);  // 必要に応じて他の位置もクリア
        lcd.print("    ");
      }
      if (currentMillis >= totalTime) {
        if (!hasfull) {
          lcd.setCursor(0, 0);
          lcd.print(totalTime / 1000.0);
          // lcd.setCursor(0, 0);
          // lcd.print(integerPart);
          // lcd.print(".");
          // lcd.print(fractionalPart);  // 小数部を1桁表示
          hasfull = true;  // 処理が終わったらフラグをtrueにする
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);  // 10msのディレイを追加
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);  // 10msのディレイを追加
  }
}

void Taskprog(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    if (digitalRead(buttonPin) == 1) {
      pwmValue = 0;
      analogWrite(pwmPin, pwmValue);
      siteir();
      vTaskDelay(100 / portTICK_PERIOD_MS);  // 10msのディレイを追加
    } else {
      currentTime = millis() - startTime;

      if (currentTime <= totalTime) {
        float T = (float)currentTime;

        // PWM 値を計算
        pwmValue = pwmStartValue + (int)((pwmEndValue - pwmStartValue) * pow(T / totalTime, beta));

        // 計算された PWM 値を 9 番ピンに出力
        analogWrite(pwmPin, pwmValue);
        voltage = pwmValue * 0.0196;

        // Teleplot フォーマットで PWM 値をシリアル出力
        Serial.print("pwmValue:");  // キーを出力
        Serial.println(pwmValue);

        float current_mA = ina219.getCurrent_mA();
        int vol_val = 0;
        int vol_val2 = 0;
        for (int i = 0; i < 10; i++) {
          vol_val += current_mA;
        }
        vol_val2 = vol_val / 10;

        lcd.setCursor(0, 1);
        lcd.print(vol_val2);
        lcd.print("mA");
        if (vol_val2 < 100) {
          lcd.setCursor(4, 1);
          lcd.print("     ");
        }
      } else {
        // 終了処理など
        analogWrite(pwmPin, pwmEndValue);  // 完了後、PWM ピンを最大値に設定
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);  // 10msのディレイを追加
  }
}

void sitei() {

  for (;;) {
    lcd.setCursor(7, 0);
    lcd.print(" sitei    ");
    if (digitalRead(buttonPin) == 0) {
      break;
    } else {
      if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        float voltage = input.toFloat();

        if (voltage >= 0 && voltage <= 5.0) {
          lcd.clear();
          PCpwmValue = map(voltage * 100, 0, 500, 0, 255);
          lcd.backlight();
          lcd.setCursor(0, 0);
          lcd.print("V:");
          lcd.print(voltage);
        } else {
          Serial.println("Invalid voltage. Please enter a voltage between 0 and 5 V.");
        }
        analogWrite(pwmPin, PCpwmValue);
      }


      float current_mA = 0;


      current_mA = ina219.getCurrent_mA();  // 電流の計測
      int vol_val = 0;
      int vol_val2 = 0;
      for (int i = 0; i < 10; i++)  // {}内を30回繰り返し
      {
        vol_val += current_mA;    //
        vol_val2 = vol_val / 10;  // vol_valにvol_val30回分で割った値を代入
      }
     
      

      


      Serial.print("Current: ");
      Serial.print(vol_val2);
      Serial.println(" mA");
      Serial.println("");
      lcd.setCursor(0, 1);
      lcd.print("Ic: ");
      lcd.print(vol_val2);
      lcd.print("mA");
      delay(1000);
      lcd.setCursor(0, 1);
      lcd.print("                ");  // 16個のスペースでクリア
      // 少し待つ
    }
  }
}


void loop() {
}
