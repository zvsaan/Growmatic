#include <WiFi.h>
#include <ThingerESP32.h>
#include <DHT.h>

// ====== Thinger.io Configuration ======
#define USERNAME "subektibimowicaksono"
#define DEVICE_ID "grow_matic"
#define DEVICE_CREDENTIAL "mFD$II9S8T7p2KUs"

// Initialize Thinger.io
ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

// Pin DHT22 and Relay
#define DHTPIN 4
#define RELAY_PIN 5
#define DHTTYPE DHT22

// Initialize DHT
DHT dht(DHTPIN, DHTTYPE);

// WiFi credentials
const char* ssid = "Kost gudang belakang";
const char* password = "herimuktisubroto";

// Variables for control
bool isAutoMode = true;
bool relayState = false;

void setup() {
  Serial.begin(115200);
  
  // Initialize relay pin
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Turn off relay initially
  
  // Initialize DHT sensor
  dht.begin();
  
  // Connect to WiFi
  thing.add_wifi(ssid, password);
  
  // Define resources in Thinger.io
  thing["sensor"] >> [](pson& out){
    out["temperature"] = dht.readTemperature();
    out["humidity"] = dht.readHumidity();
  };
  
  // Control relay from Thinger.io
  thing["relay"] << [](pson& in){
    if(in.is_empty()){
      in = (bool)relayState;
    }
    else{
      relayState = in;
      digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
      isAutoMode = false; // switch to manual mode when controlled from dashboard
    }
  };
  
  // Control mode from Thinger.io
  thing["mode"] << [](pson& in){
    if(in.is_empty()){
      in = (bool)isAutoMode;
    }
    else{
      isAutoMode = in;
    }
  };
}

void loop() {
  thing.handle();
  
  // Read and send sensor data every 2 seconds
  static unsigned long lastSent = 0;
  if(millis() - lastSent > 2000){
    lastSent = millis();
    
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    
    if(!isnan(t) && !isnan(h)){
      // Data will be automatically sent to Thinger.io when accessed
      thing.stream(thing["sensor"]);
      
      // Log to Serial Monitor
      Serial.print("Suhu: ");
      Serial.print(t);
      Serial.print(" °C, Kelembaban: ");
      Serial.print(h);
      Serial.println(" %");
    }
    else {
      Serial.println("Gagal membaca DHT22!");
    }
    
    // Automatic control if in auto mode
    if(isAutoMode){
      if(t > 30){  // If temperature > 30°C, turn on relay
        relayState = true;
        digitalWrite(RELAY_PIN, HIGH);
      }
      else {       // If temperature <= 30°C, turn off relay
        relayState = false;
        digitalWrite(RELAY_PIN, LOW);
      }
    }
    
    // Log relay state
    Serial.print("Relay: ");
    Serial.println(relayState ? "ON" : "OFF");
    Serial.print("Mode: ");
    Serial.println(isAutoMode ? "AUTO" : "MANUAL");
  }
}