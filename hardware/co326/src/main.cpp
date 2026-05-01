#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

// WiFi
const char* ssid = "Rashmika";
const char* password = "rashmikasew2001";

// MQTT
const char* mqtt_server = "broker.hivemq.com";

// Objects
WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_MPU6050 mpu;

// Relay pin
#define RELAY_PIN 5

// WiFi connect
void setup_wifi() {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    int attempts = 0;

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        attempts++;

        if (attempts > 20) {
            Serial.println("\n❌ Failed to connect!");
            return;
        }
    }

    Serial.println("\n✅ WiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

// MQTT reconnect
void reconnect() {
    while (!client.connected()) {
        Serial.print("Connecting MQTT...");
        if (client.connect("motorClient123")) {
            Serial.println("connected");
        } else {
            Serial.print("failed: ");
            Serial.println(client.state());
            delay(2000);
        }
    }
}

void setup() {
    Serial.begin(115200);

    // I2C
    Wire.begin(21, 19);

    // MPU
    if (!mpu.begin()) {
        Serial.println("MPU6050 not found!");
        while (1);
    }

    // Relay
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);

    // WiFi + MQTT
    setup_wifi();
    client.setServer(mqtt_server, 1883);

    Serial.println("System Ready!");
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float x = a.acceleration.x;
    float y = a.acceleration.y;
    float z = a.acceleration.z;

    float magnitude = sqrt(x*x + y*y + z*z);

    Serial.print("Vibration: ");
    Serial.println(magnitude);

    float threshold = 12.0;

    // Publish vibration
    char msg[50];
    sprintf(msg, "%.2f", magnitude);
    client.publish("factory/motor01/vibration", msg);

    // Control + publish status
    if (magnitude > threshold) {
        digitalWrite(RELAY_PIN, LOW);
        client.publish("factory/motor01/status", "ANOMALY");
    } else {
        digitalWrite(RELAY_PIN, HIGH);
        client.publish("factory/motor01/status", "NORMAL");
    }

    delay(500);
}