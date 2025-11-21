#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP32Servo.h>
#include <time.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// WiFi Credentials
#define WIFI_SSID "IN"
#define WIFI_PASSWORD "12345678"

// Firebase Credentials
#define API_KEY "AIzaSyBJfT4uUk4UPqK-FNEKf9yCMGasRzTeZSg"
#define DATABASE_URL "https://fishmonitoring-29c27-default-rtdb.firebaseio.com/"
#define USER_EMAIL "ilhan@gmail.com"
#define USER_PASSWORD "123456"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

// Ultrasonic Sensor
#define echoPin 33
#define trigPin 32
long duration;
float jarak;
float tinggiWadah = 28; // cm
float tinggiAir;

// Temperature Sensor
#define SENSOR_PIN 2
OneWire oneWire(SENSOR_PIN);
DallasTemperature DS18B20(&oneWire);
float tempC;

// pH Sensor
#define PH_SENSOR_PIN 34
float calibration_value = 21.34 - 12.5;
unsigned long int avgval;
int buffer_arr[10], temp;
float ph_act;

// Turbidity Sensor (Kekeruhan)
#define TURBIDITY_PIN 35  // Gunakan pin ADC yang tersedia
const int analogValueLow = 950;    // nilai analog untuk NTU tinggi (keruh)
const int analogValueHigh = 4095;  // nilai analog untuk NTU rendah (jernih)
const float ntuLow = 100.0;
const float ntuHigh = 0.0;
float kekeruhanValue = 0;

// Servo
Servo myServo;
int currentPos = 0;
int lastFirebaseValue = 0;

// Relay Pins
#define PIN_KIPAS 26
#define PIN_LAMPU 25

// NTP Time setup
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 25200; // GMT+7
const int daylightOffset_sec = 0;

void setup() {
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Menghubungkan ke WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nTerhubung ke WiFi");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  DS18B20.begin();

  myServo.attach(13);
  myServo.write(0);
  lastFirebaseValue = 0;

  // Time setup
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  pinMode(PIN_KIPAS, OUTPUT);
  digitalWrite(PIN_KIPAS, HIGH);

  pinMode(PIN_LAMPU, OUTPUT);
  digitalWrite(PIN_LAMPU, HIGH);

  pinMode(TURBIDITY_PIN, INPUT); // kekeruhan
  Serial.println("Setup selesai.");
}

// Fungsi bantu Firebase
int bacaInteger(String path, String label) {
  if (Firebase.RTDB.getInt(&fbdo, path)) {
    if (fbdo.dataType() == "int") return fbdo.intData();
  } else {
    Serial.print("Gagal membaca "); Serial.print(label); Serial.print(": ");
    Serial.println(fbdo.errorReason());
  }
  return -1;
}

bool bacaBool(String path, String label) {
  if (Firebase.RTDB.get(&fbdo, path)) {
    String tipe = fbdo.dataType();
    if (tipe == "boolean") {
      return fbdo.boolData();
    } else if (tipe == "int") {
      return fbdo.intData() != 0;
    } else {
      Serial.print("Tipe data untuk "); Serial.print(label);
      Serial.print(" bukan boolean/int: "); Serial.println(tipe);
    }
  } else {
    Serial.print("Gagal membaca "); Serial.print(label); Serial.print(": ");
    Serial.println(fbdo.errorReason());
  }
  return false;
}

void loop() {
  // === 1. Perintah Servo dari Firebase ===
  int nilaiServo = bacaInteger("/servoValue", "Servo");
  if (nilaiServo == 180) {
    Serial.println("Perintah servo diterima: 180");
    myServo.write(180);
  } else {
    myServo.write(0);
  }

  // === 2. Pembacaan Sensor ===
  // Ultrasonik
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  jarak = duration * 0.034 / 2;
  tinggiAir = tinggiWadah - jarak;

  // Suhu
  DS18B20.requestTemperatures();
  tempC = DS18B20.getTempCByIndex(0);

  // pH
  for (int i = 0; i < 10; i++) {
    buffer_arr[i] = analogRead(PH_SENSOR_PIN);
    delay(30);
  }
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buffer_arr[i] > buffer_arr[j]) {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }
  avgval = 0;
  for (int i = 2; i < 8; i++) avgval += buffer_arr[i];
  float volt = (float)avgval * 3.3 / 4095.0 / 6;
  ph_act = -5.70 * volt + calibration_value;

  // Kekeruhan
  int turbidityADC = analogRead(TURBIDITY_PIN);
  kekeruhanValue = map(turbidityADC, analogValueLow, analogValueHigh, ntuLow * 100, ntuHigh * 100) / 100.0;
  Serial.print(kekeruhanValue);

  // === 3. Kontrol Otomatis / Manual ===
  bool otomatis = bacaBool("/app/otomatis", "Otomatis"); // ✅ tambahkan ini

  if (otomatis) {
    // Otomatis: kipas berdasarkan suhu
    digitalWrite(PIN_KIPAS, tempC >= 30.0 ? LOW : HIGH);

    // Otomatis: lampu berdasarkan jadwal
    String currentTime = getTimeString();
    String startTime = bacaString("/lamp_schedule/start", "Lamp Start");
    String endTime = bacaString("/lamp_schedule/end", "Lamp End");

    if (isWithinSchedule(currentTime, startTime, endTime)) {
      digitalWrite(PIN_LAMPU, LOW);  // nyala (aktif LOW)
    } else {
      digitalWrite(PIN_LAMPU, HIGH); // mati
    }

  } else {
    // Mode manual
    bool kipasManual = bacaBool("/app/kipas", "Kipas");
    bool lampuManual = bacaBool("/app/lampu", "Lampu");

    digitalWrite(PIN_KIPAS, kipasManual ? HIGH : LOW);
    digitalWrite(PIN_LAMPU, lampuManual ? HIGH : LOW);
  }

  // Get timestamp
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  char timeStringBuff[30];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  String timeString = String(timeStringBuff);

  // Send Data ke Firebase
  if (Firebase.ready()) {
    FirebaseJson json;
    json.set("tinggiAir", tinggiAir);
    json.set("suhuValue", tempC);
    json.set("phValue", ph_act);
    json.set("kekeruhanValue", kekeruhanValue);
    json.set("timestamp", timeString);
    json.set("timestampMillis", millis());

    String path = "/sensordata/" + String(millis());
    if (Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json)) {
      Serial.println("Data sent to Firebase successfully.");
    } else {
      Serial.println("Failed to send data to Firebase.");
      Serial.println("Reason: " + fbdo.errorReason());
    }
  } else {
    Serial.println("Firebase not ready.");
  }

  // Debug log
  Serial.print("Tinggi Air: "); Serial.println(tinggiAir);
  Serial.print("Suhu: "); Serial.println(tempC);
  Serial.print("pH: "); Serial.println(ph_act);
  Serial.print("Kekeruhan: "); Serial.print(kekeruhanValue); Serial.println(" NTU");

  delay(1000);
}

// =====================
// === FUNGSI BANTU ===
// =====================

String bacaString(String path, String label) {
  if (Firebase.RTDB.getString(&fbdo, path)) {
    return fbdo.stringData();
  } else {
    Serial.print("Gagal membaca "); Serial.print(label); Serial.print(": ");
    Serial.println(fbdo.errorReason());
    return "";
  }
}

String getTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "";

  char timeString[6];
  strftime(timeString, sizeof(timeString), "%H:%M", &timeinfo);
  return String(timeString);
}

bool isWithinSchedule(String currentTime, String startTime, String endTime) {
  if (currentTime == "" || startTime == "" || endTime == "") return false;

  int now = waktuKeMenit(currentTime);
  int start = waktuKeMenit(startTime);
  int end = waktuKeMenit(endTime);

  if (start <= end) {
    return now >= start && now < end;
  } else {
    return now >= start || now < end;
  }
}

int waktuKeMenit(String waktu) {
  int jam = waktu.substring(0, 2).toInt();
  int menit = waktu.substring(3, 5).toInt();
  return jam * 60 + menit;
}
