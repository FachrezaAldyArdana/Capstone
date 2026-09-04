# SIADUM - Sistem Irigasi Dua Musim Berbasis IoT

Proyek *Capstone Design* berupa sistem irigasi otomatis untuk area persawahan dataran rendah berbasis mikrokontroler ESP8266. Sistem ini dirancang untuk memantau dan mengendalikan debit aliran air secara otomatis menyesuaikan kebutuhan tanaman pada dua kondisi musim (kemarau dan hujan).

---

## 📌 Fitur Utama

- **Pemantauan Real-Time**: Memantau tingkat kelembapan tanah dan ketinggian permukaan air secara langsung.
- **Kendali Otomatis Pintu Irigasi**: Menggerakkan servo secara otomatis untuk membuka dan menutup katup/saluran air berdasarkan ambang batas sensor.
- **Dukungan Dua Musim**: Logika pengaturan debit air yang adaptif terhadap kondisi musim kemarau maupun hujan.
- **Monitoring Jarak Jauh**: Terintegrasi ke Dashboard IoT via jaringan Wi-Fi untuk pengawasan parameter lingkungan dari mana saja.

---

## 🛠️ Komponen & Teknologi

### Perangkat Keras (Hardware)
- **Mikrokontroler**: ESP8266 (NodeMCU / Wemos)
- **Sensor**:
  - *Soil Moisture Sensor* (Mengukur tingkat kelembapan tanah)
  - *Ultrasonic Sensor HC-SR04* (Mendeteksi ketinggian permukaan air saluran)
- **Aktuator**:
  - *Servo Motor* (Membuka & menutup pintu/saluran irigasi)
- Catu daya 5V & instalasi mekanik miniatur saluran

### Perangkat Lunak (Software)
- Arduino IDE (C/C++ Embedded)
- Dashboard IoT / Web Interface

---

## 🔌 Konfigurasi Pinout

| Komponen | Pin Modul | Pin NodeMCU (ESP8266) | Keterangan |
| :--- | :--- | :--- | :--- |
| **Soil Moisture Sensor** | Analog AOUT | **A0** | Pembacaan analog kelembapan tanah |
| **HC-SR04 (Ultrasonic)** | Trig | **D1 (GPIO 5)** | Sinyal Trigger ultrasonik |
| | Echo | **D2 (GPIO 4)** | Sinyal Echo pantulan |
| **Servo Motor** | Signal (PWM) | **D5 (GPIO 14)** | Kendali sudut buka-tutup katup |

*(Catatan: Sesuaikan pemetaan pin di atas dengan konfigurasi di dalam file `.ino` kamu jika ada perbedaan pin GPIO).*

---

## ⚙️ Petunjuk Pemasangan

1. **Clone Repositori**:
   ```bash
   git clone [https://github.com/FachrezaAldyArdana/Capstone.git](https://github.com/FachrezaAldyArdana/Capstone.git)
