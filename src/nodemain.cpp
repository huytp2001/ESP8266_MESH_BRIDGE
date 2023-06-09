// // Node: Send data to gateway over mesh network

// #include <Arduino.h>
// #include <painlessMesh.h>
// #include <vector>

// #define   MESH_PREFIX     "whateverYouLike"
// #define   MESH_PASSWORD   "somethingSneaky"
// #define   MESH_PORT       5555
// #define MAX_PACKET_SIZE   1000

// Scheduler userScheduler;
// painlessMesh  mesh;

// std::vector<String> split_string(String &input_string, char delimiter) {
//   std::vector<String> result;
//   String temp_string = "";
//   for (unsigned int i = 0; i < input_string.length(); i++) {
//     char c = input_string.charAt(i);
//     if (c == delimiter) {
//       result.push_back(temp_string);
//       temp_string = "";
//     } else {
//       temp_string += c;
//     }
//   }
//   result.push_back(temp_string);

//   return result;
// }

// class Packet {
// public:
//   Packet() {}

//   void set_src(const String& src) {
//     src_ = src;
//   }

//   String get_src() const {
//     return src_;
//   }

//   void set_seq_num(int seq_num) {
//     seq_num_ = seq_num;
//   }

//   int get_seq_num() const {
//     return seq_num_;
//   }

//   void add_hop(const String& hop) {
//     hops_.push_back(hop);
//   }

//   std::vector<String> get_hops() const {
//     return hops_;
//   }

//   void set_max_seq(int max_seq) {
//     max_seq_ = max_seq;
//   }

//   int get_max_seq() const {
//     return max_seq_;
//   }

//   void set_des(const String& des) {
//     des_ = des;
//   }

//   String get_des() const {
//     return des_;
//   }

//   void set_data(const String& data) {
//     data_ = data;
//   }

//   String get_data() const {
//     return data_;
//   }

//   String to_string() const {
//     String str;
//     str += src_;
//     str += '|';
//     str += String(seq_num_);
//     str += '|';
//     for (auto hop : hops_) {
//       str += hop;
//       str += ';';
//     }
//     str += '|';
//     str += String(max_seq_);
//     str += '|';
//     str += des_;
//     str += '|';
//     str += data_;
//     return str;
//   }

//   void from_string(const String& str) {
//     String s = str;
//     std::vector<String> parts = split_string(s, '|');
//     src_ = parts[0];
//     seq_num_ = parts[1].toInt();
//     String hops_str = parts[2];
//     int pos = 0;
//     while ((pos = hops_str.indexOf(";")) != -1) {
//       String part = hops_str.substring(0, pos);
//       hops_.push_back(part);
//       hops_str.remove(0, pos + 1);
//     }
//     max_seq_ = parts[3].toInt();
//     des_ = parts[4];
//     data_ = parts[5];
//   }

// private:
//   String src_;
//   int seq_num_;
//   std::vector<String> hops_;
//   int max_seq_;
//   String des_;
//   String data_;
// };

// void sendMessage();
// Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

// bool isSend = false;
// String data = "";
// int count = 0;

// void sendMessage() {
//     Packet packet = Packet();
//     if (isSend) {
//         if (data.substring(count*MAX_PACKET_SIZE, count*MAX_PACKET_SIZE+MAX_PACKET_SIZE).length() == 0) {
//             isSend = false;
//             return;
//         }
//         String body = data.substring(count*MAX_PACKET_SIZE, count*MAX_PACKET_SIZE+MAX_PACKET_SIZE);
//         packet.set_data(body);
//         packet.set_des("539322912");
//         if (data.length() % MAX_PACKET_SIZE==0) {
//             packet.set_max_seq(data.length()/MAX_PACKET_SIZE);
//         } else {
//             packet.set_max_seq((data.length()/MAX_PACKET_SIZE)+1);
//         }
//         packet.set_seq_num(count+1);
//         packet.set_src(String(mesh.getNodeId()));
//         packet.add_hop(String(mesh.getNodeId()));
//         Serial.println("Send message");
//         mesh.sendBroadcast(packet.to_string());
//         count++;
//         taskSendMessage.setInterval(random( TASK_SECOND * 5, TASK_SECOND * 5 ));
//     }
// }

// void startSend() {
//     count = 0;
//     data = "node!";
//     data += String(mesh.getNodeId()) + "!";
//     data += String(analogRead(A0));
//     isSend = true;
// }

// void setup() {
//     Serial.begin(115200);
//     mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
//     userScheduler.addTask(taskSendMessage);
//     taskSendMessage.enable();
//     Serial.print("Node: ");
//     Serial.println(mesh.getNodeId());
//     data = "node!";
//     data = String(mesh.getNodeId()) + "!";
//     data += String(analogRead(A0));
// }

// void loop() {
//     mesh.update();
//     if (!isSend) {
//         startSend();
//     }
// }







