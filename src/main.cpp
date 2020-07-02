#include <Arduino.h>
#include <Timers.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Wire.h>

//LCD
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

//Timery
Timer wateringTimer;
Timer intervalTimer;

//Pins
const int pump = 11;
const char sensor = A2;

//Keypad
const int buttonOne = 6;
const int buttonTwo = 7;
const int buttonThree = 9;
const int buttonFour = 8;

//Minimal moisture
const int moistureMinID = 1;
int moistureMin = EEPROM.read(moistureMinID);

//Watering time
int wateringMinutesID = 2;
int wateringSecondsID = 3;
int wateringMinutes = EEPROM.read(wateringMinutesID);
int wateringSeconds = EEPROM.read(wateringSecondsID);
long wateringTime = (int(wateringMinutes) * 60000) + (int(wateringSeconds) * 1000);

//Interval time
int intervalMinutesID = 4;
int intervalSecondsID = 5;
int intervalMinutes = EEPROM.read(intervalMinutesID);
int intervalSeconds = EEPROM.read(intervalSecondsID);
long intervalTime = (int(intervalMinutes) * 60000) + (int(intervalSeconds) * 1000);

//Other
int menuState = 0;
int counter = 0;
int counterWateringTime = 0;
int counterIntervalTime = 0;
int counterWatering = 0;
//Moisture sensor
int moistureValue = 0;
int moisturePercent = 0;

const int airValue = 620;
const int waterValue = 310;

bool save = false;
bool isMenu = true;

void setup() {
  Serial.begin(9600);

  setWateringTime();
  setIntervalTime();

  //Set keypad buttons
  pinMode(buttonOne, INPUT_PULLUP);
  pinMode(buttonTwo, INPUT_PULLUP);
  pinMode(buttonThree, INPUT_PULLUP);
  pinMode(buttonFour, INPUT_PULLUP);

  //Set pump as output
  pinMode(pump, OUTPUT);

  //Set sensor as input
  pinMode(sensor, INPUT);

  //Set LCD
  lcd.begin(16, 2);
  lcd.print("Welcome!");

  wateringTimer.begin(wateringTime);
}

void loop() {
  moistureValue = analogRead(sensor);
  moisturePercent = map(moistureValue, airValue, waterValue, 0, 100);
  if(moisturePercent < 0) {
    moisturePercent = 0;
  }
  if(moisturePercent > 100) {
    moisturePercent = 100;
  }
  if (isMenu == true) {
    if (digitalRead(buttonOne) == LOW) {
      counter++;
      if (counter > 3) {
        counter = 0;
      }
      delay(200);
    }
    menu();
  }

  saveData();

  setMinimalMoistureByUser();
  setWateringTimeByUser();
  setIntervalTimeByUser();

  watering();
}

void menu() {
  switch (counter) {
  case 0:
    lcd.setCursor(0, 1);
    lcd.print("Moisture: ");
    lcd.setCursor(10, 1);
    lcd.print(String(moisturePercent) + "%        ");
    menuState = 0;
    break;

  case 1:
    lcd.setCursor(0, 1);
    lcd.print(">min moisture     ");
    if (digitalRead(buttonFour) == LOW) {
      menuState = 1;
    }
    break;

  case 2:
    lcd.setCursor(0, 1);
    lcd.print(">watering time     ");
    if (digitalRead(buttonFour) == LOW) {
      menuState = 3;
    }
    break;

  case 3:
    lcd.setCursor(0, 1);
    lcd.print(">interval time      ");
    if (digitalRead(buttonFour) == LOW) {
      menuState = 5;
    }
    break;
  }
}

void setMinimalMoistureByUser() {
  if (digitalRead(buttonFour) == LOW && menuState == 1 && counter == 1) {
    isMenu = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set value:");
    lcd.setCursor(0, 1);
    lcd.print(String(moistureMin) + "%");
    menuState = 2;
  }

  while (digitalRead(buttonTwo) == LOW && menuState == 2) {
    isMenu = false;

    if (moistureMin > 100) {
      moistureMin = 100;
    }

    if (moistureMin < 100) {
      moistureMin += 1;
      delay(100);
    }

    if (moistureMin < 10) {
      lcd.setCursor(0, 1);
      lcd.print("0");
      lcd.setCursor(1, 1);
      lcd.print(moistureMin);
      lcd.setCursor(2, 1);
      lcd.print("%       ");
    }
    else {
      lcd.setCursor(0, 1);
      lcd.print(moistureMin);
      lcd.setCursor(2, 1);
      lcd.print("%       ");
    }

    if (moistureMin == 100) {
      lcd.setCursor(0, 1);
      lcd.print(moistureMin);
      lcd.setCursor(3, 1);
      lcd.print("%       ");
    }
    save = true;
  }

  while (digitalRead(buttonThree) == LOW && digitalRead(buttonTwo) == HIGH && menuState == 2) {
    isMenu = false;

    if (moistureMin > 0) {
      moistureMin -= 1;
      delay(100);
    }
    if (moistureMin < 10) {
      lcd.setCursor(0, 1);
      lcd.print("0");
      lcd.setCursor(1, 1);
      lcd.print(moistureMin);
      lcd.setCursor(2, 1);
      lcd.print("%   ");
    }
    else {
      lcd.setCursor(0, 1);
      lcd.print(moistureMin);
      lcd.setCursor(2, 1);
      lcd.print("%   ");
    }
    if (moistureMin == 100) {
      lcd.setCursor(0, 1);
      lcd.print(moistureMin);
      lcd.setCursor(3, 1);
      lcd.print("%   ");
    }

    save = true;
  }
}

void setWateringTimeByUser() {
  if (digitalRead(buttonFour) == LOW && menuState == 3 && counter == 2) {
    isMenu = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set value:");
    lcd.setCursor(0, 1);
    lcd.print(String(wateringMinutes) + ":" + String(wateringSeconds));

    if (wateringMinutes < 10) {
      lcd.setCursor(0, 1);
      lcd.print("0" + String(wateringMinutes) + ":" + String(wateringSeconds) + "      ");
    }

    if (wateringSeconds < 10) {
      lcd.setCursor(0, 1);
      lcd.print(String(wateringMinutes) + ":0" + String(wateringSeconds) + "        ");
    }

    if (wateringSeconds < 10 && wateringMinutes < 10) {
      lcd.setCursor(0, 1);
      lcd.print("0" + String(wateringMinutes) + ":0" + String(wateringSeconds) + "        ");
    }

    menuState = 4;
  }
  //Choose seconds or minutes to edit
  if (digitalRead(buttonOne) == LOW && menuState == 4) {
    counterWateringTime++;
    if (counterWateringTime > 1) {
      counterWateringTime = 0;
    }
    delay(100);
  }

  while (digitalRead(buttonTwo) == LOW && menuState == 4 && counter == 2) {
    isMenu = false;

    if (counterWateringTime == 0 && wateringMinutes < 60) {
      wateringMinutes += 1;
      delay(100);
    }
    if (counterWateringTime == 1 && wateringSeconds >= 0) {
      if (wateringSeconds == 60) {
        wateringSeconds = 0;
        wateringMinutes += 1;
      }
      wateringSeconds += 1;
      delay(100);
    }
    if (wateringMinutes < 10) {
      lcd.setCursor(0, 1);
      lcd.print("0");
      lcd.setCursor(1, 1);
      lcd.print(wateringMinutes);
    }
    else {
      lcd.setCursor(0, 1);
      lcd.print(wateringMinutes);
    }
    if (wateringSeconds < 10) {
      lcd.setCursor(3, 1);
      lcd.print("0");
      lcd.setCursor(4, 1);
      lcd.print(String(wateringSeconds) + "              ");
    }
    else {
      lcd.setCursor(3, 1);
      lcd.print(String(wateringSeconds) + "                            ");
    }
    delay(100);

    setWateringTime();
    save = true;
  }

  while (digitalRead(buttonThree) == LOW && digitalRead(buttonTwo) == HIGH && menuState == 4 && counter == 2) {
    isMenu = false;

    lcd.setCursor(2, 1);
    lcd.print(":");
    if (counterWateringTime == 0 && wateringMinutes > 0) {
      wateringMinutes -= 1;
      delay(100);
    }
    if (counterWateringTime == 1 && wateringMinutes >= 0 && wateringSeconds >= 0) {
      if (wateringSeconds == 0) {
        if (wateringMinutes > 0) {
          wateringMinutes -= 1;
        }
        wateringSeconds = 60;
      }
      wateringSeconds -= 1;
      delay(100);
    }
    if (wateringMinutes < 10) {
      lcd.setCursor(0, 1);
      lcd.print("0");
      lcd.setCursor(1, 1);
      lcd.print(wateringMinutes);
    }
    else {
      lcd.setCursor(0, 1);
      lcd.print(wateringMinutes);
    }
    if (wateringSeconds < 10) {
      lcd.setCursor(3, 1);
      lcd.print("0");
      lcd.setCursor(4, 1);
      lcd.print(String(wateringSeconds) + "                ");
    }
    else {
      lcd.setCursor(3, 1);
      lcd.print(String(wateringSeconds) + "                      ");
    }
    delay(100);
    setWateringTime();

    save = true;
  }
}

void setIntervalTimeByUser() {
  if (digitalRead(buttonFour) == LOW && menuState == 5 && counter == 3) {
    isMenu = false;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set value:");
    lcd.setCursor(0, 1);
    lcd.print(String(intervalMinutes) + ":" + String(intervalSeconds));

    if (intervalMinutes < 10) {
      lcd.setCursor(0, 1);
      lcd.print("0" + String(intervalMinutes) + ":" + String(intervalSeconds) + "      ");
    }

    if (intervalMinutes < 10) {
      lcd.setCursor(0, 1);
      lcd.print(String(intervalMinutes) + ":0" + String(intervalSeconds) + "        ");
    }

    if (intervalSeconds < 10 && intervalMinutes < 10) {
      lcd.setCursor(0, 1);
      lcd.print("0" + String(intervalMinutes) + ":0" + String(intervalSeconds) + "        ");
    }

    menuState = 6;
  }

  if (digitalRead(buttonOne) == LOW && menuState == 6) {
    counterIntervalTime++;
    if (counterIntervalTime > 1) {
      counterIntervalTime = 0;
    }
    delay(100);
  }

  while (digitalRead(buttonTwo) == LOW && menuState == 6 && counter == 3) {
    isMenu = false;

    lcd.setCursor(2, 1);
    lcd.print(":");
    if (counterIntervalTime == 0 && intervalMinutes >= 0 && intervalMinutes < 60) {
      intervalMinutes += 1;
      delay(100);
    }
    if (counterIntervalTime == 1 && intervalSeconds >= 0) {
      if (intervalSeconds == 60) {
        intervalSeconds = 0;
        intervalMinutes += 1;
      }
      intervalSeconds += 1;
      delay(100);
    }
    if (intervalMinutes < 10) {
      lcd.setCursor(0, 1);
      lcd.print("0");
      lcd.setCursor(1, 1);
      lcd.print(intervalMinutes);
    }
    else {
      lcd.setCursor(0, 1);
      lcd.print(intervalMinutes);
    }
    if (intervalSeconds < 10) {
      lcd.setCursor(3, 1);
      lcd.print("0");
      lcd.setCursor(4, 1);
      lcd.print(String(intervalSeconds) + "              ");
    }
    else {
      lcd.setCursor(3, 1);
      lcd.print(String(intervalSeconds) + "                            ");
    }
    delay(100);

    setIntervalTime();

    save = true;
  }

  while (digitalRead(buttonThree) == LOW && digitalRead(buttonTwo) == HIGH && menuState == 6 && counter == 3) {
    isMenu = false;

    lcd.setCursor(2, 1);
    lcd.print(":");
    if (counterIntervalTime == 0 && intervalMinutes > 0) {
      intervalMinutes -= 1;
      delay(100);
    }
    if (counterIntervalTime == 1 && intervalMinutes >= 0 && intervalSeconds >= 0) {
      if (intervalSeconds == 0) {
        if (intervalMinutes > 0) {
          intervalMinutes -= 1;
        }
        intervalSeconds = 60;
      }
      intervalSeconds -= 1;
      delay(100);
    }
    if (intervalMinutes < 10) {
      lcd.setCursor(0, 1);
      lcd.print("0");
      lcd.setCursor(1, 1);
      lcd.print(intervalMinutes);
    }
    else {
      lcd.setCursor(0, 1);
      lcd.print(intervalMinutes);
    }
    if (intervalSeconds < 10) {
      lcd.setCursor(3, 1);
      lcd.print("0");
      lcd.setCursor(4, 1);
      lcd.print(String(intervalSeconds) + "                ");
    }
    else {
      lcd.setCursor(3, 1);
      lcd.print(String(intervalSeconds) + "                      ");
    }
    delay(100);

    setIntervalTime();

    save = true;
  }
}

void watering() {
  if (moisturePercent < moistureMin && menuState == 0 && isMenu == true) {
    if (wateringTimer.available() == true && counterWatering == 0) {
      digitalWrite(pump, HIGH);
      intervalTimer.begin(intervalTime);
      counterWatering = 1;
    }
    if (intervalTimer.available() == true && counterWatering == 1) {
      digitalWrite(pump, LOW);
      wateringTimer.begin(wateringTime);
      counterWatering = 0;
    }
  }
  if (moisturePercent > moistureMin) {
    digitalWrite(pump, HIGH);
    counterWatering = 0;
  }
}

void saveData() {
  if (digitalRead(buttonFour) == LOW && save == true && (menuState == 2 || menuState == 4 || menuState == 6 || menuState == 8)) {
    lcd.clear();
    lcd.begin(16, 2);
    lcd.print("Welcome!");
    lcd.setCursor(0, 1);
    lcd.print("Status: saved");

    EEPROM.write(moistureMinID, moistureMin);
    EEPROM.write(wateringSecondsID, wateringSeconds);
    EEPROM.write(wateringMinutesID, wateringMinutes);
    EEPROM.write(intervalSecondsID, intervalSeconds);
    EEPROM.write(intervalMinutesID, intervalMinutes);

    wateringTimer.begin(wateringTime);
    intervalTimer.begin(intervalTime);

    delay(500);

    lcd.setCursor(0, 1);
    lcd.print("                    ");

    counter = 0;
    counterWateringTime = 0;
    menuState = 0;

    isMenu = true;
    save = false;
  }
}

void setWateringTime() {
  wateringTime = wateringMinutes * 60000 + wateringSeconds * 1000;
}

void setIntervalTime() {
  intervalTime = intervalMinutes * 60000 + intervalSeconds * 1000;
}
