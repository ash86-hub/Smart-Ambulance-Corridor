#include <Arduino.h>

// Each row lists NS red, NS green, EW red, EW green.
const uint8_t signalPins[2][4] = {{4, 5, 6, 7}, {A1, A2, A3, A4}};
const uint8_t ambulancePins[] = {8, 9};
const uint8_t hospitalPins[] = {10, 12};
const uint8_t buttonPins[] = {2, 3, A0};  // SOS1, SOS2, NEXT
const uint8_t buzzerPin = 11;
const unsigned long clearTime = 2000;

enum Signal { ALL_RED, NS_GREEN, EW_GREEN };
enum Stage { NORMAL, CLEAR_FOR_PICKUP, TO_PATIENT, CLEAR_FOR_HOSPITAL, TO_HOSPITAL };

Stage stage = NORMAL;
int active = -1;
int waiting = -1;
int displayedSignal[] = {-1, -1};
unsigned long stageStarted = 0;
unsigned long normalStarted[2] = {0, 0};

int lastReading[] = {HIGH, HIGH, HIGH};
int stableReading[] = {HIGH, HIGH, HIGH};
unsigned long readingChanged[3] = {0, 0, 0};

bool pressed(uint8_t button, unsigned long now) {
  int reading = digitalRead(buttonPins[button]);
  if (reading != lastReading[button]) {
    lastReading[button] = reading;
    readingChanged[button] = now;
  }
  if (reading != stableReading[button] && now - readingChanged[button] >= 40) {
    stableReading[button] = reading;
    return reading == LOW;
  }
  return false;
}

void setSignal(uint8_t zone, Signal signal) {
  if (displayedSignal[zone] == signal) return;
  const uint8_t *pins = signalPins[zone];

  // Switch off both greens before enabling either direction.
  digitalWrite(pins[1], LOW);
  digitalWrite(pins[3], LOW);
  digitalWrite(pins[0], HIGH);
  digitalWrite(pins[2], HIGH);

  if (signal == NS_GREEN) {
    digitalWrite(pins[0], LOW);
    digitalWrite(pins[1], HIGH);
  } else if (signal == EW_GREEN) {
    digitalWrite(pins[2], LOW);
    digitalWrite(pins[3], HIGH);
  }
  displayedSignal[zone] = signal;
}

Signal normalSignal(unsigned long elapsed) {
  unsigned long phase = elapsed % 14000UL;
  if (phase < 2000) return ALL_RED;
  if (phase < 7000) return NS_GREEN;
  if (phase < 9000) return ALL_RED;
  return EW_GREEN;
}

void startEmergency(int id, unsigned long now) {
  active = id;
  stage = CLEAR_FOR_PICKUP;
  stageStarted = now;
  setSignal(id, ALL_RED);
  digitalWrite(ambulancePins[id], HIGH);
  tone(buzzerPin, 1200, 600);
}

void requestEmergency(int id, unsigned long now) {
  if (id == active || id == waiting) return;
  if (active < 0) {
    startEmergency(id, now);
  } else {
    waiting = id;
    tone(buzzerPin, 1600, 150);
  }
}

void finishEmergency(unsigned long now) {
  digitalWrite(ambulancePins[active], LOW);
  digitalWrite(hospitalPins[active], LOW);
  setSignal(active, ALL_RED);
  normalStarted[active] = now;
  active = -1;
  stage = NORMAL;

  if (waiting >= 0) {
    int nextEmergency = waiting;
    waiting = -1;
    startEmergency(nextEmergency, now);
  } else {
    tone(buzzerPin, 900, 500);
  }
}

void setup() {
  for (uint8_t button = 0; button < 3; ++button) {
    pinMode(buttonPins[button], INPUT_PULLUP);
  }
  pinMode(buzzerPin, OUTPUT);
  for (uint8_t zone = 0; zone < 2; ++zone) {
    for (uint8_t light = 0; light < 4; ++light) {
      pinMode(signalPins[zone][light], OUTPUT);
    }
    pinMode(ambulancePins[zone], OUTPUT);
    pinMode(hospitalPins[zone], OUTPUT);
    digitalWrite(ambulancePins[zone], LOW);
    digitalWrite(hospitalPins[zone], LOW);
    setSignal(zone, ALL_RED);
    normalStarted[zone] = millis();
  }
}

void loop() {
  unsigned long now = millis();
  bool sos1 = pressed(0, now);
  bool sos2 = pressed(1, now);
  bool next = pressed(2, now);

  // SOS1 takes priority if both presses are detected on the same pass.
  if (sos1) requestEmergency(0, now);
  if (sos2) requestEmergency(1, now);

  // NEXT is accepted only while the ambulance is travelling.
  if (next && stage == TO_PATIENT) {
    digitalWrite(hospitalPins[active], HIGH);
    tone(buzzerPin, 1800, 300);
    stage = CLEAR_FOR_HOSPITAL;
    stageStarted = now;
    setSignal(active, ALL_RED);
  } else if (next && stage == TO_HOSPITAL) {
    finishEmergency(now);
  }

  if (now - stageStarted >= clearTime) {
    if (stage == CLEAR_FOR_PICKUP) stage = TO_PATIENT;
    if (stage == CLEAR_FOR_HOSPITAL) stage = TO_HOSPITAL;
  }

  if (waiting >= 0) {
    digitalWrite(ambulancePins[waiting], now % 1000UL < 500 ? HIGH : LOW);
  }

  for (uint8_t zone = 0; zone < 2; ++zone) {
    if (zone != active) {
      setSignal(zone, normalSignal(now - normalStarted[zone]));
    } else if (stage == TO_PATIENT) {
      setSignal(zone, EW_GREEN);
    } else if (stage == TO_HOSPITAL) {
      setSignal(zone, active == 1 ? NS_GREEN : EW_GREEN);
    } else {
      setSignal(zone, ALL_RED);
    }
  }
}
