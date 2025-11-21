#define PH_PIN A0 // Pin analog yang terhubung dengan modul pH

float slope = -7.14; // Slope yang dihitung dari kalibrasi
float intercept = 26.635; // Intercept yang dihitung dari kalibrasi

void setup() {
  Serial.begin(115200); // Inisialisasi komunikasi serial
}

void loop() {
  int analogValue = analogRead(PH_PIN); // Membaca nilai analog
  float voltage = analogValue * (5.0 / 1023.0); // Konversi nilai ADC ke tegangan

  // Menghitung nilai pH berdasarkan tegangan
  float pHValue = slope * voltage + intercept;

  Serial.print("Analog Value: ");
  Serial.print(analogValue);
  Serial.print("  Voltage: ");
  Serial.print(voltage, 2);
  Serial.print("  pH Value: ");
  Serial.println(pHValue, 2);

  delay(1000); // Tunggu 1 detik sebelum pembacaan berikutnya
}
