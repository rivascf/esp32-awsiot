#include <pgmspace.h>

#define SECRET
#define THINGNAME "MyESP32IoTDev"
#define AWS_IOT_PUBLISH_TOPIC   "test/testing"
#define AWS_IOT_SUBSCRIBE_TOPIC "test/testing"

const char WIFI_SSID[] = "xxxxxx";
const char WIFI_PASSWORD[] = "xxxxxx";
const char AWS_IOT_ENDPOINT[] = "xxxxxxxxxx.iot.us-east-1.amazonaws.com";

// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

// Device Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
-----END RSA PRIVATE KEY-----
)KEY";

void connectAWS();
void publishMessage();

void messageHandler(String &topic, String &payload);
