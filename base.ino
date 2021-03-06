//#define BLYNK_DEBUG
#define BLYNK_PRINT Serial

#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>

// pins
#define TAP1 14 // D5
#define TAP2 12 // D6
#define TAP3 13 // D7
#define TAP4 4  // D2

// vpins
#define vPIN_MOISTURE1 V1
#define vPIN_MOISTURE2 V2
#define vPIN_MOISTURE3 V3

#define vPIN_SENS1_BRIDGE V4
#define vPIN_SENS2_BRIDGE V5
#define vPIN_SENS3_BRIDGE V6
#define vPIN_TAP1_LED V7
#define vPIN_TAP2_LED V8
#define vPIN_TAP3_LED V9
#define vPIN_TAP4_LED V10
#define vPIN_TAP1_THRESHOLD V11
#define vPIN_TAP2_THRESHOLD V12
#define vPIN_TAP3_THRESHOLD V13
#define vPIN_TAP1_TIMEOUT V14
#define vPIN_TAP2_TIMEOUT V15
#define vPIN_TAP3_TIMEOUT V16
#define vPIN_WATER_TOTAL V17

SimpleTimer timer;
WidgetBridge sensor1(vPIN_SENS1_BRIDGE);
WidgetBridge sensor2(vPIN_SENS2_BRIDGE);


char auth[] = "xxxxxxxxxxxxxxxxxxx";

char auth_sensor1[] = "xxxxxxxxxxxxxxxxxxxxxxx";
char auth_sensor2[] = "xxxxxxxxxxxxxxxxxxxxxxx";

char ssid[] = "xxxxxxxxxxxxxxxx";
char pass[] = "xxxxxxxxxxxxxxxx";

int tap1_threshold_value, tap2_threshold_value, tap3_threshold_value;
int tap1_timeout_value,   tap2_timeout_value,   tap3_timeout_value;

byte sensorInterrupt = 15;  // 0 = digital pin 2
//byte sensorPin       = 15;
// The hall-effect flow sensor outputs approximately 4.5 pulses per second per litre/minute of flow.
float calibrationFactor = 4.5;
volatile byte pulseCount;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned long oldTime;

bool isFirstConnect = true;

void GetFlowSensorData() {
  detachInterrupt(sensorInterrupt);
  flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
  oldTime = millis();
  flowMilliLitres = (flowRate / 60) * 1000;
  totalMilliLitres += flowMilliLitres;
  unsigned int frac;
  frac = (flowRate - int(flowRate)) * 10;
  pulseCount = 0;
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  //Serial.print("totalMilliLitres=");
  //Serial.println(totalMilliLitres);
  Blynk.virtualWrite(vPIN_WATER_TOTAL, (float)totalMilliLitres / 1000);
}

void pulseCounter() {
  pulseCount++;
}

BLYNK_CONNECTED() {
  sensor1.setAuthToken(auth_sensor1);
  sensor2.setAuthToken(auth_sensor2);
  Serial.println("Syncing on connect..");
  if (!tap1_threshold_value || !tap1_timeout_value) {
    Blynk.syncVirtual(vPIN_TAP1_THRESHOLD, vPIN_TAP1_TIMEOUT);
  }
  //Blynk.syncVirtual(vPIN_TAP2_THRESHOLD);
  //Blynk.syncVirtual(vPIN_TAP3_THRESHOLD);
  //Blynk.syncVirtual(vPIN_TAP2_TIMEOUT);
  //Blynk.syncVirtual(vPIN_TAP3_TIMEOUT);
  if (!totalMilliLitres) {
    //Blynk.syncVirtual(vPIN_WATER_TOTAL);
  }
}

BLYNK_WRITE(vPIN_MOISTURE1) {
  int moisture1 = param.asInt();
  Serial.print("Moisture Sensor1=");
  Serial.println(moisture1);

  if (moisture1 <= tap1_threshold_value && digitalRead(TAP1)) {
    digitalWrite(TAP1, LOW); // switch on tap1
    Blynk.virtualWrite(vPIN_TAP1_LED, 255);
    Serial.println("TAP1=ON");
    timer.setTimeout(tap1_timeout_value, TAP1_Off);
  }
}

void TAP1_Off() {
  digitalWrite(TAP1, HIGH); // switch on tap1
  Blynk.virtualWrite(vPIN_TAP1_LED, 0);
  Serial.println("TAP1=OFF");
}

BLYNK_WRITE(vPIN_MOISTURE2) {
  int moisture2 = param.asInt();
  Serial.print("Moisture Sensor2=");
  Serial.println(moisture2);

  if (moisture2 <= tap2_threshold_value) {
    digitalWrite(TAP2, LOW); // switch on tap2
    Blynk.virtualWrite(vPIN_TAP2_LED, 255);
    Serial.println("TAP2=ON");
    timer.setTimeout(tap2_timeout_value, TAP2_Off);
  }
}

void TAP2_Off() {
  digitalWrite(TAP2, HIGH); // switch on tap1
  Blynk.virtualWrite(vPIN_TAP2_LED, 0);
  Serial.println("TAP2=OFF");
}

BLYNK_WRITE(vPIN_MOISTURE3) {
  int moisture3 = param.asInt();
  Serial.print("Moisture Sensor3=");
  Serial.println(moisture3);

  if (moisture3 <= tap3_threshold_value) {
    digitalWrite(TAP3, LOW); // switch on tap3
    Blynk.virtualWrite(vPIN_TAP3_LED, 255);
    Serial.println("TAP3=ON");
    timer.setTimeout(tap3_timeout_value, TAP3_Off);
  }
}

void TAP3_Off() {
  digitalWrite(TAP3, HIGH); // switch on tap1
  Blynk.virtualWrite(vPIN_TAP3_LED, 0);
  Serial.println("TAP3=OFF");

}

BLYNK_WRITE(vPIN_TAP1_THRESHOLD) {
  tap1_threshold_value = param.asInt();
  Serial.print("TAP1 THRESHOLD=");
  Serial.println(tap1_threshold_value);
}

BLYNK_WRITE(vPIN_TAP2_THRESHOLD) {
  tap2_threshold_value = param.asInt();
  Serial.print("TAP2 THRESHOLD=");
  Serial.println(tap2_threshold_value);
}

BLYNK_WRITE(vPIN_TAP3_THRESHOLD) {
  tap3_threshold_value = param.asInt();
  Serial.print("TAP3 THRESHOLD=");
  Serial.println(tap3_threshold_value);
}

BLYNK_WRITE(vPIN_TAP1_TIMEOUT) {
  tap1_timeout_value = param.asInt() * 60000;
  Serial.print("TAP1 TIMEOUT=");
  Serial.println(tap1_timeout_value);
}

BLYNK_WRITE(vPIN_TAP2_TIMEOUT) {
  tap2_timeout_value = param.asInt() * 60000;
  Serial.print("TAP2 TIMEOUT=");
  Serial.println(tap2_timeout_value);
}

BLYNK_WRITE(vPIN_TAP3_TIMEOUT) {
  tap3_timeout_value = param.asInt() * 60000;
  Serial.print("TAP3 TIMEOUT=");
  Serial.println(tap3_timeout_value);
}

void setup() {
  // PINMODS
  pinMode(TAP1, OUTPUT);
  pinMode(TAP2, OUTPUT);
  pinMode(TAP3, OUTPUT);
  pinMode(TAP4, OUTPUT);
  // SET RELAYS OFF
  digitalWrite(TAP1, HIGH);
  digitalWrite(TAP2, HIGH);
  digitalWrite(TAP3, HIGH);
  digitalWrite(TAP4, HIGH);
  // Set up Flow Meter
  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Blynk.begin(auth, ssid, pass);
  while (Blynk.connect() == false) {}
  ArduinoOTA.setHostname("Garduino-Base");
  ArduinoOTA.begin();


  timer.setInterval(1000L, GetFlowSensorData);
}

void loop() {
  Blynk.run();
  ArduinoOTA.handle();
  timer.run();
}
