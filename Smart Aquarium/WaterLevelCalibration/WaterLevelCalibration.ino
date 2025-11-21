// // //Code Kalibrasi water level
// // //===============================================
const int analogInPin = 33;
int sensorValue = 0;
 
void setup() {
 Serial.begin(9600); 
}
 
void loop() {
 sensorValue = analogRead(analogInPin); 
 
 Serial.print("Sensor = " ); 
 Serial.println(sensorValue); 
 
 delay(1000); 
}

//==================================================

// #include <Wire.h>

// // Sensor pins
// #define sensorPin 33 // GPIO 13 untuk pembacaan analog (hanya mendukung input analog)

// // Thresholds untuk level air (berdasarkan pembacaan sensor analog)
// int lowerThreshold = 3100;
// int upperThreshold = 3700;

// int val = 0; // Variabel untuk level air

// void setup() {
//   Serial.begin(115200); // Baud rate ESP32 lebih tinggi
// }

// void loop() {
//   int rawValue = readSensor(); // Nilai mentah dari sensor

//   // Konversi nilai sensor ke persentase (0% - 100%) menggunakan threshold
//   int percentage;
//   if (rawValue <= lowerThreshold) {
//     percentage = map(rawValue, 0, lowerThreshold, 0, 33); // 0% - 33%
//   } else if (rawValue > lowerThreshold && rawValue <= upperThreshold) {
//     percentage = map(rawValue, lowerThreshold, upperThreshold, 34, 66); // 34% - 66%
//   } else {
//     percentage = map(rawValue, upperThreshold, 4095, 67, 100); // 67% - 100%
//   }

//   // Pastikan nilai persentase tetap dalam batas 0%-100%
//   percentage = constrain(percentage, 0, 100);

//   // Tampilkan kategori level air berdasarkan persentase
//   if (percentage == 0) {
//     Serial.println("Water Level: Empty");
//   } else if (percentage > 0 && percentage <= 33) {
//     Serial.println("Water Level: Low");
//   } else if (percentage > 33 && percentage <= 66) {
//     Serial.println("Water Level: Medium");
//   } else if (percentage > 66) {
//     Serial.println("Water Level: High");
//   }

//   // Tampilkan nilai persentase dan nilai mentah
//   Serial.print("Sensor Raw Value = ");
//   Serial.println(rawValue);
//   Serial.print("Water Level (Percentage) = ");
//   Serial.print(percentage);
//   Serial.println("%");

//   delay(1000);
// }

// // Fungsi untuk membaca sensor
// int readSensor() {
//   delay(10); // Tunggu sensor stabil (jika ada noise atau delay dari sensor fisik)
//   val = analogRead(sensorPin); // Baca nilai dari sensor
//   return val;
// }
