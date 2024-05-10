#include <WiFi.h>
#include "DHT.h"
#include <ESPAsyncWebServer.h>
#include <LiquidCrystal.h>
#include <MQ135.h>

#define ANALOGPIN 32    // Define Analog PIN on Arduino Board
#define DPIN 33         // DHT sensor connected to GPIO 33
#define DTYPE DHT11     // DHT type is DHT11
#define FAN_PIN 2       // Fan control pin
#define Relay 27        // Relay control pin
MQ135 mq135_sensor = MQ135(ANALOGPIN);

DHT dht(DPIN, DTYPE);
const char *ssid = "YourWiFiSSID";
const char *password = "YourWiFiPassword";

AsyncWebServer server(80);
LiquidCrystal lcd(19, 23, 18, 17, 16, 15); // RS, EN, D4, D5, D6, D7

String getHTML(float temperature, float humidity, int fanState, float ppmValue) {
  String html = "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Fan Control</title>";
  html += "<style>";
  // Add your CSS styles here
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<div class='container'>";
  html += "<h1>Fan Control</h1>";
  html += "<p>Temperature: " + String(temperature) + " C</p>";
  html += "<p>Humidity: " + String(humidity) + " %</p>";
  html += "<p>PPM Value: " + String(ppmValue) + " ppm</p>";
  html += "<p>Fan state: <span class='status'>";

  if (fanState == LOW)
    html += "OFF";
  else
    html += "ON";

  html += "</span></p>";
  html += "<a href='/fan/on' class='btn'>Turn ON Fan</a>";
  html += "<br><br>";
  html += "<a href='/fan/off' class='btn'>Turn OFF Fan</a>";
  html +="<br><br><hr>";

  html += "</div>";
  html += "</body>";
  html += "</html>";

  return html;
}

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(Relay, OUTPUT);
  digitalWrite(Relay, LOW);
  digitalWrite(FAN_PIN, LOW);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("ESP32 Web Server's IP address: ");
  Serial.println(WiFi.localIP());

  // Home page
  server.on("/", HTTP_GET, [] (AsyncWebServerRequest *request) {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    float ppmValue = mq135_sensor.getCorrectedPPM(temperature, humidity);
    request->send(200, "text/html", getHTML(temperature, humidity, digitalRead(FAN_PIN), ppmValue));
  });

  // Fan ON
  server.on("/fan/on", HTTP_GET, [] (AsyncWebServerRequest *request) {
    digitalWrite(FAN_PIN, HIGH);
    digitalWrite(Relay, HIGH);
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    float ppmValue = mq135_sensor.getCorrectedPPM(temperature, humidity);
    request->send(200, "text/html", getHTML(temperature, humidity, HIGH, ppmValue));
  });

  // Fan OFF
  server.on("/fan/off", HTTP_GET, [] (AsyncWebServerRequest *request) {
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(Relay, LOW);
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    float ppmValue = mq135_sensor.getCorrectedPPM(temperature, humidity);
    request->send(200, "text/html", getHTML(temperature, humidity, LOW, ppmValue));
  });

  server.begin();
}

void loop() {
  // Nothing to add here for now
}
