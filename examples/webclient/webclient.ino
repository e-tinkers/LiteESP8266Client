// This is a demo example using a public http server for testing both GET and POST requests

#include <Arduino.h>
#include <LiteESP8266Client.h>

#define PACKET_MTU 1500    // Standard network MTU is 1500 bytes

LiteESP8266 radio;

const char ssid[] PROGMEM = "your_ssid";    //change it to your wifi SSID
const char password[] PROGMEM = "your_password";    //change it to your wifi password
const char host[] PROGMEM = "httpbin.org";
const int port = 80;

const char http_get_request[] PROGMEM = "GET /get?key=value HTTP/1.1\r\n";
const char http_post_request[] PROGMEM = "POST /post HTTP/1.1\r\n";
const char http_useragent[] PROGMEM = "User-Agent: Arduino-stm32/0.1\r\n";
const char http_content_type_json[] PROGMEM = "Content-Type: application/json\r\n";
const char http_host[] PROGMEM = "Host: httpbin.org\r\n";
const char http_close_connection[] PROGMEM = "Connection: close\r\n\r\n";
const char http_content_length_header[] PROGMEM = "Content-Length: ";
const char success[] PROGMEM = "success";
const char failed[] PROGMEM = "failed";
const char CRLF[] PROGMEM = "\r\n";
const char error_data_null[] PROGMEM = "Error: data came back null.";

void setupStationMode() {
  Serial.print(F("Setup station mode... "));
  if (radio.set_station_mode()) {
    Serial.println(Plash_P(success));
  } else {
    Serial.println(Plash_p(failed));
  }
}

void joinAP() {
  Serial.print(F("Join AP "));
  Serial.print(ssid);
  Serial.print(F("... "));
  if (radio.connect_to_ap(ssid, password)) {
    Serial.println(Flash_P(success));
  } else {
    Serial.println(Flash_p(failed));
  }
}

void establishTcpConnect() {
  Serial.print(F("Establish TCP Connection... "));
  if (radio.connect_progmem(host, port)) {
    Serial.println(Flash_P(success));
  } else {
    Serial.println(Flash_P(failed));
  }
}

void getHttpResponse() {
  char *data;
  while ((data = radio.get_http_response(PACKET_MTU, 5000))) {
    if (data) {
      Serial.print(F("Response Payload Length = "));
      Serial.println(strlen(data));
      Serial.println(data);
      free(data);
    } else {
      Serial.println(Flash_P(error_data_null));
    }
  }
}

void getHttpPacket() {
  char *data;
  while ((data = radio.get_response_packet(PACKET_MTU, 5000))) {
    if (data) {
      Serial.println(F("Packet Received..."));
      Serial.println(data);
      free(data);
    } else {
      Serial.println(Flash_P(error_data_null));
    }
  }
}

void httpGet() {
  Serial.println(F("Sending GET request... "));
  radio.send_progmem(http_get_request);
  radio.send_progmem(http_useragent);
  radio.send_progmem(http_host);
  radio.send_progmem(http_close_connection);
}

void httpPost() {
  Serial.println(F("Sending POST request..."));
  radio.send_progmem(http_post_request);
  radio.send_progmem(http_useragent);
  radio.send_progmem(http_host);
  radio.send_progmem(http_content_type_json);

  char http_post_content[22] = {0};
  float t = 26.5;
  // This requires sprintf floating point support,
  // see platformio.ini build_flags for ststm32 Arduino core
  // for avr-based Arduinos, uses `build_flags = -Wl,-u,vfprintf -lprintf_flt -lm`
  sprintf(http_post_content, "{\"temperature\":%.2f}", t);

  char content_length[8];
  itoa(strlen_P(http_post_content), content_length, 10);
  radio.send_progmem(http_content_length_header);
  radio.send(content_length);
  radio.send_progmem(CRLF);

  radio.send_progmem(http_close_connection);
  radio.send_progmem(http_post_content);
}

void setup() {
  delay(2000);
  radio.begin(115200);
  Serial.begin(11500);
  while (!Serial) {};

  setupStationMode();
  joinAP();
  establishTcpConnect();

  httpGet();
  getHttpPacket();

  establishTcpConnect();
  httpPost();
  getHttpResponse();
}

void loop() {}
