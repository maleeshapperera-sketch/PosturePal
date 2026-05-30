#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <MPU9250_WE.h>
#include <Wire.h>
#include <WiFiManager.h> 

#define MPU9250_ADDR 0x68
#define BUZZER_PIN 4

unsigned long prevTime = 0;
const long buzzDuration = 5000;
bool goodPosture = true;
bool alertDispatched = true;
unsigned long currentTime = 0;
bool continuousBuzz = false;
const long alertDelay = 20000;
unsigned long lastMqttDispatchTime = 0;
const long mqttDispatchInterval = 30000;
const long pauseTime = 5000;
const long buzzTime = 3000;

bool checkingPosture = false;
unsigned long checkStartTime = 0;
const unsigned long checkDuration = 3000;


MPU9250_WE imuSensor = MPU9250_WE(MPU9250_ADDR);

bool motionDetectionActive = false;
unsigned long motionDetectionStart = 0;
unsigned long idleThreshold = 1 * 60 * 1000;
bool idleAlertDispatched = false;

int slouchCounter = 0;

const char* wifiSSID = "OnePlus 6T";
const char* wifiPass = "12345678";

const char* brokerAddress = "218638469dfa429db85a2e1df0b4f8c7.s1.eu.hivemq.cloud";
const int brokerPort = 8883;
const char* brokerUser = "hivemq.webclient.1742574310428";
const char* brokerPass = "61b!5CgSPQc>xqu.3@JM";
const char* mqttTopic = "alert/posture";

WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);


void initWiFi() {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(wifiSSID, wifiPass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println(" Connected!");
}

void initMQTT() {
    mqttClient.setServer(brokerAddress, brokerPort);
    while (!mqttClient.connected()) {
        Serial.print("Connecting to MQTT...");
        if (mqttClient.connect("ESP32Device", brokerUser, brokerPass)) {
            Serial.println(" Connected!");
        } else {
            Serial.print(" Failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" Retrying in 5 seconds...");
            delay(5000);
        }
    }
}


void setup() {
    delay(500);
    Serial.begin(115200);
    Wire.begin(21, 22);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    startWiFiManager();

    initMQTT();
    delay(2000);

    if (!imuSensor.init()) {
      Serial.println("MPU9250 not detected! Continuing without MPU...");
    } else {
      Serial.println("MPU9250 detected!");
    }
    Serial.println("Calibrating MPU9250... Keep the sensor flat.");
    delay(2000);
    imuSensor.autoOffsets();
    Serial.println("Calibration Done!");

    imuSensor.setAccRange(MPU9250_ACC_RANGE_2G);
    imuSensor.enableAccDLPF(true);
    imuSensor.setAccDLPF(MPU9250_DLPF_6);
    
    imuSensor.setGyrRange(MPU9250_GYRO_RANGE_250);
    imuSensor.enableGyrDLPF();
    imuSensor.setGyrDLPF(MPU9250_DLPF_6);
}

void loop() {
  unsigned long nowMillis = millis();

  ensureMQTTConnected();
  mqttClient.loop();

  xyzFloat tiltAngle = fetchAngles();
  evaluatePosture(tiltAngle);  
  scanForMovement();
  delay(1000);
}


void evaluatePosture(xyzFloat tiltAngle) {
  float zAxis = tiltAngle.z;

  if (zAxis < 70 && goodPosture) {
    goodPosture = false;
    currentTime = millis();
    
  }

  else if (zAxis < 70 && !goodPosture &&
           millis() - currentTime > buzzDuration &&
           millis() - currentTime < buzzDuration + buzzTime) {
    activateBuzzer();
    if (!alertDispatched) {
      publishAlert("Posture Alert! Incorrect Posture Detected", zAxis);
      alertDispatched = true;
      slouchCounter++;  
      Serial.print("Slouch Count: ");
      Serial.println(slouchCounter);
    }
   
  }

  else if (zAxis < 70 && !goodPosture &&
           millis() - currentTime > alertDelay) {
    activateBuzzer();
    checkingPosture = false;
    if (!alertDispatched) {
      publishAlert("Posture Alert! Continued Incorrect Posture", zAxis);
      alertDispatched = true;
    }
  }

  else if (zAxis > 70 && !goodPosture) {
    confirmPosture(zAxis);
  }

  else {
    deactivateBuzzer();
    alertDispatched = false;
  }
}


void activateBuzzer(){
  digitalWrite(BUZZER_PIN, HIGH);
}

void deactivateBuzzer(){
  digitalWrite(BUZZER_PIN, LOW);
}

void confirmPosture(float zAxis) {
  if (!checkingPosture) {
    checkStartTime = millis();
    checkingPosture = true;
    deactivateBuzzer();
  }

  if (zAxis < 70) {
    checkingPosture = false;
  } else if (millis() - checkStartTime >= checkDuration) {
    goodPosture = true;
    checkingPosture = false;
    publishAlert("Posture Corrected", zAxis);
    alertDispatched = false;
    
  }
}


void publishAlert(String alertMsg, float tiltValue) {
  StaticJsonDocument<200> jsonPayload;
  jsonPayload["message"] = alertMsg;
  jsonPayload["angle"] = tiltValue;

  char payloadBuffer[256];
  serializeJson(jsonPayload, payloadBuffer);

  mqttClient.publish(mqttTopic, payloadBuffer);
  Serial.println("MQTT Sent: " + String(payloadBuffer));
}

xyzFloat fetchAngles() {
  xyzFloat tiltAngle = imuSensor.getAngles();
  Serial.print("Angle Z = ");
  Serial.println(tiltAngle.z);
  return tiltAngle;
}

void ensureMQTTConnected() {
  if (!mqttClient.connected()) {
    initMQTT();
  }
}

void scanForMovement(){
  if (!motionDetectionActive){
    motionDetectionActive = true;
    motionDetectionStart = millis();
  }
  xyzFloat gyroData = imuSensor.getGyrValues();
  float gyrZ = gyroData.z;
  float gyrY = gyroData.y;
  float gyrX = gyroData.x;

  Serial.print("Gyro X = ");
  Serial.print(gyroData.x, 2);
  Serial.print(" | Y = ");
  Serial.print(gyroData.y, 2);
  Serial.print(" | Z = ");
  Serial.println(gyroData.z, 2);

  float gyroTotal = sqrt(gyroData.x * gyroData.x + gyroData.y * gyroData.y + gyroData.z * gyroData.z);

  if (gyroTotal > 15.0 && abs(gyrX) > 15) {
      Serial.println("Movement Detected");
      motionDetectionActive = false;
      idleAlertDispatched = false;
  }

  if (millis() - motionDetectionStart > idleThreshold){
    if (!idleAlertDispatched){
      publishAlert("Idle for too long", 10);
      idleAlertDispatched = true;
      motionDetectionActive = false;
    }
  }
}

void startWiFiManager() {
  WiFi.mode(WIFI_STA);

  WiFiManager wifiMgr;

  bool result = wifiMgr.autoConnect("PostureMonitorAP", "123455678");

  if (!result) {
    Serial.println("❌ Failed to connect");
  } else {
    Serial.println("✅ Connected to WiFi!");
    secureClient.setInsecure();
  }
}