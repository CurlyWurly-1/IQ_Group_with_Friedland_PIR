#include <ESP8266WiFi.h>
#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
//#define LED_PIN 1
char msg[MSG_BUFFER_SIZE];

#define GC_rxPin 2                 // RF Digital Input  Pin
#define GC_sensorPin 28            // LDR pin
#define GC_ldPin LED_BUILTIN       // LED pin
#define GC_minStateLen 750         // Minimum microseconds that a state has to last for  - To be recognised as being a 1
#define GC_oneUnitPulseLen 900     // One unit Pulse Length expected in microseconds
#define GC_minStateEndLowLen 4000  // Minimum microseconds that a end state LOW has to last for - To indicate that a pulse train has finished
#define GC_maxState 400            // Maximum States (Halve for pulses, will detect "pulse trains" up to 100 pulses long)
#define GC_lightLevel 300          // Above this light level, the code will sleep 
#define GC_sleepTime  5000         // Sleep time in milliseconds 

void setup_wifi();
void callback(char*, byte*, unsigned int);
void reconnect();
void txSend(int); 
void timePrint();
void infoReport(int, int);
bool novaDtlCheck(int, int);
bool heartbeatCheck(const char*, int);
bool novaHdrCheck(int);
void pirTxOutput(const char*, int, int, int);
void pulseReverse(int);
void resetSearch();

// Array of HiLo pulse lengths (starts with Hi)
int GT_array[GC_maxState];      

// Update these with values suitable for your network.
const char* ssid        = "your SSID";
const char* password    = "Your SSID password";
const char* mqtt_server = "Your MQTT server";
const char* mqtt_user   = "Your MQTT user";
const char* mqtt_pass   = "Your MQTT password";
const char* mqtt_topic1 = "PICO_433/CONNECT";
const char* mqtt_topic2 = "PICO_433/TRIGGER";
const char* GV_pirTextUnknown = "??????";
unsigned int GV_countLow   = 125;
unsigned int GV_countHigh  = 135;
unsigned int GV_ignoreTime = 30000;

// These are Novar PIR Device pulse trains that are to be recognised (N.B. These patterns go backwards!!)
int GT_n1[] = {129,2,1,2,1,2,2,1,1,2,2,1,1,2,1,2,2,1,1,2,2,1,2,1,2,1,1,2,2,1,2,1,1,2,2,1,1,2,2,1,2,1,2,1,1,2,2,1,2,1,1,2,1,2,1,2,1,2,2,1,1,2,2,1,2,1,2,1,1,2,1,2,2,1,1,2,2,1,2,1,2,1,1,2,1,2,2,1,1,2,2,1,2,1,1,2,2,1,2,1,1,2,2,1,1,2,2,1,1,2,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,2,1,2};
int GT_n2[] = {129,1,1,2,2,1,1,2,1,2,2,1,1,2,1,2,2,1,1,2,2,1,2,1,2,1,1,2,2,1,2,1,2,1,2,1,1,2,1,2,2,1,2,1,1,2,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,2,1,2,1,1,2,1,2,1,2,2,1,2,1,1,2,1,2,2,1,2,1,2,1,1,2,1,2,2,1,1,2,1,2,2,1,2,1,2,1,2,1,1,2,1,2,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,2,1,1};
int GT_n3[] = {129,1,1,2,1,2,1,2,1,2,1,2,2,1,1,2,2,1,1,2,2,1,2,1,2,1,1,2,1,2,2,1,1,2,1,2,2,1,1,2,1,2,2,1,1,2,2,1,2,1,2,1,2,1,1,2,2,1,2,1,1,2,2,1,1,2,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,2,1,1,2,1,2,1,2,1,2,2,1,2,1,1,2,1,2,2,1,2,1,1,2,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,2,1,1};
int GT_n4[] = {129,1,1,2,2,1,2,1,2,1,1,2,2,1,2,1,2,1,1,2,2,1,2,1,2,1,1,2,1,2,2,1,1,2,2,1,1,2,2,1,2,1,1,2,1,2,2,1,1,2,2,1,2,1,1,2,2,1,1,2,2,1,2,1,2,1,1,2,2,1,1,2,2,1,2,1,2,1,2,1,1,2,2,1,1,2,2,1,2,1,2,1,2,1,2,1,1,2,1,2,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,2,1,1};
int GT_n5[] = {129,1,1,2,2,1,1,2,2,1,2,1,2,1,1,2,2,1,1,2,2,1,2,1,2,1,1,2,1,2,2,1,1,2,1,2,1,2,2,1,1,2,1,2,2,1,1,2,1,2,2,1,2,1,1,2,2,1,2,1,1,2,2,1,2,1,1,2,2,1,1,2,1,2,1,2,2,1,1,2,2,1,1,2,2,1,1,2,1,2,1,2,2,1,1,2,2,1,1,2,1,2,2,1,1,2,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,2,1,1};
int GT_n6[] = {129,1,2,1,1,2,1,2,1,2,1,2,1,2,2,1,2,1,1,2,2,1,2,1,2,1,1,2,1,2,2,1,2,1,2,1,2,1,1,2,1,2,1,2,2,1,1,2,2,1,2,1,2,1,2,1,1,2,1,2,2,1,1,2,1,2,1,2,2,1,1,2,2,1,2,1,2,1,1,2,2,1,1,2,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,2,1,1};

unsigned long GV_lastMqttTime;     //
unsigned long GV_lastHeartbeatTime;     //
unsigned long GV_startTime;             // Start Time of pulse in us
unsigned long GV_endTime;               // End   Time of pulse in us
unsigned long GV_HIpulseLen;            // Hi pulse Length
unsigned long GV_LOpulseLen;            // LO pulse Length

const char* GV_lastMqttText = "";
const char* GV_lastHeartbeatText = "";
const char* GV_pirText       = "";

int GV_sensorValue = 0;  // variable to store the value coming from the Light sensor
int GV_minStateEndLowLen;
int GV_pulseLen;
int GV_count;


bool GB_searching;
bool GB_sleeping;

bool GB_n1;  // Nova 1
bool GB_n2;  // Nova 2
bool GB_n3;  // Nova 3
bool GB_n4;  // Nova 4
bool GB_n5;  // Nova 5
bool GB_n6;  // Nova 6



//############################################################
// setup()
//############################################################
void setup() {

  Serial.begin(115200);
  pinMode(GC_sensorPin, INPUT);
  pinMode(GC_rxPin, INPUT);
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
  Serial.println("Sniffing for 433 MHz RF messages...");
  while (digitalRead(GC_rxPin) == true);
  while (digitalRead(GC_rxPin) == false);
  GV_startTime = micros();
  GB_sleeping   = false;
  resetSearch();
}

//############################################################
// loop()
//############################################################
void loop() {

  GV_sensorValue = analogRead(GC_sensorPin);

  while (GB_searching) {
    // Wait until it goes low
    while (digitalRead(GC_rxPin) == true);
    // We now have a LOW, so calculate pulse length of the HIGH
    GV_endTime = micros();
    GV_HIpulseLen = GV_endTime - GV_startTime;
    GV_startTime = GV_endTime;
    if (GV_HIpulseLen < GC_minStateLen) {
      // N.B. Wait until it goes HIGH again and then continue
      while (digitalRead(GC_rxPin) == false);
      GV_startTime = micros();
// Stop any more processing of this run of the "while (GB_searching)" and start again
      resetSearch();
      continue;
    };

    // You will be here if the HI pulse length is OK and the state is currently LO
    // Wait until the state goes HIGH again, so we can calculate pulse length of the LOW
    // N.B. In the loop, check the length of the LOW. If it is greater than the LOW timeout
    //      then this signifies that end of the pulse train has happened - So set "GB_searching" to false
    //      to signal that the code is to break out at the end of the "while (GB_searching)" loop
    while (digitalRead(GC_rxPin) == false) {
      if ((micros() - GV_startTime) > GC_minStateEndLowLen) {
      // PIR devices check routine 1 of 2
      // Check if this pulse train is recognisable as a being from a NOVA PIR device and if so, set all indicators to true
        if (novaHdrCheck(GV_count) == true) {
          GB_searching = false;
          break;
        }else{
// Stop any more processing of this run of "while (GB_searching)" loop and start again
          resetSearch();
          continue;
        }
      }
    };

    GV_endTime = micros();
    GV_LOpulseLen = GV_endTime - GV_startTime;
    GV_startTime = GV_endTime;
    if (GV_LOpulseLen < GC_minStateLen) {
// Stop any more processing of this run of "while (GB_searching)" loop and start again
      resetSearch();
      continue;
    }

    // if you are here, then the pulse is OK - So add the pair of HI and LO pulse lengths to the array
    //  GV_pulseLen = ((GT_array[i + 1] * 10) + (5 * GV_shortestPulseLen)) / (10 * GV_shortestPulseLen);
    GT_array[GV_count] = GV_HIpulseLen / GC_oneUnitPulseLen;  // Store Pulse Length
    if (GT_array[GV_count] == 0) {
      GT_array[GV_count] = 1;
    };
    GV_count++;
    GT_array[GV_count] = GV_LOpulseLen / GC_oneUnitPulseLen;  // Store Pulse Length
    if (GT_array[GV_count] == 0) {
      GT_array[GV_count] = 1;
    };
    GV_count++;
// If for some reason the pulse train "breaks the bank" of available array space, jsut cancel and start again    
    if (GV_count > (GC_maxState-2)) {
      resetSearch();
      continue;
    };
  };
//************************************************
// If you are here, then the flag "GB_searching" has been set to false because a LOW state has been found 
// that is sufficiently long enoogh to be considered as being an end low state (i.e. the pulses have stopped)    
// This means that a successful collection of pulses should have been collected, and they are suitable for further recognition checks.
// Before continuing thuogh, the "GV_count" value just needs to be pulled back by one and thereafter, the pulse train is reversed.
// The reversal of the pulse train is done because it enables a more reliable check. After reversal, the pulse train starts where the "end" long pause is,
// and then it proceeds backwards to the start pulse.
//************************************************
  GV_count--;
  pulseReverse(GV_count);

// Now we have to review the details of the pulse train by comparing to known pulse trains   
// First, set all indicators to true
  GB_n1 = true;
  GB_n2 = true;
  GB_n3 = true;
  GB_n4 = true;
  GB_n5 = true;
  GB_n6 = true;
// Check the pulse train detail appropriately
// If one of the indexes doesn't match, then switch off the associated indicator 
// Ignore last 33 pulses (They are some sort of random code?) and ignore the first couple of pulses (after 91)
  for (int i = 33; i < 91; i++) {
    if (GB_n1) { GB_n1 = novaDtlCheck(GT_array[i], GT_n1[i] ); };     // Check if Nova PIR 1 is still OK, if not, set indicator to false
    if (GB_n2) { GB_n2 = novaDtlCheck(GT_array[i], GT_n2[i] ); };     // Check if Nova PIR 2 is still OK, if not, set indicator to false
    if (GB_n3) { GB_n3 = novaDtlCheck(GT_array[i], GT_n3[i] ); };     // Check if Nova PIR 3 is still OK, if not, set indicator to false
    if (GB_n4) { GB_n4 = novaDtlCheck(GT_array[i], GT_n4[i] ); };     // Check if Nova PIR 4 is still OK, if not, set indicator to false
    if (GB_n5) { GB_n5 = novaDtlCheck(GT_array[i], GT_n5[i] ); };     // Check if Nova PIR 5 is still OK, if not, set indicator to false
    if (GB_n6) { GB_n6 = novaDtlCheck(GT_array[i], GT_n6[i] ); };     // Check if Nova PIR 6 is still OK, if not, set indicator to false
  };

// Set the PIR Text and output the pulse train
  GV_pirText = GV_pirTextUnknown;
  if (GB_n1) { GV_pirText = "NOVA 1";};
  if (GB_n2) { GV_pirText = "NOVA 2";};
  if (GB_n3) { GV_pirText = "NOVA 3";};
  if (GB_n4) { GV_pirText = "NOVA 4";};
  if (GB_n5) { GV_pirText = "NOVA 5";};
  if (GB_n6) { GV_pirText = "NOVA 6";};
  pirMqttOutput(GV_pirText, GV_count, GT_array);

  resetSearch();
// Wait until ready 
  while (digitalRead(GC_rxPin) == false)
  GV_startTime = micros();
}

//############################################################
// void setup_wifi()
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
// void callback()
//############################################################
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  String hexRGB = String((char*)payload);      
  // convert HEX to RGB
  Serial.println();
  Serial.print(hexRGB);
  Serial.println();
  hexRGB.toUpperCase();
  char char1[6];
  hexRGB.toCharArray(char1,7);
  
}

//############################################################
// void reconnect()
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
      client.publish(mqtt_topic1, "I am now connected");
      // ... and resubscribe
//      client.subscribe(mqtt_topic);
 
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
// resetSearch()
//############################################################
void resetSearch() {
  GV_count = 0;
  GB_searching = true;
  memset(GT_array, 0, GC_maxState);
}

//############################################################
// pulseReverse()
//############################################################
void pulseReverse(int LV_count){
int LV_pulses;
int LV_tempInt;
  // Determine the pulses in the message
  LV_pulses = int(LV_count / 2);

  // Reverse the pulses
  for (int i = 0; i < LV_count; i++) {
    LV_tempInt = GT_array[i];
    GT_array[i] = GT_array[LV_count - i - 1];
    GT_array[LV_count - i - 1] = LV_tempInt;
    if (i >= LV_pulses) {
      break;
    }
  };
}

//############################################################
// novaHdrCheck()
//############################################################
bool novaHdrCheck(int LV_count) {
  if ( (LV_count >= GV_countLow) & (LV_count <= GV_countHigh) ) {
    return true;
  } else {
    return false;
  };
}

//############################################################
// novaDtlCheck()
//############################################################
bool novaDtlCheck(int LV_val1, int LV_val2) {
  if ( ((LV_val1 == 1) & (LV_val2 == 1)) || ((LV_val1 != 1) & (LV_val2 != 1) )) {
    return true;
  } else {
    return false;
  };
}

//############################################################
// pirTxOutput()
//############################################################
void pirMqttOutput(const char* LV_text, int LV_count, int LT_array[]) {

bool LB_heartbeat;

// Check if a heartbeat signal
  LB_heartbeat = heartbeatCheck(LV_text, LT_array);
  if (LB_heartbeat) {
    if ( (GV_lastHeartbeatText != LV_text) or ( (millis() - GV_lastHeartbeatTime ) > GV_ignoreTime) ){
      GV_lastHeartbeatText = LV_text;
      GV_lastHeartbeatTime = millis();

      timePrint();
      Serial.print(" - ");
      Serial.print(LV_text);
      Serial.print(" - Heartbeat ignored - ");
      Serial.print(LT_array[25]);
      Serial.print(LT_array[26]);
      Serial.print(LT_array[27]);
      Serial.print(LT_array[28]);
      Serial.println(LT_array[29]);
    };
  }else{
    if ( (GV_lastMqttText != LV_text) or ( (millis() - GV_lastMqttTime ) > GV_ignoreTime) ){
      GV_lastMqttText = LV_text;
      GV_lastMqttTime = millis();
      timePrint();
      Serial.print(" - ");
      Serial.print(LV_text);
      Serial.print(" - ");
      infoReport(LV_count, LT_array);
      mqttSend(LV_text, LV_count, LT_array);
    };
  };
}

//############################################################
// heartbeatCheck()
//############################################################
bool heartbeatCheck(const char* LV_text, int LT_array[]) {
// Check for heartbeat nible at index 25 to 28 (2,1,1,2)  
  if ( ( (LT_array[25] == 2) && (LT_array[26] == 1) && (LT_array[27] == 1) && (LT_array[28] == 2) )  || ( (LT_array[26] == 2) && (LT_array[27] == 1) && (LT_array[28] == 1) && (LT_array[29] == 2) ) ) {
    return true; 
  }else{  
    return false;    
  }
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
// infoReport()
//############################################################
void infoReport(int LV_count, int LT_code[]) {
  Serial.print("GV_count = ");
  Serial.print(LV_count);
  Serial.print(",");
  for (int j = 1; j < LV_count; j++) {
    Serial.print(LT_code[j]);
    Serial.print(",");
  };
  Serial.println(" ");
}

//############################################################
// mqttSend()
//############################################################
void mqttSend(const char* LV_text, int LV_count, int code[]) {

  digitalWrite(GC_ldPin, HIGH);  // LED pin on
  if (!client.connected()) {
    reconnect();
  }

  client.publish(mqtt_topic2, LV_text);
  client.loop();
  digitalWrite(GC_ldPin, LOW);  // LED pin off
}
