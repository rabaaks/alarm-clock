#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <SPI.h>
#include <Wire.h>
#include <IRremote.hpp>

const byte PIN_LM35{A1};
const byte PIN_BUZZER{3};

const byte PIN_YELLOW{A4};
const byte PIN_BLUE{A3};
const byte PIN_RED{A5};

const byte PIN_MIC{A2};

const byte PIN_IR{2};

const byte PIN_POT{A0};

const byte PIN_RFID_SS{10};

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte PAD_SIZE{4};

char keys[PAD_SIZE][PAD_SIZE]{
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte row[PAD_SIZE]{1, 0, 9, 8};
byte col[PAD_SIZE]{7, 6, 5, 4};

Keypad keypad = Keypad(makeKeymap(keys), row, col, PAD_SIZE, PAD_SIZE);

MFRC522DriverPinSimple ssPin{PIN_RFID_SS};
MFRC522DriverSPI rfidDriver{ssPin};
MFRC522 rfid{rfidDriver};

enum State {
  BASE,
  MENU,
  SET_ALARM,
  SET_TIME,
  RECORD,
  SET_PASSWORD,
  VOLUME,
  ALARM,
  NUM_STATES
};

State state = BASE;
State lastState = BASE;

// For temporary states, the timer tracks when it should go back to the previous state
unsigned long stateTimer{};

struct StateFunctions {
  void (*init)();
  void (*run)();
};

void idleInit();
void idleRun();

void menuInit();
void menuRun();

void setAlarmInit();
void setAlarmRun();

void setTimeInit();
void setTimeRun();

void recordInit();
void recordRun();

void setPasswordInit();
void setPasswordRun();

void volumeInit();
void volumeRun();

void alarmInit();
void alarmRun();


StateFunctions states[NUM_STATES] = {
  {idleInit, idleRun},
  {menuInit, menuRun},
  {setAlarmInit, setAlarmRun},
  {setTimeInit, setTimeRun},
  {recordInit, recordRun},
  {setPasswordInit, setPasswordRun},
  {volumeInit, volumeRun},
  {alarmInit, alarmRun}
};

void changeState(State newState) {
  lastState = state;
  state = newState;
  stateTimer = 0;
  states[state].init();
}

void setTimeout(unsigned long milliseconds) {
  stateTimer = millis() + milliseconds;
}

unsigned long startingTime{};
unsigned long alarmTime{};

unsigned long seconds{};
unsigned long lastSeconds{};

const byte PASSWORD_LENGTH{6};
char password[PASSWORD_LENGTH+1]{};

const char noteKeys[]{'1', '2', '3', 'A', '4', '5', '6', 'B', '7', '8', '9', 'C'};
const unsigned int frequencies[]{262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494};
const byte SOUND_LENGTH{16};
unsigned int recordedSound[SOUND_LENGTH];

const byte MAX_VOLUME{10};
byte volume = MAX_VOLUME / 2;

byte hoursPart{};
byte minutesPart{};
byte secondsPart{};
int temperature;

byte alarmHours{};
byte alarmMinutes{};

void updateClock() {
  lastSeconds = seconds;
  seconds = millis() / 1000;
  bool timeChanged = lastSeconds != seconds;
  if (timeChanged) {
    hoursPart = seconds / 3600 % 24;
    minutesPart = seconds / 60 % 60;
    secondsPart = seconds % 60;

    temperature = (int) ((analogRead(PIN_LM35) * (500.0 / 1023.0)) * 1.8 + 32);
  }
}

void paddingPrint(byte num) {
  if (num < 10) lcd.print('0');
  lcd.print(num);
}

void idleInit() {

}

void idleRun() {
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  paddingPrint(hoursPart);
  lcd.print(':');
  paddingPrint(minutesPart);
  lcd.print(':');
  paddingPrint(secondsPart);

  lcd.setCursor(0, 1);
  lcd.print("Alarm: ");
  paddingPrint(alarmHours);
  lcd.print(':');
  paddingPrint(alarmMinutes);

  lcd.print(' ');
  lcd.print(temperature);
  lcd.print('\xB0');

  char key = keypad.getKey();
  if (key == '#') {
    changeState(MENU);
  }
}

void menuInit() {
  lcd.setCursor(0, 0);
  lcd.print("A-Alarm B-Time");
  lcd.setCursor(1, 0);
  lcd.print("C-Record D-Pswd");
  setTimeout(5000);
  // If the previous state was another temporary state, make sure to go back to idle
  lastState = BASE;
}

void menuRun() {
  char key = keypad.getKey();
  if (key) {
    switch (key) {
      case 'A':
        changeState(SET_ALARM);
        break;
      case 'B':
        changeState(SET_TIME);
        break;
      case 'C':
        changeState(RECORD);
        break;
      case 'D':
        changeState(SET_PASSWORD);
        break;
    }
  }
}

byte enteredTimeDigits[4]{};
byte enteredTimeIndex{};

void displayEnteredTime(char* label) {
  lcd.setCursor(0, 0);
  lcd.print(label);
  lcd.setCursor(1, 0);
  for (byte i{}; i < enteredTimeIndex; i++) {
    byte digit = enteredTimeDigits[i];
    lcd.print(digit);
    if (i == 1) {
      lcd.print(":");
    }
  }
}

bool timeEntry() {
  char key = keypad.getKey();
  if (!key) return false;
  
  setTimeout(10000);
  if (key >= '0' && key <= '9' && enteredTimeIndex < 4) {
    enteredTimeDigits[enteredTimeIndex] = key - '0';
    enteredTimeIndex++;
    return false;
  }

  if (key == '*' && enteredTimeIndex > 0) {
    enteredTimeIndex--;
    return false;
  }

  if (key == '#' && enteredTimeIndex == 4) {
    return true;
  }

  return false;
}

void setAlarmInit() {
  lcd.setCursor(1, 0);
  displayEnteredTime("Set alarm:");
  setTimeout(10000);
}

void setTimeInit() {
  lcd.setCursor(1, 0);
  displayEnteredTime("Set time:");
  setTimeout(10000);
}

bool alarmEnabled = false;

void setAlarmRun() {
  if (timeEntry()) {
    alarmHours = (enteredTimeDigits[0] * 10 + enteredTimeDigits[1]);
    alarmMinutes = (enteredTimeDigits[2] * 10 + enteredTimeDigits[3]);

    alarmEnabled = true;

    changeState(BASE);
  }

  displayEnteredTime("Set alarm: ");
}

void setTimeRun() {
  if (timeEntry()) {
    seconds = (enteredTimeDigits[0] * 10 + enteredTimeDigits[1]) * 3600 + (enteredTimeDigits[2] * 10 + enteredTimeDigits[3]) * 60;

    changeState(BASE);
  }

  displayEnteredTime("Set time: ");
}

byte recordIndex{};

void recordInit() {
  recordIndex = 0;
  lcd.setCursor(0, 0);
  lcd.print("Recording");
}

void recordRun() {
  char key = keypad.getKey();
  if (!key) return;

  setTimeout(10000);
  if (key == '*' && recordIndex > 0) {
    recordIndex--;
  } else {
    for (byte i{}; i < 12; i++) {
      if (key == noteKeys[i]) {
        tone(PIN_BUZZER, frequencies[i], 250);
        if (recordIndex < SOUND_LENGTH) {
          recordedSound[recordIndex] = frequencies[i];
          recordIndex++;
        }
        break;
      }
    }
  }
  lcd.setCursor(0, 1);
  lcd.print(recordIndex + 1);
  lcd.print("/16 Notes");
}

byte passwordIndex{};

void setPasswordInit() {
  passwordIndex = 0;
  lcd.setCursor(0, 0);
  lcd.print("Set password:");
  setTimeout(10000);
}

void setPasswordRun() {
  char key = keypad.getKey();
  if (!key) return;

  setTimeout(10000);
  if (key == '*' && passwordIndex > 0) {
    passwordIndex--;
  } else if (passwordIndex < PASSWORD_LENGTH) {
    password[passwordIndex] = key;
    passwordIndex++;
  }

  lcd.setCursor(0, 1);
  for (byte i{}; i < passwordIndex; i++) {
    lcd.print("*");
  }

  if (passwordIndex == PASSWORD_LENGTH) {
    password[PASSWORD_LENGTH] = '\0';
    changeState(BASE);
  }
}

void volumeInit() {
  lcd.setCursor(0, 0);
  lcd.print("Volume");
  lcd.setCursor(0, 1);
  for (byte i{}; i < volume; i++) {
    lcd.print('X');
  }
  setTimeout(3000);
}

void volumeRun() {
  
}

bool passwordDone{}, rfidDone{}, clapDone{};

byte stopPasswordIndex{};
char stopPassword[PASSWORD_LENGTH]{};

byte ledStep{};
unsigned long ledTimer{};

byte noteIndex{};
unsigned long noteTimer{};

void alarmInit() {
  passwordDone = rfidDone = clapDone = false;

  lcd.setCursor(0, 0);
  lcd.print("Alarm Pass,RFID,");
  lcd.setCursor(0, 1);
  lcd.print("Clap to stop");
}

void alarmRun() {
  if (millis() > ledTimer) {
    ledTimer = millis() + 250;
    digitalWrite(PIN_YELLOW, ledStep == 0);
    digitalWrite(PIN_BLUE, ledStep == 1);
    digitalWrite(PIN_RED, ledStep == 2);
    ledStep = (ledStep + 1) % 3;
  }

  if (millis() > noteTimer) {
    noteTimer = millis() + 250;
    tone(PIN_BUZZER, recordedSound[noteIndex], 250);
    noteIndex = (noteIndex + 1) % SOUND_LENGTH;
  }

  char key = keypad.getKey();
  if (key && stopPasswordIndex < PASSWORD_LENGTH) {
    stopPassword[stopPasswordIndex] = key;
    stopPasswordIndex++;
    if (stopPasswordIndex == PASSWORD_LENGTH) {
      passwordDone = strcmp(stopPassword, password) == 0;
      if (!passwordDone) stopPasswordIndex = 0;
    }
  }

  if (!rfidDone && rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    rfidDone = true;
    rfid.PICC_HaltA();
  }

  if (!clapDone) {
    int level = analogRead(PIN_MIC);
    if (level > 500) {
      clapDone = true;
    }
  }

  if (passwordDone && rfidDone && clapDone) {
    noTone(PIN_BUZZER);
    digitalWrite(PIN_YELLOW, 0);
    digitalWrite(PIN_BLUE, 0);
    digitalWrite(PIN_RED, 0);
    changeState(BASE);
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  lcd.init();
  lcd.backlight();

  Wire.begin();
  SPI.begin();
  rfid.PCD_Init();
  IrReceiver.begin(PIN_IR);

  states[state].init();
}

const byte IR_VOLUME_UP{};
const byte IR_VOLUME_DOWN{};

int potReading = 0;

void loop() {
  updateClock();
  if (millis() >= stateTimer) {
    stateTimer = 0;
    changeState(lastState);
  }

  if (IrReceiver.decode()) {
    unsigned long code = IrReceiver.decodedIRData.decodedRawData;
    if (code == IR_VOLUME_UP && volume < 10) {
      volume++;
      changeState(VOLUME);
    } else if (code == IR_VOLUME_DOWN && volume > 0) {
      volume--;
      changeState(VOLUME);
    }
    IrReceiver.resume();
  }

  byte level = map(analogRead(PIN_POT), 0, 1023, 0, 10);
  if (level != potReading) {
    potReading = level;
    volume = level;
    changeState(VOLUME);
  }

  if (alarmEnabled && state != ALARM &&
    hoursPart == alarmHours && minutesPart == alarmMinutes) {
    changeState(ALARM);
  }

  states[state].run();
}