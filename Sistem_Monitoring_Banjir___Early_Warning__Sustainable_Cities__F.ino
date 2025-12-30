#define BLYNK_TEMPLATE_ID "TMPL6AXl7qyEL"
#define BLYNK_TEMPLATE_NAME "Sistem Monitoring Banjir and Early Warning"
#define BLYNK_AUTH_TOKEN "9eLh7kpV_gTYgCymSjomJF4XzXsX_CiZ"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>

// Kredensial WiFi Wokwi
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// --- KONFIGURASI PIN ---
#define TRIG_PIN    5
#define ECHO_PIN    18
#define LED_MERAH   2
#define LED_KUNING  4
#define LED_HIJAU   15
#define BUZZER_PIN  19

LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;

// Variabel
float tinggi_air = 0;
bool notifikasiTerkirim = false; // Flag untuk fitur notifikasi agar tidak spam

void setup() {
  Serial.begin(115200);

  // Setup Pin
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_MERAH, OUTPUT);
  pinMode(LED_KUNING, OUTPUT);
  pinMode(LED_HIJAU, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Setup LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Koneksi ke...");
  lcd.setCursor(0, 1);
  lcd.print("Blynk IoT...");

  // Koneksi ke Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  lcd.clear();
  lcd.print("SISTEM ONLINE!");
  delay(1000);
  lcd.clear();

  // Interval pengiriman data ke Blynk (1 detik sekali)
  timer.setInterval(1000L, sendSensor);
}

void loop() {
  Blynk.run();
  timer.run();
}

void sendSensor() {
  // 1. Baca Sensor Ultrasonik
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  
  // Logika: Nilai slider Wokwi = Tinggi Air
  tinggi_air = duration * 0.034 / 2;

  // 2. Kirim Data Angka ke Blynk (V0)
  Blynk.virtualWrite(V0, tinggi_air);

  // 3. Debugging
  Serial.print("Tinggi Air: ");
  Serial.print(tinggi_air);
  Serial.println(" cm");

  // --- LOGIKA INDIKATOR BLYNK (V1) ---
  
  // KONDISI AMAN (70 - 100 cm)
  if (tinggi_air >= 70 && tinggi_air <= 100) { 
    // Hardware Lokal
    digitalWrite(LED_HIJAU, HIGH);
    digitalWrite(LED_KUNING, LOW);
    digitalWrite(LED_MERAH, LOW);
    noTone(BUZZER_PIN);
    tampilLCD(tinggi_air, "STATUS: AMAN");
    
    Blynk.virtualWrite(V1, "AMAN");             
    Blynk.setProperty(V1, "color", "#00FF00");  
    
    notifikasiTerkirim = false; 
  }
  
  // KONDISI WASPADA (30 - 69 cm)
  else if (tinggi_air >= 30 && tinggi_air <= 69) { 
    // Hardware Lokal
    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(LED_KUNING, HIGH);
    digitalWrite(LED_MERAH, LOW);
    noTone(BUZZER_PIN);
    tampilLCD(tinggi_air, "STATUS: WASPADA");
    
    Blynk.virtualWrite(V1, "WASPADA");
    Blynk.setProperty(V1, "color", "#FFFF00");
    
    notifikasiTerkirim = false; 
  }
  
  // KONDISI BAHAYA (1 - 29 cm)
  else if (tinggi_air >= 1 && tinggi_air <= 29) { 
    // Hardware Lokal
    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(LED_KUNING, LOW);
    digitalWrite(LED_MERAH, HIGH);
    tone(BUZZER_PIN, 1000);
    tampilLCD(tinggi_air, "STATUS: BAHAYA");
    
    Blynk.virtualWrite(V1, "BAHAYA!!");
    Blynk.setProperty(V1, "color", "#FF0000");
    
    if (notifikasiTerkirim == false) {
      Blynk.logEvent("banjir_alert", "PERINGATAN! Air Siaga 1 (Bahaya).");
      notifikasiTerkirim = true;
    }
  }
  
  // KERING / OFF ->
  else {
    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(LED_KUNING, LOW);
    digitalWrite(LED_MERAH, LOW);
    noTone(BUZZER_PIN);
    tampilLCD(tinggi_air, "STATUS: KERING");
    
    Blynk.virtualWrite(V1, "KERING");
    Blynk.setProperty(V1, "color", "#FFFFFF"); 
    notifikasiTerkirim = false;
  }
}

void tampilLCD(float t_air, String status) {
  lcd.setCursor(0, 0);
  lcd.print("Jarak Air: ");
  lcd.print((int)t_air);
  lcd.print(" cm      "); 
  lcd.setCursor(0, 1);
  lcd.print(status);
  lcd.print("    ");
}