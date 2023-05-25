// PROJEK AKHIR PRAKTIKUM IOT //

#include <ESP8266WiFi.h>   //Library untuk esp8266
#include <PubSubClient.h>  //Library untuk MQTT
#include <NewPing.h>       //Library untuk HC-SR04

const char* ssid = "Universitas Mulawarman";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com";

#define TRIGGER_PIN D5    //Pin Trigger HC-SR04 pada NodeMCU
#define ECHO_PIN D6       //Pin Echo HC-SR04 pada NodeMCU
#define MAX_DISTANCE 400  //Maksimum Pembacaan Jarak (cm)
#define BUTTON_PIN D4     //Pin Buzzer pada NodeMCU

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);  //Setup Pin HC-SR04 dan Jarak Pembacaan dengan Fungsi Library

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0; // Membuat variabel lastMsg dengan tipe data unsigned long yang akan digunakan untuk menyimpan waktu terakhir pesan dikirim atau diterima
#define MSG_BUFFER_SIZE (50) // Menggunakan praprosesor untuk mendefinisikan konstanta MSG_BUFFER_SIZE dengan nilai 50, yang akan digunakan sebagai ukuran maksimum buffer pesan.
char msg[MSG_BUFFER_SIZE];
int value = 0; // mendeklarasikan variabel value dengan tipe data integer dan menginisialisasinya dengan nilai 0.
int buttonState = 0; // Variabel ini akan digunakan untuk menyimpan status tombol.

// Variable untuk menyimpan status sensor
bool sensorAktif = false;

void setup_wifi() {
  // Fungsi ini digunakan untuk mengatur dan menghubungkan NodeMCU ke jaringan WiFi menggunakan SSID dan password yang telah ditentukan.
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Selama proses koneksi, fungsi ini akan menampilkan status koneksi WiFi dan alamat IP yang berhasil diperoleh.
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//  dipanggil ketika ada pesan yang diterima dari broker MQTT dan menampilkan topik dan isi pesan yang diterima.
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Pesan Diterima [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// fungsi untuk menghubungkan ke broker
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("iot_unmul/iot_b_1"); //Jika koneksi berhasil, fungsi ini akan melakukan subscribe ke topik "iot_unmul/iot_b_1".
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" Coba lagi dalam 5 detik...");
      delay(1000);
    }
  }
}

// inisialisasi serial, koneksi WiFi, koneksi MQTT, dan mode pin dilakukan.
void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  sensorAktif = false;
  digitalWrite(TRIGGER_PIN, LOW);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  int buttonState = digitalRead(BUTTON_PIN);

  // Jika tombol ditekan, status sensor diubah dan dilakukan pengiriman sinyal untuk mengaktifkan atau mematikan sensor ultrasonik.
  if (buttonState == LOW) {
    // Ubah status sensor
    sensorAktif = !sensorAktif;

    // Tampilkan status sensor
    if (sensorAktif) {
      Serial.println("Sensor diaktifkan");
      digitalWrite(TRIGGER_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(TRIGGER_PIN, LOW);
    } else {
      Serial.println("Sensor dimatikan");
    }

    // Tunggu hingga tombol dilepas
    while (buttonState == LOW) {
      buttonState = digitalRead(BUTTON_PIN);
    }

    // Tunggu sejenak agar tidak terjadi bouncing
    delay(50);
  }
  if (sensorAktif) {
    unsigned int jarak = sonar.ping_cm();  //Melakukan Pembacaan Jarak dalam bentuk cm
    delay(1000);
    Serial.println("Sensor diaktifkan");
    snprintf(msg, MSG_BUFFER_SIZE, "%s", itoa(jarak, msg, 10));  // itoa (konversi integer ke string)
    Serial.print("Publish message: ");
    Serial.print(msg);
    Serial.println(" cm");
    client.publish("iot_unmul", msg);  // fungsi untuk publish ke broker
  }
  if (buttonState == HIGH) {
  }
  delay(1000); // Setelah pengiriman data, terdapat penundaan 1 detik sebelum memasuki iterasi selanjutnya.
}
