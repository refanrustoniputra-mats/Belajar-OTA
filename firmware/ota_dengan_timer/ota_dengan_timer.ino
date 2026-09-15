#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "version.h"

// ---- GANTI BAGIAN INI ----
const char* ssid     = "MAKER 2026";
const char* password = "Makerdotindo2026";

const char* versionURL  = "https://raw.githubusercontent.com/refanrustoniputra-mats/Belajar-OTA/main/version.txt";
const char* firmwareURL = "https://raw.githubusercontent.com/refanrustoniputra-mats/Belajar-OTA/main/firmware.bin";

const unsigned long INTERVAL_CEK_UPDATE = 10UL * 60UL * 1000UL;
const unsigned long INTERVAL_BACA_SENSOR = 5UL * 1000UL;
// ---------------------------

unsigned long waktuTerakhirCek = 0;
unsigned long waktuTerakhirBaca = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.print("Menghubungkan ke WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi terhubung!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Versi firmware saat ini: ");
  Serial.println(FIRMWARE_VERSION);

  randomSeed(analogRead(0));

  cekVersiTerbaru();
  waktuTerakhirCek = millis();
}

void loop() {
  if (millis() - waktuTerakhirBaca >= INTERVAL_BACA_SENSOR) {
    waktuTerakhirBaca = millis();
    bacaSensor();
  }

  if (millis() - waktuTerakhirCek >= INTERVAL_CEK_UPDATE) {
    waktuTerakhirCek = millis();
    if (WiFi.status() == WL_CONNECTED) {
      cekVersiTerbaru();
    } else {
      Serial.println("WiFi terputus, skip cek update kali ini.");
    }
  }
}

void bacaSensor() {
  float suhu = random(200, 350) / 10.0;
  float kelembapan = random(400, 900) / 10.0;
  int cahaya = random(0, 1024);
  int kualitasUdara = random(50, 500); // dummy MQ-135, satuan ppm CO2-equivalent

  Serial.println("---- Data Sensor ----");
  Serial.print("Suhu          : ");
  Serial.print(suhu);
  Serial.println(" °C");
  Serial.print("Kelembapan    : ");
  Serial.print(kelembapan);
  Serial.println(" %");
  Serial.print("Cahaya        : ");
  Serial.print(cahaya);
  Serial.println(" (nilai ADC)");
  Serial.print("Kondisi cahaya: ");
  Serial.println(cahaya < 300 ? "Gelap" : "Terang");
  Serial.print("Kualitas udara: ");
  Serial.print(kualitasUdara);
  Serial.println(" ppm");

  if (kualitasUdara > 350) {
    Serial.println("Status udara  : Buruk, perlu ventilasi!");
  } else if (kualitasUdara > 150) {
    Serial.println("Status udara  : Sedang");
  } else {
    Serial.println("Status udara  : Baik");
  }
  Serial.println("----------------------");
}

void cekVersiTerbaru() {
  Serial.println("Mengecek versi terbaru di GitHub...");
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, versionURL);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String versiTerbaru = http.getString();
    versiTerbaru.trim();
    Serial.print("Versi terbaru di GitHub: ");
    Serial.println(versiTerbaru);

    if (versiTerbaru != FIRMWARE_VERSION) {
      Serial.println(">> Ada update baru! Mulai proses download & flash...");
      http.end();
      lakukanUpdate();
    } else {
      Serial.println(">> Firmware sudah versi terbaru, tidak perlu update.");
      http.end();
    }
  } else {
    Serial.print("Gagal mengambil version.txt, kode error: ");
    Serial.println(httpCode);
    http.end();
  }
}

void lakukanUpdate() {
  WiFiClientSecure client;
  client.setInsecure();
  Serial.println("Mendownload firmware.bin...");
  httpUpdate.rebootOnUpdate(true);

  t_httpUpdate_return hasil = httpUpdate.update(client, firmwareURL);

  switch (hasil) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("Update GAGAL. Error (%d): %s\n",
                    httpUpdate.getLastError(),
                    httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("Tidak ada update (harusnya tidak sampai sini).");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("Update BERHASIL! ESP32 akan restart...");
      break;
  }
}
