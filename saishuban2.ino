#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219_A;
Adafruit_INA219 ina219_B(0x41);
Adafruit_INA219 ina219;              // INA219オブジェクトの生成(アドレス0x40)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD address to 0x27 for a 16 chars and 2 line display
const int pwmPin = 9;                // PWM出力に使うピン番号
const int stepSize = 5;              // 0.1V相当のステップ数
const int maxPWMValue = 255;         // PWMの最大値 (5V相当)
const int buttonPin = 3;             // デジタル3番ピンを指定 (ボタンピン)
unsigned long totalTime = 30000;      // 合計時間（ミリ秒）
unsigned long delayTime;
static int pwmValue = 0;

float vout;
float v;
unsigned long previousMillisTime = 0;
unsigned long previousMillisPc = 0;
unsigned long intervalTime;    // time()関数での1秒間隔
const long intervalPc = 1000;  // pc()関数での1秒間隔
int vol_val2 = 0;
int pft;

float voltage = 0;
void setup() {
  lcd.begin();                       // LCDの初期化
  pinMode(pwmPin, OUTPUT);           // PWMピンを出力モードに設定
  pinMode(buttonPin, INPUT_PULLUP);  // 3番ピンをプルアップ付きの入力ピンとして設定
  Serial.begin(115200);              // シリアル通信を開始
  ina219.begin();                    // INA219との通信を開始(初期化)
  ina219.setCalibration_32V_1A();    // 測定レンジの設定
  lcd.setCursor(6, 0);
  lcd.print("START");  //  lcd.print("START");
  delay(2000);
  lcd.setCursor(0, 0);
  lcd.print("                 ");
  pft = (totalTime / 1000)^2 * 2000;
}

void loop() {
  while (1) {  //time
    if (digitalRead(buttonPin) == 1) {
      lcd.setCursor(0, 0);
      lcd.print("                 ");
      lcd.setCursor(0, 1);
      lcd.print("                 ");
      break;
    }
    time();
    lcd.setCursor(10, 0);
    lcd.print(totalTime / 1000);
    //  Serial.println(millis());
    displayTime();
    V();
    delay(1);
  }
  while (1) {  //pc
    if (digitalRead(buttonPin) == 0) {
      lcd.setCursor(0, 0);
      lcd.print("                 ");
      break;
    }
    pc();
    V();
  }
}

void time() {

  unsigned long CM = millis();
  intervalTime = totalTime / (maxPWMValue / stepSize);
  if (CM - previousMillisTime >= intervalTime) {
    previousMillisTime = CM;

    pwmValue += stepSize;
    if (pwmValue > maxPWMValue) pwmValue = 255;
    voltage = pwmValue * 0.0196;
    analogWrite(pwmPin, pwmValue);

    float current_mA = ina219.getCurrent_mA();
    int vol_val = 0;
    int vol_val2 = 0;

    for (int i = 0; i < 10; i++) {
      vol_val += current_mA;
    }
    vol_val2 = vol_val / 10;

    Serial.print("Output Voltage: ");
    Serial.print(voltage);
    Serial.println(" V");
    lcd.backlight();
    Serial.print("Current: ");
    Serial.print(vol_val2);
    Serial.println("mA");
    Serial.println("");
    lcd.setCursor(0, 1);
    lcd.print(vol_val2);
    lcd.print("mA");
    delay(1);
    lcd.setCursor(10, 1);
    lcd.print("         ");
  }
}

void pc() {

  unsigned long currentMillis = millis();
  lcd.setCursor(7, 0);
  lcd.print(" pcmode ");
  if (currentMillis - previousMillisPc >= intervalPc) {
    previousMillisPc = currentMillis;

    if (Serial.available() > 0) {
      String input = Serial.readStringUntil('\n');
      float voltage = input.toFloat();

      if (voltage >= 0 && voltage <= 5.0) {
        int pwmValue = map(voltage * 100, 0, 500, 0, 255);
        analogWrite(pwmPin, pwmValue);
        Serial.print("Output Voltage: ");
        Serial.print(voltage);
        Serial.println("V");
        lcd.backlight();
        lcd.setCursor(0, 0);
        lcd.print("V:");
        lcd.print(voltage);
      } else {
        Serial.println("Invalid voltage. Please enter a voltage between 0 and 5 V.");
      }
    }

    float current_mA = ina219.getCurrent_mA();
    int vol_val = 0;

    for (int i = 0; i < 10; i++) {
      vol_val += current_mA;
    }
    vol_val2 = vol_val / 10;

    Serial.print("Current: ");
    Serial.print(vol_val2);
    Serial.println("mA");
    Serial.println("");
    lcd.setCursor(0, 1);
    lcd.print(vol_val2);
    lcd.print("mA");
    delay(1);
    lcd.setCursor(10, 1);
    lcd.print("         ");
  }
}
void V() {
  vout = analogRead(A0);
  v = (vout * 5) / 1023;
  lcd.setCursor(7, 1);
  lcd.print("v:");
  lcd.print(v);
}
void displayTime() {
  long currentMillis = millis() - pft;  // ミリ秒単位で経過時間を取得
  if (currentMillis > 0) {
    if (pwmValue >= (maxPWMValue - stepSize)) {

      float seconds = 0;
      seconds = currentMillis / 1000.0;  // 秒に変換

      int integerPart = 0;
      integerPart = currentMillis / 1000;  // 整数部の秒
      int fractionalPart = 0;
      fractionalPart = (currentMillis % 1000) / 100;  // 小数部の1桁（ミリ秒を100で割る）

      // 整数部と小数部を組み合わせて表示
      lcd.setCursor(0, 0);
      lcd.print(totalTime / 1000);
      lcd.print(".0");

      Serial.print("Time: ");
      Serial.print(integerPart);
      Serial.print(".");
      Serial.println(fractionalPart);
    }
    if (pwmValue <= (maxPWMValue - stepSize)) {
      lcd.setCursor(0, 0);
      lcd.print("         ");

      float seconds = 0;
      seconds = currentMillis / 1000.0;  // 秒に変換

      int integerPart = 0;
      integerPart = currentMillis / 1000;  // 整数部の秒
      int fractionalPart = 0;
      fractionalPart = (currentMillis % 1000) / 100;  // 小数部の1桁（ミリ秒を100で割る）

      // 整数部と小数部を組み合わせて表示
      lcd.setCursor(0, 0);
      lcd.print(integerPart);
      lcd.print(".");
      lcd.print(fractionalPart);  // 小数部を1桁表示

      Serial.print("Time: ");
      Serial.print(integerPart);
      Serial.print(".");
      Serial.println(fractionalPart);
    }
  } else if (currentMillis <= 0) {
    lcd.setCursor(0, 0);
    lcd.print("                 ");
    lcd.setCursor(0, 1);
    lcd.print("                 ");
  } else {
  }
}
