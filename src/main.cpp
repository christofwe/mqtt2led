#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

#define PIN D2
#define LED_NUM 1
Adafruit_NeoPixel leds = Adafruit_NeoPixel(LED_NUM, PIN, NEO_GRB + NEO_KHZ800);

const char* hostname = "{{HOSTNAME}}";

const char* ssid = "{{WIFI_SSID}}";
const char* password = "{{WIFI_PASSWD}}";

const char* mqtt_server = "{{MQTT_IP}}";
const char* mqtt_user = "{{MQTT_USER}}";
const char* mqtt_pass = "{{MQTT_PASSWD}}";
const char* mqtt_sub_topic = "{{MQTT_TOPIC}}";

WiFiClient espClient;
PubSubClient client(espClient);

DynamicJsonDocument doc(512);
unsigned long turn_off_time = 0;
bool led_on = false;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.hostname(hostname);
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

void set_led(uint8 R, uint8 G, uint8 B) {
  for (int i = 0; i < LED_NUM; i++) {
    leds.setPixelColor(i, leds.Color(G, R, B));
    leds.show();
  }
}

void turnOff_led() {
  set_led(0, 0, 0);
}

void on(uint8 R, uint8 G, uint8 B, int duration) {
  turn_off_time = millis() + duration*1000;
  set_led(R, G, B);
  led_on = true;
}

void blink(uint8 R, uint8 G, uint8 B, int duration){
  unsigned long time = millis();
  unsigned long newtime = time;

  while (newtime - time < duration*1000){
    set_led(R, G, B);
    delay(500);
    turnOff_led();
    delay(500);
    newtime = millis();
  }
}

void blink_slowly(uint8 R, uint8 G, uint8 B, int duration){
  unsigned long time = millis();
  unsigned long newtime = time;

  while (newtime - time < duration*1000){
    set_led(R, G, B);
    delay(1500);
    turnOff_led();
    delay(1500);
    newtime = millis();
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println(ESP.getFreeHeap(),DEC);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.print("Length:");
  Serial.print(length);

  deserializeJson(doc, payload);
  String pattern = doc["pattern"]; // "blink_slowly"
  JsonArray color = doc["color"];
  int R = color[0]; // 255
  int G = color[1]; // 255
  int B = color[2]; // 255
  int duration = doc["duration"]; // 10

  Serial.println();
  Serial.println(pattern);
  Serial.println(R);
  Serial.println(G);
  Serial.println(B);
  Serial.println(duration);
  Serial.println();

  if( pattern == "on") {
    on(R, G, B, duration);
  }
  else if( pattern == "blink") {
    blink(R, G, B, duration);
  }
  else if( pattern == "blink_slowly") {
    blink_slowly(R, G, B, duration);
  }
  else {
    Serial.println("No pattern matched.");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(hostname, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      client.setKeepAlive(30);
      client.subscribe(mqtt_sub_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000); // Wait 5 seconds before retrying
    }
  }
}

void setup() {
  Serial.begin(115200);

  setup_wifi();

  // This initializes the NeoPixel library.
  leds.begin();
  Serial.println("NeoPixel init.");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if ( led_on && millis() > turn_off_time) {
    Serial.println("LED Off.");
    turnOff_led();
    led_on = false;
    Serial.println(ESP.getFreeHeap(),DEC);
  }
}
