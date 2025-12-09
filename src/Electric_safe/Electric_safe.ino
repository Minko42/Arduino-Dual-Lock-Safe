#include <stdlib.h>
#include <Servo.h>
Servo myservo;

#include <LiquidCrystal.h>
// [構文]LiquidCrystal(rs, rw,  enable, d0, d1, d2, d3, d4, d5, d6, d7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//ピン番号の設定
const int R_LEDPin = 6;
const int G_LEDPin = 7;
const int One_Pin = 8;
const int Two_Pin = 9;
const int Servo_Pin = 10;
const int Sound_Pin = 13;
const int Reset_Pin = A0;
const int Enter_Pin = A1;
const int PotenPin = A2;

//定数の設定.
#define openfre 1000
#define opentime 500
#define closefre 200
#define closetime 800
#define passlen 4 //１～１０桁まで.
#define waittime 200

//初期値の設定.
const long digitalcode = 1111; //デジタルパスコード.
const int analogcode = 50; //アナログパスコード（０～１００まで）.

long dcode = 0;
int acode = 0;

bool reset = true;
bool potenlock = true;

//LCDの待機画面を設定する関数.
void lcdstart(){
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ENTER PASSCODE!");
  lcd.cursor();
}

//金庫を開ける関数.
void opensafe(){
  Serial.println("The safe is opened.");
  myservo.write(90);
  digitalWrite(G_LEDPin,HIGH);
  digitalWrite(R_LEDPin,LOW);
}

//金庫を閉じる関数.
void locksafe(){
  Serial.println("The safe is locked.");
  myservo.write(0);
  digitalWrite(G_LEDPin,LOW);
  digitalWrite(R_LEDPin,HIGH);
}

//デジタルパスコードを入力する関数.
long InputDigitalcode() {
  long pass = 0;
  for (int i = 0; i < passlen; i++) {
    lcd.setCursor(i, 1);

    //未入力は-1とする.
    int input_digit = -1;

    while (true) {
      if (digitalRead(One_Pin) == HIGH) {
        input_digit = 1;
      } 
      else if (digitalRead(Two_Pin) == HIGH) {
        input_digit = 2;
      }

      if (input_digit != -1) {
        while (digitalRead(One_Pin) == HIGH || digitalRead(Two_Pin) == HIGH) {
          delay(waittime);
        }
        lcd.print(input_digit);
        pass = (pass * 10) + input_digit;
        break;
      }
    }
  }
  return pass;
}

//アナログパスコードを入力する関数.
int InputAnalogcode(){
  int pass = 0;
  lcd.setCursor(passlen + 1, 1);
  while(true){
    int analogvalue = analogRead(PotenPin);
    pass = map(analogvalue,0,1023,0,100);
    lcd.setCursor(passlen + 1, 1);
    lcd.print(pass);
    lcd.print(" ");

    if(digitalRead(Enter_Pin) == HIGH){
      lcd.setCursor(14,1);
      lcd.print("L");
      potenlock = true;
      delay(waittime);
      break;
    }

     delay(100);
  }
  return pass;
}

//整数の桁数を返す関数.
int digitnum(long a){
  int digit = 0;
  while(a != 0){
    a = a / 10;
    digit++;
  }
  return digit;
}

//入力されたパスコードが正しいか判定する関数.
bool check(long a, long b){
  if(a == b){
    return true;
  }
  else{
    return false;
  }
}

//パスコードが異なる場合の処理をする関数.
void wrongpass(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PASSCODE IS");
  lcd.setCursor(0, 1);
  lcd.print("WRONG");
  tone(Sound_Pin,closefre,closetime);
}

//パスコードが正しい場合の処理をする関数.
void correctpass(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PASSCODE IS");
  lcd.setCursor(0, 1);
  lcd.print("CORRECT");
  tone(Sound_Pin,openfre,opentime);
}

//シリアルモニタに入力された値を表示する関数.
void printvalue(long digitalcode, int analogcode){
  Serial.print("The input value is ");
  Serial.print(digitalcode);
  Serial.print(" & ");
  Serial.print(analogcode);
  Serial.println("");
}


void setup() {
  myservo.attach(Servo_Pin,500,2400);
  Serial.begin(9600);

  pinMode(R_LEDPin, OUTPUT);
  pinMode(G_LEDPin, OUTPUT);
  pinMode(Sound_Pin, OUTPUT);
  pinMode(One_Pin, INPUT);
  pinMode(Two_Pin, INPUT);
  pinMode(Reset_Pin, INPUT);
  pinMode(Enter_Pin, INPUT);
}

void loop() {
  while(reset){
    Serial.println("------------------------------------------");
    locksafe();
    lcdstart();
    dcode = InputDigitalcode();
    
    if(digitnum(dcode) == passlen){
      reset = false;
      potenlock = false;
      break;
    }
  }

  while(!potenlock){
    acode = InputAnalogcode(); 
  }

  //エンターキーが押された時の処理.
  if(digitalRead(Enter_Pin) == HIGH){
    if(check(digitalcode,dcode) == true && check(analogcode, acode) == true){
      correctpass();
      opensafe();
    }
    else{
      wrongpass();
    }
    printvalue(dcode,acode);
    delay(waittime);
  }

  //リセットキーが押された時の処理.
  if(digitalRead(Reset_Pin) == HIGH){
    reset = true;
    delay(waittime);
  }
}
