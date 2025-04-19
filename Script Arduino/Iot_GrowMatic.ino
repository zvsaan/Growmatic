#include <WiFi.h>
#include <ThingerESP32.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Thinger.io Setup
#define USERNAME "subektibimowicaksono"
#define DEVICE_ID "grow_matic"
#define DEVICE_CREDENTIAL "mFD$II9S8T7p2KUs"

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

// DHT Sensor
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Relay
#define RELAY_PIN 5

// LCD Setup (Alamat default biasanya 0x27 atau 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2); // Alamat LCD, jumlah kolom, jumlah baris

// WiFi
const char* ssid = "Kost gudang belakang";
const char* password = "herimuktisubroto";

// Variables
bool isAutoMode = true;
bool relayState = false;
float suhu = 0.0;
float kelembaban = 0.0;

void setup() {
  Serial.begin(115200);

  // Relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // Sensor dan WiFi
  dht.begin();
  thing.add_wifi(ssid, password);

  // LCD
  lcd.init();
  lcd.backlight();

  // Thinger resources
  thing["sensor"] >> [](pson &out) {
    out["temperature"] = suhu;
    out["humidity"] = kelembaban;
  };

  thing["relay"] << [](pson &in) {
    if (in.is_empty()) {
      in = (bool)relayState;
    } else {
      relayState = in;
      digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
      isAutoMode = false;
    }
  };

  thing["mode"] << [](pson &in) {
    if (in.is_empty()) {
      in = (bool)isAutoMode;
    } else {
      isAutoMode = in;
    }
  };

  // Welcome Message
  lcd.setCursor(0, 0);
  lcd.print("  GROW-MATIC  ");
  delay(2000);
  lcd.clear();
}

void loop() {
  thing.handle();

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 2000) {
    lastUpdate = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      suhu = t;
      kelembaban = h;

      thing.stream(thing["sensor"]);

      // Log ke Serial
      Serial.print("Suhu: ");
      Serial.print(suhu);
      Serial.print(" °C, Kelembaban: ");
      Serial.print(kelembaban);
      Serial.println(" %");

      // Tampilkan di LCD
      lcd.setCursor(0, 0);
      lcd.print("T:");
      lcd.print(suhu, 1);
      lcd.print((char)223); // Derajat
      lcd.print("C H:");
      lcd.print(kelembaban, 0);
      lcd.print("%");

      lcd.setCursor(0, 1);
      lcd.print(isAutoMode ? "AUTO " : "MANUAL ");
      lcd.print("Relay:");
      lcd.print(relayState ? "ON " : "OFF");
    } else {
      Serial.println("⚠ Gagal baca DHT!");
      lcd.setCursor(0, 0);
      lcd.print("Gagal baca DHT  ");
    }

    // Otomatis
    if (isAutoMode) {
      if (suhu > 30) {
        relayState = true;
      } else {
        relayState = false;
      }
      digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
    }

    Serial.print("Relay: ");
    Serial.println(relayState ? "ON" : "OFF");
    Serial.print("Mode: ");
    Serial.println(isAutoMode ? "AUTO" : "MANUAL");
    Serial.println("=========================");
  }
}