#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WebServer.h>

String stationMode[4] = {"?", "?", "?", "?"};


const char* ssid = "EV-CHARGING";
const char* password = "TestTest";
const char* mqtt_server = "192.168.137.1";

WiFiClient espClient;
PubSubClient client(espClient);
WebServer server(80);

String currentMode = "STATIC";
unsigned long lastUpdate[4]; // Track station updates
const unsigned long timeoutMs = 5000; // 5 sec timeout

void handleRoot();
void handleSetMode();
void sendModeToStation();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  server.on("/", handleRoot);
  server.on("/setMode", handleSetMode);
  server.begin();

  for (int i = 0; i < 4; i++) {
    lastUpdate[i] = 0;
  }
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
  server.handleClient();
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>EV Dashboard</title>""<meta http-equiv='refresh' content='2'>""</head><body>";
  html += "<h1>EV Charging Dashboard</h1>";
  html += "<p>Current Mode: <b>" + currentMode + "</b></p>";

  // Mode buttons
  html += "<h2>Change Mode:</h2>";
  html += "<form action='/setMode' method='GET'>";
  html += "<button type='submit' name='mode' value='STATIC'>STATIC</button>";
  html += "<button type='submit' name='mode' value='DYNAMIC'>DYNAMIC</button>";
  html += "<button type='submit' name='mode' value='FCFS'>FCFS</button>";
  html += "<button type='submit' name='mode' value='DIRECTORS'>DIRECTORS</button>";
  html += "</form>";


  // Station status
  html += "<h2>Stations:</h2><ul>";
  for (int i = 0; i < 4; i++) {
    html += "<li>Station " + String(i+1) + ": ";

    bool alive = (lastUpdate[i] != 0 && millis() - lastUpdate[i] <= timeoutMs);
  
  if (alive)
    html += "<span style='color:green;'>Alive</span>";
  else
    html += "<span style='color:red;'>Dead</span>";

  // Show mode only if station is alive
  if (alive)
    html += " --- Mode: <b>" + stationMode[i] + "</b>";

  html += "</li>";
  }
  html += "</ul>";

  server.send(200, "text/html", html);
}

void handleSetMode() {
  Serial.println("handleSetMode");
  currentMode = server.arg("mode");
  String payload = "{\"mode\":\"" + currentMode + "\"}";
  Serial.println("payload:" + payload);
  client.publish("dashboard/mode", payload.c_str(), true);
  sendModeToStation();
  server.sendHeader("Location", "/");
  server.send(303);
}

void sendModeToStation(){
  for (int i = 1; i <= 4; i++) {
      String topic = "station/" + String(i) + "/mode";
      client.publish(topic.c_str(), currentMode.c_str(), true);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);
  Serial.println("Callback: " + t);

  // Handle station status
  if (t.startsWith("station/") && t.endsWith("/status")) {
    int id = t.charAt(8) - '1';  // station/1/status
    if (id >= 0 && id < 4) {
      lastUpdate[id] = millis();
    }
    return;
  }

  // Handle station mode reporting: station/<id>/mode
  if (t.startsWith("station/") && t.endsWith("/mode")) {
    int id = t.charAt(8) - '1';

    if (id >= 0 && id < 4) {
      String mode;
      for (int i = 0; i < length; i++) mode += (char)payload[i];
      mode.trim();

      stationMode[id] = mode;  // save mode
      Serial.printf("Station %d mode = %s\n", id + 1, stationMode[id].c_str());
    }
  }
}


void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("Dashboard")) {
      Serial.println("connected");
      client.subscribe("station/+/status");
      client.subscribe("station/+/mode");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}