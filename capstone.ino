#include <WiFi.h>               
#include <PubSubClient.h>       
#include <ESP32Servo.h>         

// Kredensial WiFi
const char* ssid = "Hshs";
const char* password = "FachrezaZa";

// Broker MQTT
const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;

// Topik MQTT
const char* soilMoistureTopic = "sensor/soilMoisture1";
const char* soilMoisture2Topic = "sensor/soilMoisture2";  
const char* waterLevelTopic = "sensor/waterLevel";
const char* seasonTopic = "control/season";
const char* gate1StatusTopic = "actuator/gate1Status";
const char* gate2StatusTopic = "actuator/gate2Status";
const char* pumpStatusTopic = "actuator/pumpStatus";
const char* manualbottom = "control/manual"; 
const char* manualcomand = "control/comand"; 

// Objek WiFi, MQTT, dan Servo
WiFiClient espClient;
PubSubClient client(espClient);

// Variabel Global
String season = "";
String kontrol = "off";
bool manualMode = false;
float soilMoistureKemarauraw;    
float soilMoistureHujanraw;   
long jarak, waterLevel, waktu;
float tinggiDasar = 5.0; // Tinggi maksimum area pengukuran (cm)

// Pin Sensor dan Aktuator
const int pumpPin = 16;
const int soilMoisturePin1 = 36;
const int soilMoisturePin2 = 39;
// Pin Sensor Ultrasonik
const int trigPin = 14;  // Pin TRIG sensor ultrasonik
const int echoPin = 27;  // Pin ECHO sensor ultrasonik
Servo gate1Servo;
Servo gate2Servo;

// Fungsi Setup
void setup() {
  Serial.begin(115200);     

  // Koneksi Wi-Fi
  connectWiFi();

  // Setup MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);

  // Inisialisasi pin dan servo
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, HIGH);  // Pompa mati
  gate1Servo.attach(17);        // Servo Gate 1
  gate2Servo.attach(25);        // Servo Gate 2
  gate1Servo.write(0);          // Tutup gate 1
  gate2Servo.write(0);          // Tutup gate 2
  pinMode(trigPin, OUTPUT); // Atur trigPin sebagai OUTPUT
  pinMode(echoPin, INPUT);  // Atur echoPin sebagai INPUT

  reconnectMQTT();
}

// Fungsi Loop
void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // Membaca sensor kelembapan tanah hujan
  soilMoistureHujanraw = analogRead(soilMoisturePin1);
  int soilMoistureHujan = (100 - ((soilMoistureHujanraw/4095.00)*100)); 
 
  // Baca nilai kelembaban tanah
  soilMoistureKemarauraw = analogRead(soilMoisturePin2);
  int soilMoistureKemarau = (100 - ((soilMoistureKemarauraw/4095.00)*100)); 

  // kalibrasi ultrasonik
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW); 
  waktu = pulseIn(echoPin, HIGH);
  jarak = (waktu / 2.0) / 28.5; 
  waterLevel =  tinggiDasar - jarak;

   // Validasi jika hasil negatif
  if (waterLevel < 0) {
    waterLevel = 0;
  }

  if (season == "Musim Kemarau") {
    handleDrySeason(soilMoistureKemarau);
  } else if (season == "Musim Hujan") {
    handleRainySeason(soilMoistureHujan, waterLevel);
  }

  delay(1000);
}

// Fungsi Koneksi Wi-Fi
void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan ke Wi-Fi...");
  }
  Serial.println("Tersambung ke Wi-Fi");
}

// Fungsi Reconnect MQTT
void reconnectMQTT() {
  String clientId = "ESP32_" + String(WiFi.macAddress()); // Membuat client ID unik berdasarkan MAC address
  while (!client.connected()) {
    Serial.print("Menyambung ke broker MQTT...");
    if (client.connect(clientId.c_str())) { // Menggunakan client ID unik
      Serial.println("Tersambung ke MQTT");
      client.subscribe(seasonTopic);
      client.subscribe(manualbottom);
      client.subscribe(manualcomand);
    } else {
      Serial.print("Gagal, rc=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

// Callback MQTT
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.trim();
  
  Serial.print("Pesan diterima di topik: ");
  Serial.println(topic);
  Serial.print("Isi Pesan: ");
  Serial.println(message);

  if (String(topic) == seasonTopic) {
    season = message;
    Serial.print("Musim diperbarui: ");
    Serial.println(season);
  } else if (String(topic) == manualbottom) {
    kontrol = message;
    if (kontrol == "on") {
      manualMode = true;  
      Serial.println("Kontrol manual diaktifkan.");
    } else if (kontrol == "off") {
      manualMode = false;  
      Serial.println("Kontrol manual dinonaktifkan.");
    } else {
      Serial.println("Pesan manualbottom tidak valid.");
    }
  } else if (manualMode && String(topic) == manualcomand) {
    handleKontrolOn(message);  // Eksekusi perintah manual
  }
}


// Fungsi Logika Musim Kemarau
void handleDrySeason(int soilMoistureKemarau) {
  client.publish(soilMoisture2Topic, String(soilMoistureKemarau).c_str()); // Mengirimkan data soilMoisture ke topik soilMoisture2
  Serial.print("Kelembaban Tanah (Kemarau): ");
  Serial.println(soilMoistureKemarau);  
  // Mengecek apakah mode manual aktif
  if (manualMode) {
    Serial.println("Mode Manual aktif, kontrol otomatis dimatikan.");
    return;  // Jika manualMode aktif, hentikan kontrol otomatis, tapi tetap kirim data sensor
  }
  //logika musim kemarau
  if (soilMoistureKemarau <= 30) {
    digitalWrite(pumpPin, LOW); // Aktifkan pompa
    client.publish(pumpStatusTopic, "1"); // Mengirimkan status pompa ke MQTT
    Serial.println("Musim Kemarau: Kelembaban RENDAH (<30), pompa diaktifkan.");
  } 
  else if (soilMoistureKemarau <= 70) {
    digitalWrite(pumpPin, LOW); // Aktifkan pompa
    client.publish(pumpStatusTopic, "1"); // Mengirimkan status pompa ke MQTT
    Serial.println("Musim Kemarau: Kelembaban SEDANG (<70), pompa diaktifkan.");
  } 
  else if (soilMoistureKemarau <= 100) {
    digitalWrite(pumpPin, HIGH); // Matikan pompa
    client.publish(pumpStatusTopic, "0"); // Mengirimkan status pompa ke MQTT
    Serial.println("Musim Kemarau: Kelembaban TINGGI (<100), pompa dimatikan.");
  } 
  else {
    digitalWrite(pumpPin, HIGH); // Matikan pompa (default)
    client.publish(pumpStatusTopic, "0"); // Mengirimkan status pompa ke MQTT
    Serial.println("Musim Kemarau: Kelembaban sangat tinggi, pompa dimatikan.");
  }
}


// Fungsi Logika Musim Hujan
void handleRainySeason(int soilMoistureHujan, int waterLevel) {
  // Kirimkan data kelembaban tanah ke MQTT meskipun dalam mode manual
  client.publish(soilMoistureTopic, String(soilMoistureHujan).c_str());
  Serial.print("Kelembaban Tanah: ");
  Serial.println(soilMoistureHujan);  // Menampilkan kelembaban tanah di serial monitor
  
  client.publish(waterLevelTopic, String(waterLevel).c_str());
  Serial.print("Level Air: ");
  Serial.println(waterLevel);  // Menampilkan level air di serial monitor
  
  if (manualMode) {
    Serial.println("Mode Manual aktif, kontrol otomatis dimatikan.");
    return;  // Jika manualMode aktif, hentikan kontrol otomatis, tapi tetap kirim data sensor
  }  
  // Logika berdasarkan kelembaban tanah
  if (soilMoistureHujan <= 30) {
    gate1Servo.write(90); // Buka gate 1
    client.publish(gate1StatusTopic, "1");
    Serial.println("Musim Hujan: Kelembaban RENDAH (<30), gate 1 dibuka.");
  } 
  else if (soilMoistureHujan <= 70) {
    gate1Servo.write(45); // Buka gate 1
    client.publish(gate1StatusTopic, "1");
    Serial.println("Musim Hujan: Kelembaban SEDANG (<70), gate 1 dibuka.");
  } 
  else if (soilMoistureHujan <= 100) {
    gate1Servo.write(0); // Buka gate 1
    client.publish(gate1StatusTopic, "0");
    Serial.println("Musim Hujan: Kelembaban TINGGI (<100), gate 1 ditutup.");
  } 
  else {
    gate1Servo.write(0); // Tutup gate 1
    client.publish(gate1StatusTopic, "0");
    Serial.println("Musim Hujan: Kelembaban sangat tinggi, gate 1 ditutup.");
  }
  // Logika berdasarkan level air
  if (waterLevel <= 50) {
    gate2Servo.write(0); // Tutup gate 2
    client.publish(gate2StatusTopic, "0");
    Serial.println("Level air: RENDAH, GATE 2 TUTUP");
  } else {
    gate2Servo.write(90); // Buka gate 2
    client.publish(gate2StatusTopic, "1");
    Serial.println("Level air: TINGGI, GATE 2 BUKA");
  }
}

// Fungsi untuk menangani perintah manual
void handleKontrolOn(String message) {
  if (message == "open_gate1") {
    gate1Servo.write(90); // Buka gate 1
    client.publish(gate1StatusTopic, "1");
    Serial.println("Perintah manual: Gate 1 dibuka.");
  } else if (message == "close_gate1") {
    gate1Servo.write(0); // Tutup gate 1
    client.publish(gate1StatusTopic, "0");
    Serial.println("Perintah manual: Gate 1 ditutup.");
  } else if (message == "open_gate2") {
    gate2Servo.write(90); // Buka gate 2
    client.publish(gate2StatusTopic, "1");
    Serial.println("Perintah manual: Gate 2 dibuka.");
  } else if (message == "close_gate2") {
    gate2Servo.write(0); // Tutup gate 2
    client.publish(gate2StatusTopic, "0");
    Serial.println("Perintah manual: Gate 2 ditutup.");
  } else if (message == "start_pump") {
    digitalWrite(pumpPin, LOW); // Aktifkan pompa
    client.publish(pumpStatusTopic, "1");
    Serial.println("Perintah manual: Pompa diaktifkan.");
  } else if (message == "stop_pump") {
    digitalWrite(pumpPin, HIGH); // Matikan pompa
    client.publish(pumpStatusTopic, "0");
    Serial.println("Perintah manual: Pompa dimatikan.");
  }
}