// Bridge 
#include <Arduino.h>
#include "painlessMesh.h"
#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Ticker.h>
#include <vector>

#define     MESH_PREFIX       "whateverYouLike"
#define     MESH_PASSWORD     "somethingSneaky"
#define     MESH_PORT         5555
#define     MAX_PACKET_SIZE   1000
#define     DHTPIN            2
#define     DHTTYPE           DHT22

void receivedCallback( const uint32_t &from, const String &msg );

painlessMesh  mesh;
Scheduler userScheduler;

BH1750 lightMeter;
DHT dht(DHTPIN, DHTTYPE);
Ticker myTicker;

bool ws_flag = false;
float temp_tem, temp_hum, temp_lux;
int temp_rain;

std::vector<String> split_string(String &input_string, char delimiter) {
  std::vector<String> result;
  String temp_string = "";
  for (unsigned int i = 0; i < input_string.length(); i++) {
    char c = input_string.charAt(i);
    if (c == delimiter) {
      result.push_back(temp_string);
      temp_string = "";
    } else {
      temp_string += c;
    }
  }
  result.push_back(temp_string);
  return result;
}

class Packet {
public:
  Packet() {}

  void set_src(const String& src) {
    src_ = src;
  }

  String get_src() const {
    return src_;
  }

  void set_seq_num(int seq_num) {
    seq_num_ = seq_num;
  }

  int get_seq_num() const {
    return seq_num_;
  }

  void add_hop(const String& hop) {
    hops_.push_back(hop);
  }

  std::vector<String> get_hops() const {
    return hops_;
  }

  void set_max_seq(int max_seq) {
    max_seq_ = max_seq;
  }

  int get_max_seq() const {
    return max_seq_;
  }

  void set_des(const String& des) {
    des_ = des;
  }

  String get_des() const {
    return des_;
  }

  void set_data(const String& data) {
    data_ = data;
  }

  String get_data() const {
    return data_;
  }

  String to_string() const {
    String str;
    str += src_;
    str += '|';
    str += String(seq_num_);
    str += '|';
    for (auto hop : hops_) {
      str += hop;
      str += ';';
    }
    str += '|';
    str += String(max_seq_);
    str += '|';
    str += des_;
    str += '|';
    str += data_;
    return str;
  }

  void from_string(const String& str) {
    String s = str;
    std::vector<String> parts = split_string(s, '|');
    src_ = parts[0];
    seq_num_ = parts[1].toInt();
    String hops_str = parts[2];
    int pos = 0;
    while ((pos = hops_str.indexOf(";")) != -1) {
      String part = hops_str.substring(0, pos);
      hops_.push_back(part);
      hops_str.remove(0, pos + 1);
    }
    max_seq_ = parts[3].toInt();
    des_ = parts[4];
    data_ = parts[5];
  }

private:
  String src_;
  int seq_num_;
  std::vector<String> hops_;
  int max_seq_;
  String des_;
  String data_;
};

void sendSensorData() {
    if (temp_tem != dht.readTemperature() || temp_hum != dht.readHumidity() || temp_rain != analogRead(A0) || temp_lux != lightMeter.readLightLevel()) {
        temp_tem = dht.readTemperature();
        temp_hum = dht.readHumidity();
        temp_rain = analogRead(A0);
        temp_lux = lightMeter.readLightLevel();
        String payload = "sensorstream|";
        payload += String(temp_tem) + "|";
        payload += String(temp_hum) + "|";
        payload += String(temp_rain) + "|";
        payload += String(temp_lux);
        Serial.println(payload);
    }
}

void setup() {
  Serial.begin(115200);

  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.setRoot(true);
  mesh.setContainsRoot(true);

  Wire.begin();
  lightMeter.begin();
  dht.begin();
  myTicker.attach(10, sendSensorData);
}

void loop() {
  mesh.update();
  if (Serial.available()) {
    if (Serial.readStringUntil('\n') == "give me data") {
      String payload = "sensordata|";
      payload += String(temp_tem) + "|";
      payload += String(temp_hum) + "|";
      payload += String(temp_rain) + "|";
      payload += String(temp_lux);
      Serial.println(payload);
    }
  }
}

void receivedCallback( const uint32_t &from, const String &msg ) {
  Packet packet = Packet();
  packet.from_string(msg);
  if (packet.get_des() == String(mesh.getNodeId())) {
    Serial.println(packet.get_data());
    return;
  }
  for (String hop : packet.get_hops()) {
    if (hop == String(mesh.getNodeId())) {
      return;
    }
  }
  packet.add_hop(String(mesh.getNodeId()));
  mesh.sendBroadcast(packet.to_string());
}



