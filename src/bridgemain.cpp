// Bridge 
#include <Arduino.h>
#include "IPAddress.h"
#include "painlessMesh.h"
#include "Hash.h"
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Ticker.h>
#include <vector>

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
#define MAX_PACKET_SIZE   1000

#define   STATION_SSID     "Hung -2.4G"
#define   STATION_PASSWORD "02081996"

#define HOSTNAME "HTTP_BRIDGE"

#define DHTPIN 2
#define DHTTYPE DHT22

void receivedCallback( const uint32_t &from, const String &msg );
IPAddress getlocalIP();

painlessMesh  mesh;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

IPAddress myIP(0,0,0,0);
IPAddress myAPIP(0,0,0,0);

BH1750 lightMeter;
DHT dht(DHTPIN, DHTTYPE);
Ticker myTicker;

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

    void printOut() {
    Serial.print("Src: ");
    Serial.println(src_);
    Serial.print("Seq_num: ");
    Serial.println(seq_num_);
    Serial.print("Hop: ");
    for (String hop: hops_) {
      Serial.print(hop);
      Serial.print(",");
    }
    Serial.println();
    Serial.print("Max_seq: ");
    Serial.println(max_seq_);
    Serial.print("Des: ");
    Serial.println(des_);
    Serial.print("Data: ");
    Serial.println(data_);
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
  String payload = "sensordata|";
  payload += String(dht.readTemperature()) + "|";
  payload += String(dht.readHumidity()) + "|";
  payload += String(analogRead(A0)) + "|";
  payload += String(lightMeter.readLightLevel());
  ws.textAll(payload);
}

void sendSensorData_get(AsyncWebServerRequest *request) {
  String payload = String(dht.readTemperature()) + "|";
  payload += String(dht.readHumidity()) + "|";
  payload += String(analogRead(A0)) + "|";
  payload += String(lightMeter.readLightLevel());
  request->send(200, "text/plain", payload);
}

void setup() {
  Serial.begin(115200);

  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6);
  mesh.onReceive(&receivedCallback);
  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);
  mesh.setRoot(true);
  mesh.setContainsRoot(true);

  myAPIP = IPAddress(mesh.getAPIP());
  Serial.println("My AP IP is " + myAPIP.toString());

  server.on("/sensorData", HTTP_GET, sendSensorData_get);

  server.addHandler(&ws);
  server.begin();

  Wire.begin();
  lightMeter.begin();
  Serial.println(F("BH1750 begin"));
  dht.begin();
  Serial.println(F("DHT begin"));
}

bool ws_flag = false;

void loop() {
  mesh.update();
  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());
    myTicker.attach(5, sendSensorData);
    ws_flag = true;
  }
}

void receivedCallback( const uint32_t &from, const String &msg ) {
  Serial.print("Receive packet: ");
  Packet packet = Packet();
  packet.from_string(msg);
  if (packet.get_des() == String(mesh.getNodeId())) {
    Serial.println("Message arriving successful");
    ws.textAll(packet.get_data());
    return;
  }
  for (String hop : packet.get_hops()) {
    if (hop == String(mesh.getNodeId())) {
      Serial.println("Message being block");
      return;
    }
  }
  Serial.println("Message being forward");
  packet.add_hop(String(mesh.getNodeId()));
  mesh.sendBroadcast(packet.to_string());
}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}


