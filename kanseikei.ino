#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_INA219.h>
#include <DallasTemperature.h>
#include <OneWire.h>

Adafruit_INA219 ina219_A;
Adafruit_INA219 ina219_B(0x41);
Adafruit_INA219 ina219;              // INA219オブジェクトの生成(アドレス0x40)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD address to 0x27 for a 16 chars and 2 line display
#define ONE_WIRE_BUS 10              // データ(黄)で使用するポート番号
#define SENSER_BIT 9                 // 精度の設定bit
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);  //                                                温度センサはここまでならいける

const int pwmPin = 9;     // PWM出力に使うピン番号
const int buttonPin = 4;  // デジタル3番ピンを指定 (ボタンピン)

unsigned long totalTime = 10000;  // <===================================================================== timeモードの時、この数値を変えて合計時間（ミリ秒）を設定(初期値10秒)
float beta = 5.0;
const int pwmStartValue = 110;  //電流が流れ始めるpwm値
const int pwmEndValue = 240;    //電流が流れがとまるpwm値
unsigned long startTime;
const int d = 100;

unsigned long currentMillis;

float v;
float m;

bool hasfull = false;  // フラグ変数を定義

float voltage = 0;
int integerPart = 0;
int fractionalPart = 0;
int vol_val;

float TempA;  //温度

int PCpwmValue;


void setup() {
  Serial.begin(115200);
  lcd.begin();                        // LCDの初期化
  pinMode(pwmPin, OUTPUT);            // PWMピンを出力モードに設定
  pinMode(buttonPin, INPUT_PULLUP);   // 3番ピンをプルアップ付きの入力ピンとして設定
  ina219.begin();                     // INA219との通信を開始(初期化)
  ina219.setCalibration_32V_1A();     // 測定レンジの設定
  sensors.setResolution(SENSER_BIT);  //Temp sensors
                                      // lcd.setCursor(6, 0);
                                      //  lcd.print("START");  //  lcd.print("START");
                                      // delay(2000);
  xTaskCreate(TaskJIKAN, "JIKAN", 256, NULL, 2, NULL);

  xTaskCreate(Taskprog, "prog", 256, NULL, 2, NULL);
}


void TaskJIKAN(void *pvParameters) {
  (void)pvParameters;
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  startTime = millis();
  for (;;) {
    if (digitalRead(buttonPin) == 1) {
      analogWrite(pwmPin, 0);
      break;
    }

    currentMillis = millis() - startTime;  // ミリ秒単位で経過時間を取得
                                           //  Serial.println(currentMillis);
    if (currentMillis < totalTime + d) {
      integerPart = (currentMillis) / 1000;           // 整数部の秒
      fractionalPart = (currentMillis % 1000) / 100;  // 小数部の1桁（ミリ秒を100で割る）
      lcd.setCursor(0, 0);
      lcd.print(integerPart);
      lcd.print(".");
      lcd.print(fractionalPart);  // 小数部を1桁表示
      m = (float)totalTime / 1000;
      lcd.setCursor(8, 0);
      lcd.print(m, 1);
      lcd.print("    ");
    }
    if (currentMillis >= totalTime + d) {
      if (!hasfull) {
        lcd.setCursor(0, 0);
        lcd.print(m, 1);
        hasfull = true;  // 処理が終わったらフラグをtrueにする
      }
    }
    sensors.requestTemperatures();             // 温度取得要求
    TempA = sensors.getTempCByIndex(0) + 3.3;  //温度センサ1を値(0)に (黄)
    vTaskDelay(50 / portTICK_PERIOD_MS);       // 10msのディレイを追加
  }
}

void Taskprog(void *pvParameters) {
  (void)pvParameters;
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  startTime = millis();
  for (;;) {
    if (digitalRead(buttonPin) == 1) {
      analogWrite(pwmPin, 0);
      break;
    }
    unsigned long currentTime = millis() - startTime;

    if (currentTime <= totalTime + d) {
      float T = (float)currentTime;

      // PWM 値を計算
      int pwmValue = pwmStartValue + (int)((pwmEndValue - pwmStartValue) * pow(T / totalTime, beta));

      // 計算された PWM 値を 9 番ピンに出力
      analogWrite(pwmPin, pwmValue);
      voltage = pwmValue * 0.0196;
      for (int i = 0; i < 2; i++) {
        Serial.print(currentTime / 1000.0);
        Serial.print("  ");
        Serial.print(vol_val);
        Serial.print("  ");
        Serial.print(v);
        Serial.print("  ");
        Serial.println(TempA);
        vTaskDelay(1 / portTICK_PERIOD_MS);
      }
    } else {
      analogWrite(pwmPin, pwmEndValue);  // 完了後、PWM ピンを最大値に設定
    }

    vol_val = ina219.getCurrent_mA();
    v = analogRead(A0) * (5.0 / 1023.0);

    if (vol_val < 10) {
      lcd.setCursor(4, 1);
      lcd.print("  ");
    }
    lcd.setCursor(0, 1);
    lcd.print(vol_val);
    lcd.print("mA");
    lcd.setCursor(8, 1);
    lcd.print(TempA, 1);
    lcd.print("C  ");

    vTaskDelay(100 / portTICK_PERIOD_MS);  // 10msのディレイを追加
  }
}


void loop() {
}
