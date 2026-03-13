#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// Wi-Fi credentials – replace with your network details
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// DHT11 settings
#define DHTPIN 4          // GPIO pin connected to DHT11 data
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MQ-2 settings
#define MQ2_ANALOG_PIN 34 // GPIO for analog output (ADC)
#define MQ2_DIGITAL_PIN 15 // GPIO for digital output (optional)

// Create web server object on port 80
WebServer server(80);

// Variables to hold sensor readings
float temperature = 0;
float humidity = 0;
int mq2Analog = 0;
float mq2Voltage = 0;
int mq2Digital = 0;

// HTML page (with meta refresh every 5 seconds)
String getHTML() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='5'>"; // auto-refresh page every 5s
  html += "<title>ESP32 Sensor Data</title>";
  html += "<style>body{font-family:Arial;margin:40px;text-align:center;}</style>";
  html += "</head><body>";
  html += "<h1>ESP32 Sensor Readings</h1>";
  html += "<p><strong>Temperature:</strong> " + String(temperature) + " °C</p>";
  html += "<p><strong>Humidity:</strong> " + String(humidity) + " %</p>";
  html += "<p><strong>MQ-2 Analog:</strong> " + String(mq2Analog) + " (0-4095)</p>";
  html += "<p><strong>MQ-2 Voltage:</strong> " + String(mq2Voltage, 2) + " V</p>";
  html += "<p><strong>MQ-2 Digital:</strong> " + String(mq2Digital ? "HIGH (gas detected)" : "LOW (clear)") + "</p>";
  html += "</body></html>";
  return html;
}

// Handler for root URL "/"
void handleRoot() {
  // Read DHT11
  humidity = dht.readHumidity();
  temperature = dht.readTemperature(); // Celsius by default
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    humidity = -1;
    temperature = -1;
  }

  // Read MQ-2
  mq2Analog = analogRead(MQ2_ANALOG_PIN);
  mq2Voltage = (mq2Analog * 3.3) / 4095.0;  // Convert ADC value to voltage
  mq2Digital = digitalRead(MQ2_DIGITAL_PIN);

  // Send the web page
  server.send(200, "text/html", getHTML());
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize DHT
  dht.begin();

  // Set MQ-2 digital pin as input
  pinMode(MQ2_DIGITAL_PIN, INPUT);

  // Configure ADC resolution (ESP32 default is 12-bit)
  analogReadResolution(12);

  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Start web server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}