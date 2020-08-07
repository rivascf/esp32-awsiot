#include <Arduino.h>
#include <ArduinoJson.h>
#include <secrets.h>
#include <bmp280imp.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
//#include <time.h>

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256); // MQTTClient(256);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

unsigned long delayTime;
unsigned long epoch;
float temperature, pressure, humidity, altitude;
String formattedDate;
String dayStamp;
String timeStamp;

void readSensorValues(float* temperature, float* pressure, float* humidity, float* altitude) {
  *temperature = bme.readTemperature();
  *pressure = bme.readPressure();
  *altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  *humidity = bme.readHumidity();
}

unsigned startBMP280(unsigned printFlag){
  if (printFlag) {
    Serial.println(F("Initializing BME280"));
  }

  unsigned status;
  status = bme.begin(0x76);
  if (!status)
  {
    if (printFlag) {
      Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
      Serial.print("SensorID was: 0x");
      Serial.println(bme.sensorID(), 16);
      Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
      Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
      Serial.print("        ID of 0x60 represents a BME 280.\n");
      Serial.print("        ID of 0x61 represents a BME 680.\n");
    }
    return status;
  }
  if (printFlag) { Serial.print("SensorID is: 0x"); }
  Serial.println(bme.sensorID(), 16);

  if (printFlag) { Serial.println("-- Sensor initialized --"); }
  return status;
}

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi SSID: ");
  Serial.println(WIFI_SSID);

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.print("Conectado, dirección IP: ");
  Serial.println(WiFi.localIP());

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if(!client.connected()){
    Serial.println("AWS IoT Connection Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  
  Serial.println("AWS IoT Connected!");
}

/*
String getFormatedTimeStamp() {
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  Serial.println(timeClient.getFormattedTime());
  formattedDate = String(timeClient.getFormattedTime());
  //timeClient.getEpochTime
  //format in which date and time is returned 2018-04-30T16:00:13Z
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  char timedate[50];
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  sprintf(timedate, "%s %s", dayStamp.c_str(), timeStamp.c_str());
  return "";
}
*/

unsigned long getEpoch() {
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  return timeClient.getEpochTime();
}

void publishMessage(bool consoleOnly)
{
  StaticJsonDocument<200> doc;
  epoch = getEpoch();
  readSensorValues(&temperature, &pressure, &humidity, &altitude);
  doc["timestamp"] = epoch; 
  doc["deviceid"] = THINGNAME;
  doc["temperature"] = temperature;
  doc["pressure"] = pressure / 100.0F;
  doc["humidity"] = humidity;
  doc["altitude"] = altitude;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); 
  if (!consoleOnly) {
    Serial.println(jsonBuffer);
  } else {
    client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  }
}

void messageHandler(String &topic, String &payload) {
  //Serial.println("-> incoming: " + topic + " - " + payload);
  epoch = getEpoch();
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.print("-> message: ");
  Serial.println(message);
  //if(strlen(message) > 0) {
  //  Serial.print("-> message: ");
  //  Serial.println(message);
  //} else {
  //  Serial.println("-> message: \"\".");
  //}
}

void setup() {
  Serial.begin(115200);
  unsigned status = startBMP280(true);
  if (!status) {
    Serial.println("Detenido");
    while (1)
    ;
  }
  connectAWS();
  Serial.print("Initlizing NTPClient to get time...");
  timeClient.begin();
  //-21600 GMT-6 America/MexicoCity Timezone 
  //-18000 GMT-5 America/MexicoCity Timezone - Summertime
  timeClient.setTimeOffset(-18000); 
  Serial.println("done!");
  delayTime = 10000;
}

void loop() {
  publishMessage(false);
  client.loop();
  delay(delayTime);
}
