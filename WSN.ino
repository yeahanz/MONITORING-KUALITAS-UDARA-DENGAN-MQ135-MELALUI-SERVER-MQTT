#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "MQ135.h"

// masukan nama WIFI, password, dan alamat server mqtt.

const char* ssid = "JTE-WIFI";  // Nama WIFI
const char* password = "";      // Password WIFI
const char* mqtt_server = "broker.mqtt-dashboard.com";    // Alamat MQTT server 
const int sensorPin= 0;     // deklarasi pin sensor
int air_quality;            

WiFiClient espClient;
PubSubClient client(espClient); 
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {

  delay(10);
  // Koneksi dengan WIFI
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// menyimpan pesan yang akan dikirimkan dalam payload
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // looping untuk koneksi dengan MQTT
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Membuat clientID secara Acak
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Publish setelah tersambung dengan MQTT
      client.publish("outMQ135", "Air Quality");
      // dan Subscribe
      client.subscribe("inMQ135");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Apabila gagal terkoneksi, mencoba lagi dalam 5 detik
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200); //komunikasi serial dengan baudrate 115200
  setup_wifi(); 
  client.setServer(mqtt_server, 1883); //Inisiasi server MQTT
  client.setCallback(callback); //memanggil fungsi callback diatas
}

void loop() {
// fungsi looping untuk mendapatkan data dari sensor MQ135
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  MQ135 gasSensor = MQ135(A0);  
  air_quality = gasSensor.getPPM();   //Mengambil data dari sensor MQ135 berdasarkan library MQ135

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    value=air_quality;
    snprintf (msg, 75, "Air Quality: %ld", value); // menampilkan data MQ135
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outMQ135", msg); //Publish data MQ135 pada MQTT server
    delay(3000);
  }
}
