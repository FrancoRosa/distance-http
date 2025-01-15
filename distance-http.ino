#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

#define rxPin 13
#define txPin 12
#define alive 2

// Ultrasonic Distance Sensor Reading for ESP8266

// WiFi credentials
const char *ssid = "RobinRata";
const char *password = "robinpapi";

// Static IP configuration
IPAddress local_IP(192, 168, 1, 200);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

ESP8266WebServer server(80);

int distance = 0; // Variable to store the distance value

EspSoftwareSerial::UART btSerial;

void handleDistanceRequest() {
  String jsonResponse = "{";
  jsonResponse += "\"distance\": ";
  jsonResponse += distance;
  jsonResponse += "}";

  server.send(200, "application/json", jsonResponse);
}

void handleNotFound() {
  String jsonResponse = "{";
  jsonResponse += "\"error\": \"Page not found\"";
  jsonResponse += "}";

  server.send(404, "application/json", jsonResponse);
}

void setup() {
  pinMode(alive, OUTPUT);
  digitalWrite(alive, LOW);
  delay(3000);

  Serial.begin(115200);
  btSerial.begin(9600, SWSERIAL_8N1, rxPin, txPin, false);
  if (!btSerial) { // If the object did not initialize, then its configuration
                   // is
                   // invalid
    Serial.println("Invalid EspSoftwareSerial pin configuration, check config");
    while (1) { // Don't continue with invalid configuration
      delay(1000);
    }
  }
  Serial.println("...pre start");
  delay(3000);
  Serial.println("... start");

  WiFi.mode(WIFI_STA);
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Failed to configure static IP");
  }
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Define HTTP routes
  server.on("/distance", HTTP_GET, handleDistanceRequest);
  server.onNotFound(handleNotFound);

  server.begin(); // Start the server
  Serial.println("HTTP server started");
}

void loop() {

  server.handleClient();

  if (btSerial.available() >= 4) { // Ensure we have at least 4 bytes to read
    uint8_t data[4];
    for (int i = 0; i < 4; i++) {
      data[i] = btSerial.read();
    }

    if (data[0] == 0xFF) { // Check for the start byte
      uint8_t h_data = data[1];
      uint8_t l_data = data[2];
      uint8_t checksum = data[3];

      // Validate checksum
      uint8_t calculated_sum = (0xFF + h_data + l_data) & 0xFF;
      if (calculated_sum == checksum) {
        // Calculate distance in mm
        distance = (h_data << 8) | l_data;
        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.println(" mm");
        digitalWrite(alive, !digitalRead(alive));

      } else {
        Serial.println("Checksum error!");
      }
    } else {
      Serial.println("Invalid start byte!");
    }
  }
}