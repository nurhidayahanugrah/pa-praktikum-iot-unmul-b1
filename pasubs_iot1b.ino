/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Perbarui dengan nilai yang sesuai untuk jaringan Anda.
const char* ssid = "Universitas Mulawarman";
const char* password = ""; 
const char* mqtt_server = "broker.hivemq.com"; // broker gratisan

// Buzzer & LED pin
#define BUZZER_PIN D6
#define LED_PIN D5

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() { // mengatur koneksi WiFi pada sebuah perangkat
  delay(10);
  // mulai dengan menghubungkan ke jaringan WiFi
  Serial.println();
  Serial.print("Connecting to "); 
  WiFi.mode(WIFI_STA); // mengatur koneksi WiFi pada sebuah perangkat
  WiFi.begin(ssid, password);// Memulai proses koneksi WiFi dengan menggunakan ssid (nama jaringan) dan password yang telah ditentukan.
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); // Menggunakan objek Serial untuk mencetak alamat IP lokal yang diberikan oleh jaringan WiFi kepada perangkat.
}
// Fungsi untuk menerima data
void callback(char* topic, byte* payload, unsigned int length) { //Mendefinisikan fungsi callback dengan tiga parameter: topic (topik pesan), payload (isi pesan), dan length (panjang pesan).
  Serial.print("Pesan diterima [");
  Serial.print(topic);
  Serial.print("] ");
  String data = ""; // variabel untuk menyimpan data yang berbentuk array char
  for (int i = 0; i < length; i++) { //Melakukan perulangan sebanyak length kali, dengan variabel perulangan i dari 0 hingga length-1. Ini digunakan untuk membaca setiap karakter dalam payload
    Serial.print((char)payload[i]);
    data += (char)payload[i]; // menyimpan kumpulan char kedalam string
  }
  Serial.println(" PPM");
  client.subscribe("kontrolbuzz"); //Berlangganan ke topik "kontrolbuzz" menggunakan objek MQTT client yang disebut client. Ini memungkinkan perangkat untuk menerima pesan yang dikirim ke topik tersebut.
  client.subscribe("kontrolbuzzof"); 
  if ((char)payload[0] == 1) {
    Serial.println("Sensor dimatikan");
    digitalWrite(BUZZER_PIN, HIGH);  // menyalakan LED dengan membuat voltase HIGH
  } else {
    digitalWrite(BUZZER_PIN, LOW);// Mematikan LED dengan membuat voltase  LOW
  }
  int jarak = data.toInt(); // konvert string ke int
  if (jarak < 40) { // pengkondisian
    tone(BUZZER_PIN,1000,2000);
    digitalWrite(LED_PIN, HIGH);  // menyalakan LED dengan membuat voltase HIGH
  } else {
    noTone(BUZZER_PIN);
    digitalWrite(LED_PIN, LOW);  // Mematikan LED dengan membuat voltase  LOW
  }
}

// fungsi untuk mengubungkan ke broker
void reconnect() {
  // perulangan sampai terhubung kembali
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    //Buat ID klien acak
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Mencoba untuk terhubung
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Setelah terhubung, publikasikan pengumuman...
      client.subscribe("iot_unmul");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 5 detik");
      // Tunggu 5 detik sebelum mencoba lagi
      delay(5000);
    }
  }
}
void setup() {
  pinMode(BUZZER_PIN, OUTPUT);  // Inisialisasi pin buzzer
  pinMode(LED_PIN, OUTPUT);     // Inisialisasi pin LED
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883); // setup awal ke server mqtt
  client.setCallback(callback); 
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}