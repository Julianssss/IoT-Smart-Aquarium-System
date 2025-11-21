// Definisikan pin untuk sensor turbidity
const int turbidityPin = 34; // Pin analog yang terhubung ke sensor turbidity

// Nilai kalibrasi
const int clearWaterValue = 4095; // Nilai analog untuk air jernih
const int turbidWaterValue = 1000; // Nilai analog untuk air keruh

// Fungsi setup
void setup() {
  Serial.begin(115200); // Memulai komunikasi serial dengan baud rate 115200
  pinMode(turbidityPin, INPUT); // Atur pin turbidity sebagai input
}

// Fungsi loop
void loop() {
  // Baca nilai analog dari sensor turbidity
  int sensorValue = analogRead(turbidityPin);

  // Konversi nilai analog ke NTU (Nephelometric Turbidity Units)
  float ntuValue = map(sensorValue, turbidWaterValue, clearWaterValue, 0, 100);

  // Cetak nilai sensor dan nilai NTU ke Serial Monitor
  Serial.print("Nilai Sensor: ");
  Serial.print(sensorValue);
  Serial.print(" | Nilai NTU: ");
  Serial.println(ntuValue);

  // Tunggu 1 detik sebelum membaca lagi
  delay(1000);
}