#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <WiFi.h>
#include <WebServer.h>

// ================= WIFI =================
const char* ssid = "ESPTEST";
const char* password = "00011100";
WebServer server(80);

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= DHT11 =================
#define DHTPIN 27
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ================= GAS / SMOKE =================
#define MQ135_PIN 35
#define MQ2_PIN   32

// ================= RELAY (via MOSFET) =================
#define RELAY_PIN 33   // HIGH = FAN ON

// ================= THRESHOLDS =================
#define TEMP_THRESHOLD    35.0
#define HUM_THRESHOLD     75.0
#define GAS_THRESHOLD     1200
#define SMOKE_THRESHOLD   1500

// ================= GLOBAL DATA =================
float temp, hum;
int gas, smoke;
bool fanRequired = false;

// ================= WEB PAGE =================
void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta http-equiv='refresh' content='2'>";
  html += "<title>Air Quality Monitoring</title>";
  html += "<style>";
  html += "body{font-family:Arial;text-align:center;background:#f2f2f2;}";
  html += ".box{background:#fff;padding:20px;margin:20px;border-radius:10px;}";
  html += "</style></head><body>";

  html += "<h1>EV Air Quality Monitoring</h1>";
  html += "<div class='box'>";
  html += "<p><b>Temperature:</b> " + String(temp) + " °C</p>";
  html += "<p><b>Humidity:</b> " + String(hum) + " %</p>";
  html += "<p><b>Gas Level:</b> " + String(gas) + "</p>";
  html += "<p><b>Smoke Level:</b> " + String(smoke) + "</p>";
  html += "<p><b>Fan Status:</b> " + String(fanRequired ? "ON" : "OFF") + "</p>";
  html += "</div>";

  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);

  // I2C
  Wire.begin(21, 22);

  // OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Sensors
  dht.begin();

  // Relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // WIFI CONNECT
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.println(WiFi.localIP());

  // WEB SERVER
  server.on("/", handleRoot);
  server.begin();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi Connected");
  display.println(WiFi.localIP());
  display.display();

  delay(3000);
}

void loop() {
  server.handleClient();

  // ===== READ SENSORS =====
  temp = dht.readTemperature();
  hum  = dht.readHumidity();
  gas = analogRead(MQ135_PIN);
  smoke = analogRead(MQ2_PIN);

  // ===== LOGIC =====
  fanRequired = false;
  if (temp > TEMP_THRESHOLD) fanRequired = true;
  if (hum > HUM_THRESHOLD) fanRequired = true;
  if (gas > GAS_THRESHOLD) fanRequired = true;
  if (smoke > SMOKE_THRESHOLD) fanRequired = true;

  digitalWrite(RELAY_PIN, fanRequired ? HIGH : LOW);

  // ===== OLED =====
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Temp: "); display.println(temp);
  display.print("Hum : "); display.println(hum);
  display.print("Gas : "); display.println(gas);
  display.print("Smoke: "); display.println(smoke);
  display.print("Fan : "); display.println(fanRequired ? "ON" : "OFF");
  display.display();

  delay(2000);
}
