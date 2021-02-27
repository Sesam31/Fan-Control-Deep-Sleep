#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <FanController.h>

#define SENSOR_PIN 5
#define SENSOR_THRESHOLD 1000

const char* ssid = "xxxxxx";
const char* password = "xxxxxx";
const char* mqtt_server = "xxxxxx";
const char* user = "xxxxxx";
const char* pass = "xxxxxx";

WiFiClient espClient;
PubSubClient client(espClient);
FanController fan(SENSOR_PIN, SENSOR_THRESHOLD);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  if (WiFi.getPersistent()) { WiFi.persistent(false); }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), user, pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("Fan-Hum/rpm", "connected");
      // ... and resubscribe
      client.subscribe("Fan-Hum/rpm");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(4, INPUT);
  pinMode(5, INPUT_PULLUP);   //fan pin
  Serial.begin(115200);

  
  
  //  connection setup
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  fan.begin();
  
  //MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  unsigned long time = millis();
  while (millis () < time + 1000){
    Serial.println("Waiting for fan to stabilize");
    delay(1000);
  }

  // Call fan.getSpeed() to get fan RPM.
  unsigned int rpms = fan.getSpeed(); // Send the command to get RPM
  Serial.print(rpms);
  Serial.println("RPM");

  float rpmfloat = rpms/1.0;
  String rpmString = (String) rpmfloat;
  char message_buff0[5];
  rpmString.toCharArray(message_buff0, rpmString.length()+1); 
  client.publish("Fan-Hum/rpm", message_buff0);

  time = millis();
  while (millis () < time + 1000){
    Serial.println("Waiting for mqtt to send data");
    delay(1000);
  }


  Serial.println("Going into deep sleep for 30 seconds");
  ESP.deepSleep(30e6); // 30e6 is 30 seconds
}

void loop() {
  
}
