// This is a demo example using a public http server for testing both GET and POST requests

#include <Arduino.h>
#include <LiteESP8266Client.h>

#define PACKET_MTU 1500    // Packet MTU is 1500 bytes

LiteESP8266 radio;

const char ssid[] PROGMEM = "DIDI1";    //change it to your wifi SSID
const char password[] PROGMEM = "blinkie291boo";    //change it to your wifi password
const char host[] PROGMEM = "httpbin.org";
const int port = 80;

const char http_useragent[] PROGMEM = "User-Agent: Arduino-stm32/0.1\r\n";
const char http_host[] PROGMEM = "Host: httpbin.org\r\n";
const char http_close_connection[] PROGMEM = "Connection: close\r\n\r\n";
const char success[] PROGMEM = "success";
const char failed[] PROGMEM = "failed";

void setupStationMode() {
  Serial.print(F("Setup station mode... "));
  if (radio.set_station_mode()) {
    Serial.println(PSTR(success));
  } else {
    Serial.println(PSTR(failed));
  }
}

void joinAP() {
  Serial.print(F("Join AP "));
  Serial.print(ssid);
  Serial.print(F("... "));
  if (radio.connect_to_ap(ssid, password)) {
    Serial.println(F("success."));
  } else {
    Serial.println(F("failed."));
  }
}

void establishTcpConnect() {
  Serial.print(F("Establish TCP Connection... "));
  if (radio.connect_progmem(host, port)) {
    Serial.println(F("success."));
  } else {
    Serial.println(F("failure."));
  }
}

void getHttpResponse() {
  char *data;
  while ((data = radio.get_http_response(PACKET_MTU, 5000))) {
    if (data) {
      Serial.print(PSTR("Response Payload Length = "));
      Serial.println(strlen(data));
      Serial.println(data);
    } else {
      Serial.println(F("Error: Data came back null."));
    }
  }
  free(data);
}

void getHttpPacket() {
  char *data;
  while ((data = radio.get_response_packet(PACKET_MTU, 5000))) {
    if (data) {
      Serial.println(PSTR("Packet Received..."));
      Serial.println(data);
    } else {
      Serial.println(F("Error: Data came back null."));
    }
  }
  free(data);
}

void httpGet() {
  Serial.println(F("Sending GET request... "));
  radio.send_progmem(PSTR("GET /get?key=value HTTP/1.1\r\n"));
  radio.send_progmem(http_useragent);
  radio.send_progmem(http_host);
  radio.send_progmem(http_close_connection);
}

void httpPost() {
  Serial.println(F("Sending POST request..."));
  radio.send_progmem(PSTR("POST /post HTTP/1.1\r\n"));
  radio.send_progmem(http_useragent);
  radio.send_progmem(http_host);
  radio.send_progmem(PSTR("Content-Type: application/json\r\n"));

  char http_post_content[22] = {0};
  float t = 26.5;
  // This requires sprintf floating point support,
  // see platformio.ini build_flags for ststm32 Arduino core
  // for avr-based Arduinos, uses `build_flags = -Wl,-u,vfprintf -lprintf_flt -lm`
  sprintf(http_post_content, "{\"temperature\":%.2f}", t);

  char content_length[8];
  itoa(strlen_P(http_post_content), content_length, 10);
  radio.send_progmem(PSTR("Content-Length: "));
  radio.send(content_length);
  radio.send_progmem(PSTR("\r\n"));

  radio.send_progmem(http_close_connection);
  radio.send_progmem(http_post_content);
}

void setup() {
  delay(2000);
  radio.begin(115200);
  Serial.begin(921600);
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
