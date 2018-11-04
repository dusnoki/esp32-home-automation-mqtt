#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "Adafruit_HTU21DF.h"
#include <IRremote.h>
#include <ArduinoJson.h>
#include "auth.h"

hw_timer_t *watchdogTimer = NULL;

int recvPin = 14;
IRrecv irrecv(recvPin);
IRsend irsend(4);
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
int mqttRetryAttempt = 0;
int wifiRetryAttempt = 0;
                      
/* create an instance of WiFiClientSecure */
WiFiClient espClient;
PubSubClient client(espClient);

/*LED GPIO pin*/
const char r1 = 17;
const char r2 = 16;
const char r3 = 27;
const char r4 = 26;
const char r5 = 25;
const char r6 = 33;
const char r7 = 32;
const char r8 = 13;

/* topics */
#define WEATHER_TOPIC "r1/weather"
#define REMOTE_TOPIC "r1/remote"
#define R1_TOPIC "r1/r1"
#define R2_TOPIC "r1/r2"
#define R3_TOPIC "r1/r3"
#define R4_TOPIC "r1/r4"
#define R5_TOPIC "r1/r5"
#define R6_TOPIC "r1/r6"
#define R7_TOPIC "r1/r7"
#define R8_TOPIC "r1/r8"
#define TV_TOPIC "r1/sony/code"
#define AV_TOPIC "r1/nec/code"

long lastMsg = 0;
char msg[20];
char touchmsg[20];
int counter = 0;
DynamicJsonBuffer  jsonBuffer(200);

void receivedCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received: ");
  Serial.print(topic);
  Serial.println("");

  Serial.print("payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("");
  if (strcmp(topic,R1_TOPIC)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r1, LOW);
    } else {
      /* we got '0' -> on */
      digitalWrite(r1, HIGH);
    }
  } else if (strcmp(topic,R2_TOPIC)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r2, LOW);
    } else {
      /* we got '0' -> on */
      digitalWrite(r2, HIGH);
    }
  } else if (strcmp(topic,WEATHER_TOPIC)==0) {
    timerWrite(watchdogTimer, 0);
  } else if (strcmp(topic,R3_TOPIC)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r3, LOW);
    } else {
      /* we got '0' -> on */
      digitalWrite(r3, HIGH);
    }
  } else if (strcmp(topic,R4_TOPIC)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r4, LOW);
    } else {
      /* we got '0' -> on */
      digitalWrite(r4, HIGH);
    }
  } else if (strcmp(topic,R5_TOPIC)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r5, LOW);
    } else {
      /* we got '0' -> on */
      digitalWrite(r5, HIGH);
    }
  } else if (strcmp(topic,R6_TOPIC)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r6, LOW);
    } else {
      /* we got '0' -> on */
      digitalWrite(r6, HIGH);
    }
  } else if (strcmp(topic,R7_TOPIC)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r7, LOW);
    } else {
      /* we got '0' -> on */
      digitalWrite(r7, HIGH);
    }
  } else if (strcmp(topic,R8_TOPIC)==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(r8, LOW);
    } else {
      /* we got '0' -> on */
      digitalWrite(r8, HIGH);
    }
  } else if (strcmp(topic,AV_TOPIC)==0) {
      JsonObject& root = jsonBuffer.parseObject(payload);
      if (!root.success()) {
        Serial.println("parseObject() failed");
        return;
      }
      const char* code1 = root["code1"];
      if (code1) {
        unsigned long hexCode = strtoul(code1, NULL, 16);
        Serial.println(hexCode, HEX);
        irsend.sendNEC(hexCode, 32);
      }
  } else if (strcmp(topic,TV_TOPIC)==0) {
      JsonObject& root = jsonBuffer.parseObject(payload);
      if (!root.success()) {
        Serial.println("parseObject() failed");
        return;
      }
      const char* code1 = root["code1"];
      const char* code2 = root["code2"];
      const char* code3 = root["code3"];
      Serial.println(code1);
      if (code1) {
        unsigned long hexCode = strtoul(code1, NULL, 16);
        Serial.println(hexCode, HEX);
        for (int i = 0; i < 3; i++) {
          irsend.sendSony(hexCode, 12);
          delay(40);
        }
      }

      if (code2) {
        unsigned long hexCode = strtoul(code2, NULL, 16);
        Serial.println(hexCode, HEX);
        delay(100);
        for (int i = 0; i < 3; i++) {
          irsend.sendSony(hexCode, 12);
          delay(40);
        }
      }

      if (code3) {
        unsigned long hexCode = strtoul(code3, NULL, 16);
        Serial.println(hexCode, HEX);
        delay(100);
        for (int i = 0; i < 3; i++) {
          irsend.sendSony(hexCode, 12);
          delay(40);
        }
      }
      
  }
}

void mqttconnect() {
  /* Loop until reconnected */
  while (!client.connected()) {
    Serial.print("MQTT connecting ...");
    /* client ID */
    String clientId = "Esp32Boris2131";
    /* connect now */
    if (client.connect(clientId.c_str(), serverUsername.c_str(), serverPassword.c_str())) {
      Serial.println("connected");
      /* subscribe topic */
      client.subscribe(R1_TOPIC);
      client.subscribe(R2_TOPIC);
      client.subscribe(R3_TOPIC);
      client.subscribe(R4_TOPIC);
      client.subscribe(R5_TOPIC);
      client.subscribe(R6_TOPIC);
      client.subscribe(R7_TOPIC);
      client.subscribe(R8_TOPIC);
      client.subscribe(TV_TOPIC);
      client.subscribe(AV_TOPIC);
      client.subscribe(WEATHER_TOPIC);
    } else {
      Serial.print("failed, status code =");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      /* Wait 5 seconds before retrying */
      delay(5000);
      mqttRetryAttempt++;
      if (mqttRetryAttempt > 5) {
        Serial.println("Restarting!");
        interuptReboot();
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  watchdogTimer = timerBegin(0,80,true);
  timerAlarmWrite(watchdogTimer, 40000000, false);
  timerAttachInterrupt(watchdogTimer, &interuptReboot, true);
  timerAlarmEnable(watchdogTimer);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    wifiRetryAttempt++;
    if (wifiRetryAttempt > 5) {
        Serial.println("Restarting!");
        interuptReboot();
      }
  }
  /* set led as output to control led on-off */
  pinMode(r1, OUTPUT);
  digitalWrite(r1, HIGH);
  pinMode(r2, OUTPUT);
  digitalWrite(r2, HIGH);
  pinMode(r3, OUTPUT);
  digitalWrite(r3, HIGH);
  pinMode(r4, OUTPUT);
  digitalWrite(r4, HIGH);
  pinMode(r5, OUTPUT);
  digitalWrite(r5, HIGH);
  pinMode(r6, OUTPUT);
  digitalWrite(r6, HIGH);
  pinMode(r7, OUTPUT);
  digitalWrite(r7, HIGH);
  pinMode(r8, OUTPUT);
  digitalWrite(r8, HIGH);
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("IP address of server: ");
  /* set SSL/TLS certificate */
  /* configure the MQTT server with IPaddress and port */
  client.setServer(serverHostname, 1883);
  /* this receivedCallback function will be invoked
    when client received subscribed topic */
  client.setCallback(receivedCallback);
  if (!htu.begin()) {
    Serial.println("Couldn't find sensor!");
    while (1);
  }
  irrecv.enableIRIn();
  Serial.println("Everything Setup");
}

String ircode (decode_results *results)
{
  String result;
  // Panasonic has an Address
  if (results->decode_type == PANASONIC) {
    result += "address\":\""+ String(results->address, HEX) + "\",\"";
  }

  result += "\"code\":\"" + String(results->value, HEX) + "\"";
  return result;
}

String encoding (decode_results *results)
{
  switch (results->decode_type) {
    default:
    case UNKNOWN:      return("UNKNOWN");
    case NEC:          return("NEC");
    case SONY:         return("SONY");
    case RC5:          return("RC5");
    case RC6:          return("RC6");
    case DISH:         return("DISH");
    case SHARP:        return("SHARP");
    case JVC:          return("JVC");
    case SANYO:        return("SANYO");
    case MITSUBISHI:   return("MITSUBISHI");
    case SAMSUNG:      return("SAMSUNG");
    case LG:           return("LG");
    case WHYNTER:      return("WHYNTER");
    case AIWA_RC_T501: return("AIWA_RC_T501");
    case PANASONIC:    return("PANASONIC");
    case DENON:        return("Denon");
  }
}

void  dumpInfo (decode_results *results)
{
  char data[90];
  String result;
  // Check if the buffer overflowed
  if (results->overflow) {
    result = "{\"message\": \"IR code too long. Edit IRremoteInt.h and increase RAWBUF\"}";
    return;
  }
  String encodingString = String(encoding(results));

  String codeString = String(ircode(results));
  result = "{" + String(ircode(results)) + ",\"encoding\":\"" + String(encoding(results)) + "\",\"bits\":\"" + String(results->bits, DEC) + "\"}";  
  result.toCharArray(data, (result.length()+1));
  Serial.println("publishing");
  client.publish(REMOTE_TOPIC, data, false);
}


void loop() {
  /* if client was disconnected then try to reconnect again */
  if (!client.connected()) {
    mqttconnect();
  }
  /* this function will listen for incomming
    subscribed topic-process-invoke receivedCallback */
  client.loop();
  /* we increase counter every 3 secs
    we count until 3 secs reached to avoid blocking program if using delay()*/
  long now = millis();

   decode_results  results;        // Somewhere to store the results

  if (irrecv.decode(&results)) {  // Grab an IR code
    dumpInfo(&results);           // Output the results
    irrecv.resume();              // Prepare for the next value
  }
  
  if (now - lastMsg > 5000) {
    lastMsg = now;
    char data[90];
    char temp[8];
    char humidity[8];
    dtostrf(htu.readTemperature(),  6, 2, temp);
    dtostrf(htu.readHumidity(), 6, 2, humidity);
    String json = "{\"temperature\":" + String(temp) + ",\"humidity\":" + String(humidity) + "}";
    json.toCharArray(data, (json.length() + 1));
    client.publish(WEATHER_TOPIC, data, false);
  }
}

void interuptReboot() {
    Serial.println("Rebooting");
    esp_restart_noos();
}
