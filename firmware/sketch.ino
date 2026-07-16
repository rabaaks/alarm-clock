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

char key{};
char lastKey{};

// States are stored as functions that are run when the state is entered
void (*state)(){idle};
void (*lastState)(){idle};

// For temporary states, the timer tracks when it should go back to the previous state
unsigned long timer{};

// Arduino Blink Example
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  lcd.init();
  lcd.backlight();
}

void displayTime() {
  lcd.setCursor(0, 0);
  lcd.print("Time: " + hoursPart + ":" + minutesPart + ":" + secondsPart);
}

void idle() {
  displayTime();
}

void menu() {
  lcd.setCursor(0, 0);
  lcd.print("A-Alarm B-Time");
  lcd.setCursor(1, 0);
  lcd.print("C-Record D-Pswd");
  timer = millis() + 5000;
  // If the previous state was another temporary state, make sure to go back to idle
  lastState = idle;
}

byte enteredTimeDigits[6]{};
byte enteredTimeIndex{};

void displayEnteredTime() {
  lcd.setCursor(1, 0);
  for (byte i{}; i < enteredTimeIndex; i++) {
    byte digit = enteredTimeDigits[i];
    lcd.print(digit);
    if (i == 1 || i == 3) {
      lcd.print(":");
    }
  }
}

void displayEnteredPassword() {
  lcd.setCursor(1, 0);
  for (byte i{}: i < enteredPasswordIndex; i++) {
    lcd.print("*");
  }
}

void setAlarm() {
  lcd.setCursor(0, 0);
  lcd.print("Set alarm:")
  lcd.setCursor(1, 0);
  displayEnteredTime();
  timer = millis() + 10000
}

void setTime() {
  lcd.setCursor(0, 0);
  lcd.print("Set time:")
  lcd.setCursor(1, 0);
  displayEnteredTime();
  timer = millis() + 10000
}



void displayRecordingProgress() {
  lcd.setCursor(1, 0);
  byte num{recordingTimer - millis()}
}

void recordAlarm() {
  lcd.setCursor(0, 0);
  lcd.print("Recording in progress");
  if (recordingTimer == 0) {
    recordingTimer = millis()
  }
  displayRecordingProgress();
}

void setPassword() {
  lcd.setCursor(0, 0);
  lcd.print("Set password:")
  
  timer = millis() + 10000
}

void changeState((*newState)()) {
  lastState = state;
  state = newState
  state();
}

void loop() {
  lastSeconds = seconds;
  seconds = millis() / 1000;
  bool timeChanged = lastSeconds != seconds;
  if (timeChanged) {
    hoursPart = seconds / 3600 % 24;
    minutesPart = seconds / 60 % 60;
    secondsPart = seconds % 60;
  }
  lastKey = key;
  key = keypad.getKey();
  bool keyChanged = lastKey != key;

  if (timer == millis()) {
    timer = 0;
    changeState(previousState);
  }

  switch (state) {
    case idle:
      if (timeChanged) {
        displayTime();
      }
      if (key == '#') changeState(menu);
      break;
    case menu:
      if (key) {
        switch (key) {
          case 'A':
            changeState(setAlarm);
            break;
          case 'B':
            changeState(setTime);
            break;
          case 'C':
            changeState(recordAlarm);
            break;
          case 'D':
            changeState(setPassword);
            break;
        }
      }
    case setAlarm:
    case setTime:

  }
}