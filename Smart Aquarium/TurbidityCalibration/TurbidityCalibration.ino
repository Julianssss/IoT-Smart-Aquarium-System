#include <Arduino.h>

void setup(void) {
  Serial.begin(115200);  // Inisialisasi komunikasi serial pada baud rate 115200 (umum digunakan pada ESP32)
}

void loop(void) {
  int sensorValue = analogRead(34);  // Membaca nilai analog dari pin 34 (ADC1_6) pada ESP32

  Serial.print("Analog Value: ");  // Mencetak teks ke serial monitor
  Serial.println(sensorValue);  // Mencetak nilai analog ke serial monitor

  delay(1000);  // Menunggu selama 1 detik sebelum membaca nilai lagi
}
