# 🚀 ESP32 Crypto Price Ticker (OLED Display)

A real-time **cryptocurrency price tracker** built using **ESP32 + OLED display**, fetching live data from the internet and displaying it with smooth animations.

---

## 📌 Overview

This project connects an **ESP32 microcontroller** to WiFi, fetches live crypto prices from the **CoinGecko API**, and displays them on a **128x64 OLED screen**.

It cycles through:
- 🪙 Bitcoin (BTC)  
- 🪙 Ethereum (ETH)  
- 🪙 Solana (SOL)  

…with smooth scrolling transitions and live 24h price changes.

---

## ✨ Features

- 📡 **Live Crypto Prices** (auto-updated every 60 seconds)
- 🔄 **Auto Rotation** between coins (every 3 seconds)
- 📉 **24h Change Indicator**
  - ▲ Up arrow for gains
  - ▼ Down arrow for losses
- 🎬 **Smooth Scroll Animation**
- 📺 **Minimal & Clean UI**
- 🌐 **No API Key Required** (CoinGecko free API)
- ⚡ Lightweight & optimized for ESP32

---

## 🛠️ Tech Stack

### Hardware
- ESP32
- SSD1306 OLED Display (128x64, I2C)

### Libraries
- WiFi.h
- HTTPClient.h
- ArduinoJson
- Adafruit_GFX
- Adafruit_SSD1306

---

## 🔌 Hardware Setup

| Component | ESP32 Pin |
|----------|----------|
| VCC      | 3.3V     |
| GND      | GND      |
| SDA      | GPIO 21  |
| SCL      | GPIO 22  |

---

## ⚙️ How It Works

1. ESP32 connects to WiFi  
2. Sends HTTP request to CoinGecko API  
3. Parses JSON response  
4. Displays:
   - Price  
   - 24h % change  
5. Updates every 60 seconds  
6. Smoothly scrolls between coins  

---

## 🧠 Core Logic

- Non-blocking timing using `millis()`
- JSON parsing via ArduinoJson
- Custom UI rendering on OLED
- Animated transitions using frame updates

---

## 📷 Output UI

- Header: `BTC/USD`, `ETH/USD`, etc.
- Center: Large formatted price
- Bottom:
  - 24h change with arrow
  - Last updated timer

---

## 🚀 Setup Instructions

1. Install Arduino IDE  
2. Add ESP32 board support  
3. Install required libraries:
   - Adafruit SSD1306  
   - Adafruit GFX  
   - ArduinoJson  
4. Update WiFi credentials:
   ```cpp
   const char* SSID = "your_wifi";
   const char* PASSWORD = "your_password";
