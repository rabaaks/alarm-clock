#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

byte size{4};

char keys[size][size]{
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte row[size]{11, 10, 9, 8};
byte col[size]{7, 6, 5, 4};

Keypad keypad = Keypad(makeKeymap(keys), row, col, size, size);

unsigned long startingTime{};
unsigned long alarmTime{};

unsigned long seconds{};
unsigned long lastSeconds{};

byte hoursPart{};
byte minutesPart{};
byte secondsPart{};

// Arduino Blink Example
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  lcd.init();
  lcd.backlight();
}

void displayTime() {
  lcd.setCursor(0, 0);
  lcd.print("Time: %d:%d:%d", hoursPart, minutesPart, secondsPart);
}

void idle() {
  displayTime();
  state = "idle";
}

void menu() {
  lcd.setCursor(0, 0);
  lcd.print("A-Alarm B-Time");
  lcd.setCursor(1, 0);
  lcd.print("C-Record D-Pswd");
  state = "menu";
}

void setAlarm() {

}

void setTime() {

}

void recordAlarm() {

}

void setPassword() {

}

void (*state)(){idle};
void (*lastState)(){idle};

void loop() {
  lastSeconds = seconds;
  seconds = millis() / 1000;
  bool timeChanged = lastSeconds != seconds;
  if (timeChanged) {
    hoursPart = seconds / 3600 % 24;
    minutesPart = seconds / 60 % 60;
    secondsPart = seconds % 60;
  }
  char key = keypad.getKey();
  switch (state) {
    case idle:
      if (timeChanged) {
        displayTime();
      }
      if (key == '#') menu();
      break;
    case menu:
      if (key) {
        switch (key) {
          case 'A':
            setAlarm();
          case 'B':
            setTime();
          case 'C':
            recordAlarm();
          case 'D':
            setPassword();
        }
      }
  }
}