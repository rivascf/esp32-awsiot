#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.9)

Adafruit_BME280 bme;

unsigned startBMP280(unsigned printFlag);
void readSensorValues(float* temperature, float* pressure, float* humidity, float* altitude);