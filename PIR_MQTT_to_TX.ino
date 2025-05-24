#include <ESP8266WiFi.h>
#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
//#define LED_PIN 1
char msg[MSG_BUFFER_SIZE];

#define GC_txPin 3                 // RF Digital Output Pin
#define GC_ldPin LED_BUILTIN       // LED pin

void setup_wifi();
void callback(char*, byte*, unsigned int);
void reconnect();
void pirTxOutput(const char*, int);
void txSend(int); 
void timePrint();


// Update these with values suitable for your network.
const char* ssid        = "your SSID";
const char* password    = "Your SSID password";
const char* mqtt_server = "Your MQTT server";
const char* mqtt_user   = "Your MQTT user";
const char* mqtt_pass   = "Your MQTT password";
const char* mqtt_topic_hello = "PICO_433/CONNECT/SENDER";
const char* mqtt_topic  = "PICO_433/TRIGGER/433MHZ";
unsigned int GV_ignoreTime = 30000;

// This is the Friedland PIR alarm pulse train (N.B. This pattern goes forward!!)
//int GT_CodeTX[] = {12,21,730,1,1,1,1,1,1,1,4,1,3,1,6,1,5,1,7,1,5,1,3,1,6,1,3,1,8,1,3,1,5,1,5,1,5,1,5,1,4,1,5,1,35};
int GT_CodeTX[] = {15,21,800,1,1,1,1,1,1,1,4,1,3,1,6,1,5,1,7,1,5,1,3,1,6,1,3,1,8,1,3,1,5,1,5,1,5,1,5,1,4,1,5,1,40};

unsigned long GV_lastTxTime;     //
const char*   GV_lastTxText = "";


//############################################################
// setup()
//############################################################
void setup() {

  Serial.begin(115200);

  pinMode(GC_txPin, OUTPUT);
  pinMode(GC_ldPin, OUTPUT);

  delay(3000);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  delay(1000);

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  Serial.println(".................."); 
  Serial.println("Waiting for MQTT trigger on PICO_433/TRIGGER/433MHZ");
}

//############################################################
// loop()
//############################################################
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

}

//############################################################
// setup_wifi()
//############################################################
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
  Serial.println("");
  Serial.print("WiFi connected - ");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

//############################################################
// reconnect()
//############################################################
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {

    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "PICOClient-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
//      client.publish(mqtt_topic_hello, "I am now connected");
      // ... and resubscribe
      client.subscribe(mqtt_topic);
 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//############################################################
// callback()
//############################################################
void callback(char* topic, byte* payload, unsigned int length) {

  timePrint();
  Serial.print(" - Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  pirTxOutput(topic, GT_CodeTX); 

}

//############################################################
// pirTxOutput()
//############################################################
void pirTxOutput(const char* LV_text, int LT_CodeTX[]) {

  if ( (GV_lastTxText != LV_text) or ( (millis() - GV_lastTxTime ) > GV_ignoreTime) ){
    GV_lastTxText = LV_text;
    GV_lastTxTime = millis();
    Serial.println(" - TX 433 MHz sent");
    txSend(LT_CodeTX);
    delay(1000);
    txSend(LT_CodeTX);
  }else{
    Serial.println(" - Too soon, so ignored");
  };

}

//############################################################
// timePrint()
//############################################################
void timePrint() {
// unsigned long runMillis= millis();
  unsigned long allSeconds=millis()/1000;
  int runHours= allSeconds/3600;
  int secsRemaining=allSeconds%3600;
  int runMinutes=secsRemaining/60;
  int runSeconds=secsRemaining%60;
  char buf[14];
  sprintf(buf,"%02d:%02d:%02d",runHours,runMinutes,runSeconds);
  Serial.print(buf);
}

//############################################################
// txSend()
//############################################################
void txSend(int code[]) {
  digitalWrite(GC_ldPin, HIGH);  // LED pin on
  for (int j = 0; j < code[0]; j = j + 1) {
    for (int i = 3; i < ((code[1] * 2) + 3); i = i + 1) {
      if (((i) % 2) == 0) {
        digitalWrite(GC_txPin, LOW);
      } else {
        digitalWrite(GC_txPin, HIGH);
      };
      for (int k = 0; k < code[i]; k = k + 1) {
        delayMicroseconds(code[2]);
      };
    };
  };
  digitalWrite(GC_ldPin, LOW);  // LED pin off
}
