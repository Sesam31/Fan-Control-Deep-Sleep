#include <WiFi.h>
#include <PubSubClient.h>
#include <SHT31.h>
#include <Wire.h>


#define FAN_PIN 23
#define SHT31_ADDRESS   0x44

uint32_t start;
uint32_t stop;

const char* ssid = "xxxxxx";
const char* password = "xxxxxx";
const char* mqtt_server = "xxxxxx";
const char* user = "xxxxxx";
const char* pass = "xxxxxx";

int rpm, InterruptCounter;

WiFiClient espClient;
PubSubClient client(espClient);
SHT31 sht;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

//  if (WiFi.getPersistent()) { WiFi.persistent(false); }
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

//rpm measurement function
ICACHE_RAM_ATTR void measure() {
  InterruptCounter = 0;
  attachInterrupt(digitalPinToInterrupt(FAN_PIN), countup, RISING);
  delay(1000);
  detachInterrupt(digitalPinToInterrupt(FAN_PIN));
  rpm = (InterruptCounter / 2) * 60;
}

ICACHE_RAM_ATTR void countup() {
  InterruptCounter++;
}

void setup() {
  
  pinMode(FAN_PIN, INPUT_PULLUP);   //fan pin
  Serial.begin(115200);

  Wire.begin();
  sht.begin(SHT31_ADDRESS);
  Wire.setClock(100000);

  uint16_t stat = sht.readStatus();
  Serial.print(stat, HEX);
  Serial.println();

  Wire.begin();
  sht.begin(SHT31_ADDRESS);
  Wire.setClock(100000);

  stat = sht.readStatus();
  Serial.print(stat, HEX);
  Serial.println();

  start = micros();
  sht.read();         // default = true/fast       slow = false
  stop = micros();

  Serial.print("\t");
  Serial.print(stop - start);
  Serial.print("\t");
  Serial.print(sht.getTemperature(), 1);
  Serial.print("\t");
  Serial.println(sht.getHumidity(), 1);
  
  //  connection setup
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  
  //MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long time = millis();
  while (millis () < time + 1000){
    measure();
    Serial.println("Getting fan rpm");
    delay(1000);
  }
  Serial.println(rpm);

  
  
  float rpmfloat = rpm/1.0;
  
  //read data from sht31 chip
  float t = sht.getTemperature();
  float h = sht.getHumidity();
  Serial.println(t);
  Serial.println(h);
  
  //send data over mqtt
  String rpmString = (String) rpmfloat;
  char message_buff0[sizeof rpmString];
  rpmString.toCharArray(message_buff0, rpmString.length()+1);

  String t_mqtt = (String) t;
  char message_buff1[sizeof t_mqtt];
  t_mqtt.toCharArray(message_buff1, t_mqtt.length()+1); 
  
  String h_mqtt = (String) h;
  char message_buff2[sizeof h_mqtt];
  h_mqtt.toCharArray(message_buff2, h_mqtt.length()+1); 
  
  client.publish("Fan-Hum/rpm", message_buff0);
  client.publish("Fan-Hum/tmp", message_buff1);
  client.publish("Fan-Hum/hum", message_buff2);
  
  time = millis();
  while (millis () < time + 5000){
    Serial.println("Waiting for mqtt to send data");
    delay(1000);
  }


  Serial.println("Going into deep sleep for 30 seconds");
  ESP.deepSleep(30e6); // 30e6 is 30 seconds
}

void loop() {
  
}
