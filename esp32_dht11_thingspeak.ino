#include <WiFi.h>
#include <ThingSpeak.h>
#include <DHT.h>

// Wi-Fi credentials
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ThingSpeak configuration
unsigned long channelID = 1234567;
const char* writeAPIKey = "YOUR_WRITE_API_KEY";

// DHT11 configuration
#define DHTPIN 21
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;

unsigned long lastUpload = 0;
const unsigned long uploadInterval = 20000; // 20 seconds

void setup() {
  Serial.begin(115200);

  dht.begin();

  Serial.println("\nESP32 DHT11 ThingSpeak Logger");
  Serial.print("Connecting to Wi-Fi");

  WiFi.begin(ssid, password);

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startTime < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWi-Fi connection failed!");
  }

  ThingSpeak.begin(client);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected. Reconnecting...");
    WiFi.reconnect();
    delay(5000);
    return;
  }

  if (millis() - lastUpload >= uploadInterval) {
    lastUpload = millis();

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    Serial.println("\n--- DHT11 Sensor Data ---");

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("DHT11 reading failed!");
      return;
    }

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    ThingSpeak.setField(1, temperature);
    ThingSpeak.setField(2, humidity);

    int response = ThingSpeak.writeFields(channelID, writeAPIKey);

    if (response == 200) {
      Serial.println("Upload successful!");
    } else {
      Serial.print("ThingSpeak upload failed. HTTP response: ");
      Serial.println(response);
    }

    Serial.println("-------------------------");
  }
}