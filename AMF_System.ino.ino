#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>

// ==================================================
// LCD SETUP
// ==================================================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ==================================================
// PIN CONNECTIONS
// ==================================================
const int RED_LED = 5;
const int YELLOW_LED = 6;
const int GREEN_LED = 7;

const int R2_GENERATOR = 8;
const int R1_MAIN = 9;

const int GENERATOR_SENSE = 10;
const int MAIN_SENSE = 11;
const int LOAD_SENSE = 12;  // Reserved, not used yet

// ==================================================
// SETTINGS
// ==================================================
// Active-LOW relay modules:
// LOW = ON
// HIGH = OFF

const int RELAY_ON = LOW;
const int RELAY_OFF = HIGH;

const unsigned long CHANGEOVER_TIME = 5000;
const unsigned long BREAK_TIME = 200;

// ==================================================
// SYSTEM VARIABLES
// ==================================================
bool changeoverActive = false;
bool generatorFault = false;

unsigned long changeoverStart = 0;
int previousCountdown = -1;

char lastLine1[17] = "";
char lastLine2[17] = "";

// ==================================================
// SUPPLY SENSING
// ==================================================
// D11 HIGH = main supply available
// D10 HIGH = generator supply available
// D12 HIGH = load voltage present

bool mainAvailable() {
  return digitalRead(MAIN_SENSE) == HIGH;
}

bool generatorAvailable() {
  return digitalRead(GENERATOR_SENSE) == HIGH;
}

bool loadVoltagePresent() {
  return digitalRead(LOAD_SENSE) == HIGH;
}

// ==================================================
// LCD DISPLAY
// ==================================================
void showScreen(const char* line1, const char* line2) {
  if (strcmp(line1, lastLine1) == 0 &&
      strcmp(line2, lastLine2) == 0) {
    return;
  }

  strncpy(lastLine1, line1, 16);
  lastLine1[16] = '\0';

  strncpy(lastLine2, line2, 16);
  lastLine2[16] = '\0';

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(line1);

  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void updateCountdown() {
  unsigned long elapsed = millis() - changeoverStart;

  int secondsLeft =
      (int)(CHANGEOVER_TIME / 1000) -
      (int)(elapsed / 1000);

  if (secondsLeft < 0) {
    secondsLeft = 0;
  }

  if (secondsLeft != previousCountdown) {
    previousCountdown = secondsLeft;

    lcd.setCursor(0, 1);
    lcd.print("Wait: ");
    lcd.print(secondsLeft);
    lcd.print(" seconds  ");

    lastLine2[0] = '\0';
  }
}

// ==================================================
// INDICATOR LEDs
// ==================================================
void indicatorsOff() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
}

void redOn() {
  indicatorsOff();
  digitalWrite(RED_LED, HIGH);
}

void yellowOn() {
  indicatorsOff();
  digitalWrite(YELLOW_LED, HIGH);
}

void greenOn() {
  indicatorsOff();
  digitalWrite(GREEN_LED, HIGH);
}

// ==================================================
// RELAY CONTROL
// ==================================================
void mainOff() {
  digitalWrite(R1_MAIN, RELAY_OFF);
}

void generatorOff() {
  digitalWrite(R2_GENERATOR, RELAY_OFF);
}

void bothSupplyRelaysOff() {
  mainOff();
  generatorOff();
}

// ==================================================
// SWITCH TO MAIN SUPPLY
// ==================================================
bool turnMainOn() {
  // Disconnect both supplies first
  bothSupplyRelaysOff();

  // Break-before-make delay
  delay(BREAK_TIME);

  // Verify main supply is still available
  if (!mainAvailable()) {
    return false;
  }

  // Connect main supply
  digitalWrite(R1_MAIN, RELAY_ON);

  redOn();

  changeoverActive = false;
  generatorFault = false;
  previousCountdown = -1;

  showScreen("MAIN SUPPLY ON", "R1 ON  R2 OFF");

  return true;
}

// ==================================================
// SWITCH TO GENERATOR SUPPLY
// ==================================================
bool turnGeneratorOn() {
  // Disconnect both supplies first
  bothSupplyRelaysOff();

  // Break-before-make delay
  delay(BREAK_TIME);

  // Verify generator supply is still available
  if (!generatorAvailable()) {
    return false;
  }

  // Connect generator supply
  digitalWrite(R2_GENERATOR, RELAY_ON);

  greenOn();

  changeoverActive = false;
  generatorFault = false;
  previousCountdown = -1;

  showScreen("GENERATOR ON", "R1 OFF R2 ON");

  return true;
}

// ==================================================
// START CHANGEOVER
// ==================================================
void startChangeover(bool toGenerator) {
  // Immediately disconnect both supplies
  bothSupplyRelaysOff();

  // Show changeover status
  yellowOn();

  changeoverActive = true;
  changeoverStart = millis();
  previousCountdown = -1;

  if (toGenerator) {
    showScreen("MAIN SUPPLY LOST", "Both relays OFF");
  } else {
    showScreen("RETURN TO MAIN", "Both relays OFF");
  }
}

// ==================================================
// FAULT / NO SUPPLY
// ==================================================
void showGeneratorFault() {
  bothSupplyRelaysOff();

  changeoverActive = false;
  generatorFault = true;

  indicatorsOff();

  showScreen("GENERATOR FAULT", "Load cut off");
}

void showNoSupply() {
  bothSupplyRelaysOff();

  indicatorsOff();
  changeoverActive = false;

  if (generatorFault) {
    showScreen("GENERATOR FAULT", "Load cut off");
  } else {
    showScreen("NO SUPPLY", "Both relays OFF");
  }
}

// ==================================================
// SETUP
// ==================================================
void setup() {
  // Set safe relay output levels before enabling outputs
  digitalWrite(R1_MAIN, RELAY_OFF);
  digitalWrite(R2_GENERATOR, RELAY_OFF);

  pinMode(R1_MAIN, OUTPUT);
  pinMode(R2_GENERATOR, OUTPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  pinMode(MAIN_SENSE, INPUT);
  pinMode(GENERATOR_SENSE, INPUT);
  pinMode(LOAD_SENSE, INPUT);

  bothSupplyRelaysOff();
  indicatorsOff();

  Wire.begin();
  delay(100);

  lcd.init();
  lcd.backlight();

  showScreen("AMF SYSTEM", "Initializing...");
  delay(1500);
}

// ==================================================
// MAIN LOOP
// ==================================================
void loop() {

  // ----------------------------------------------
  // 1. HANDLE ACTIVE CHANGEOVER
  // ----------------------------------------------
  if (changeoverActive) {

    bothSupplyRelaysOff();
    yellowOn();

    updateCountdown();

    if (millis() - changeoverStart >= CHANGEOVER_TIME) {

      changeoverActive = false;

      // Prefer main supply if it has returned
      if (mainAvailable()) {
        if (!turnMainOn()) {
          showNoSupply();
        }
      }

      // Otherwise, try the generator
      else if (generatorAvailable()) {
        if (!turnGeneratorOn()) {
          showGeneratorFault();
        }
      }

      // Neither supply is available
      else {
        showNoSupply();
      }
    }

    return;
  }

  // ----------------------------------------------
  // 2. MAIN RELAY IS ON
  // ----------------------------------------------
  if (digitalRead(R1_MAIN) == RELAY_ON) {

    generatorOff();

    if (!mainAvailable()) {
      startChangeover(true);
    }
  }

  // ----------------------------------------------
  // 3. GENERATOR RELAY IS ON
  // ----------------------------------------------
  else if (digitalRead(R2_GENERATOR) == RELAY_ON) {

    mainOff();

    if (!generatorAvailable()) {
      showGeneratorFault();
    }

    else if (mainAvailable()) {
      startChangeover(false);
    }
  }

  // ----------------------------------------------
  // 4. BOTH RELAYS ARE OFF
  // ----------------------------------------------
  else {

    if (mainAvailable()) {

      // Main supply has priority
      if (!turnMainOn()) {
        showNoSupply();
      }
    }

    else if (generatorAvailable()) {

      // Use generator if main supply is unavailable
      if (!turnGeneratorOn()) {
        showGeneratorFault();
      }
    }

    else {
      showNoSupply();
    }
  }
}
