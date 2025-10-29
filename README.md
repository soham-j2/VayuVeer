# 🌬️ VāyuVeer – IoT Based Gas Detection & Alert System

**VāyuVeer** is an IoT-based real-time gas monitoring system designed to detect hazardous gases using an **ESP32 microcontroller** and **MQ-6 sensor**. It provides live ppm readings, visual alerts, and Telegram notifications through **Firebase Realtime Database** and a custom **web dashboard**.

---

## ⚙️ Features
- Real-time gas detection and alert system.
- Firebase integration for live data visualization.
- Telegram bot notifications for unsafe conditions.
- Responsive dark-themed web dashboard.
- Adjustable safety threshold via web app.
- Live ADC/PPM graph visualization.

---

## 🧩 Components Used
| Component | Description |
|------------|--------------|
| **ESP32** | Wi-Fi-enabled microcontroller controlling the system. |
| **MQ-6 Gas Sensor** | Detects LPG, propane, and other gases. |
| **LEDs** | Red (Unsafe) and Green (Safe) indication. |
| **Buzzer** | Alerts in case of gas leakage. |
| **Firebase** | Realtime database for cloud storage and web sync. |
| **Telegram Bot** | Sends safety alerts remotely. |

---

## 🔌 Circuit Diagram
![Circuit Diagram](hardware/circuit_diagram.png)

---

## 🧠 Working Principle
- ESP32 reads analog data from MQ-6.
- Converts ADC value to equivalent PPM.
- Compares against user-set threshold from Firebase.
- If unsafe → triggers red LED, buzzer, and Telegram alert.
- Data continuously updates on Firebase and the web dashboard.

---

## 🌐 Tech Stack
- **Hardware:** ESP32, MQ-6
- **Software:** Arduino IDE, HTML, CSS, JS
- **Cloud:** Firebase Realtime Database
- **API:** Telegram Bot API

---

## ✅ Advantages
- Real-time monitoring and cloud data logging.
- Low-cost and scalable IoT implementation.
- Works remotely via internet.
- Mobile-friendly dashboard.

---

## ⚠️ Limitations
- Dependent on Wi-Fi connection stability.
- Sensor calibration required for accuracy.
- Detects limited gas types (mainly LPG/propane).

---

## 💡 Applications
- Home gas safety systems.
- Industrial safety monitoring.
- Smart kitchen and laboratory automation.
- IoT safety research projects.

---

## 📚 References
- [Firebase Documentation](https://firebase.google.com/docs)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [MQ-6 Sensor Datasheet](https://components101.com/mq6-gas-sensor)
- [Telegram Bot API](https://core.telegram.org/bots/api)

---

## 👨‍💻 Developers
**Soham Jadhav**  
Nashik, Maharashtra  
@Soham_j2
