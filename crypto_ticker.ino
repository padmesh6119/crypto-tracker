#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ── OLED config ──────────────────────────────────────────────
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1   // no reset pin
#define OLED_ADDR    0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ── WiFi credentials ─────────────────────────────────────────
const char* SSID     = "401";
const char* PASSWORD = "zxcvbnm?";

// ── CoinGecko API (free, no key needed) ──────────────────────
// Fetches BTC, ETH, SOL in USD
const char* API_URL =
  "https://api.coingecko.com/api/v3/simple/price"
  "?ids=bitcoin,ethereum,solana"
  "&vs_currencies=usd"
  "&include_24hr_change=true";

// ── Coin data ─────────────────────────────────────────────────
struct Coin {
  const char* symbol;
  float       price;
  float       change24h;
  bool        fetched;
};

Coin coins[] = {
  { "BTC", 0, 0, false },
  { "ETH", 0, 0, false },
  { "SOL", 0, 0, false }
};
const int NUM_COINS = 3;

// ── Timing ────────────────────────────────────────────────────
unsigned long lastFetch   = 0;
unsigned long lastScroll  = 0;
const unsigned long FETCH_INTERVAL  = 60000;  // fetch every 60s
const unsigned long SCROLL_INTERVAL = 3000;   // rotate coin every 3s

int displayIndex = 0;

// ── Scroll animation state ────────────────────────────────────
int  scrollX     = SCREEN_WIDTH;
bool scrolling   = false;

// ─────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  // Init OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 init failed");
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  showBootScreen();

  // Connect WiFi
  connectWiFi();

  // Initial fetch
  fetchPrices();
}

// ─────────────────────────────────────────────────────────────
void loop() {
  unsigned long now = millis();

  // Re-fetch every minute
  if (now - lastFetch >= FETCH_INTERVAL) {
    fetchPrices();
    lastFetch = now;
  }

  // Rotate displayed coin every 3s
  if (now - lastScroll >= SCROLL_INTERVAL) {
    lastScroll   = now;
    displayIndex = (displayIndex + 1) % NUM_COINS;
    scrollX      = SCREEN_WIDTH;    // reset scroll-in
    scrolling    = true;
  }

  // Animate scroll-in
  if (scrolling) {
    scrollX -= 6;
    if (scrollX <= 0) {
      scrollX   = 0;
      scrolling = false;
    }
    drawCoinScreen(displayIndex, scrollX);
    delay(16);   // ~60fps
  } else {
    drawCoinScreen(displayIndex, 0);
    delay(100);
  }
}

// ─────────────────────────────────────────────────────────────
void connectWiFi() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 24);
  display.print("Connecting WiFi...");
  display.display();

  WiFi.begin(SSID, PASSWORD);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 30) {
    delay(500);
    tries++;
  }

  display.clearDisplay();
  display.setCursor(0, 24);
  if (WiFi.status() == WL_CONNECTED) {
    display.print("WiFi OK");
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
  } else {
    display.print("WiFi FAILED");
  }
  display.display();
  delay(1000);
}

// ─────────────────────────────────────────────────────────────
void fetchPrices() {
  if (WiFi.status() != WL_CONNECTED) return;

  showFetchingScreen();

  HTTPClient http;
  http.begin(API_URL);
  http.addHeader("Accept", "application/json");

  int code = http.GET();
  if (code == HTTP_CODE_OK) {
    String payload = http.getString();

    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, payload);

    if (!err) {
      coins[0].price    = doc["bitcoin"]["usd"].as<float>();
      coins[0].change24h= doc["bitcoin"]["usd_24h_change"].as<float>();
      coins[0].fetched  = true;

      coins[1].price    = doc["ethereum"]["usd"].as<float>();
      coins[1].change24h= doc["ethereum"]["usd_24h_change"].as<float>();
      coins[1].fetched  = true;

      coins[2].price    = doc["solana"]["usd"].as<float>();
      coins[2].change24h= doc["solana"]["usd_24h_change"].as<float>();
      coins[2].fetched  = true;

      Serial.printf("BTC: $%.2f (%.2f%%)\n", coins[0].price, coins[0].change24h);
      Serial.printf("ETH: $%.2f (%.2f%%)\n", coins[1].price, coins[1].change24h);
      Serial.printf("SOL: $%.2f (%.2f%%)\n", coins[2].price, coins[2].change24h);
    } else {
      Serial.println("JSON parse error");
    }
  } else {
    Serial.printf("HTTP error: %d\n", code);
  }

  http.end();
  lastFetch = millis();
}

// ─────────────────────────────────────────────────────────────
void drawCoinScreen(int idx, int offsetX) {
  display.clearDisplay();

  Coin& c = coins[idx];
  if (!c.fetched) {
    display.setTextSize(1);
    display.setCursor(20, 28);
    display.print("Fetching...");
    display.display();
    return;
  }

  // ── Header bar ──────────────────────────────────────────
  display.fillRect(0, 0, SCREEN_WIDTH, 14, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(4 + offsetX, 3);
  display.print(c.symbol);
  display.print("/USD");

  // Dot indicators (which coin is showing)
  for (int i = 0; i < NUM_COINS; i++) {
    int dotX = SCREEN_WIDTH - (NUM_COINS - i) * 10;
    if (i == idx) {
      display.fillCircle(dotX, 7, 3, SSD1306_BLACK);
    } else {
      display.drawCircle(dotX, 7, 3, SSD1306_BLACK);
    }
  }

  display.setTextColor(SSD1306_WHITE);

  // ── Price (large) ───────────────────────────────────────
  display.setTextSize(2);
  String priceStr = formatPrice(c.price);
  int16_t tx, ty; uint16_t tw, th;
  display.getTextBounds(priceStr, 0, 0, &tx, &ty, &tw, &th);
  display.setCursor((SCREEN_WIDTH - tw) / 2 + offsetX, 20);
  display.print(priceStr);

  // ── 24h change ──────────────────────────────────────────
  display.setTextSize(1);
  String changeStr = (c.change24h >= 0 ? "+" : "") + String(c.change24h, 2) + "%";
  display.getTextBounds(changeStr, 0, 0, &tx, &ty, &tw, &th);

  // Arrow triangle
  if (c.change24h >= 0) {
    // up arrow
    display.fillTriangle(
      SCREEN_WIDTH/2 - tw/2 - 8 + offsetX, 52,
      SCREEN_WIDTH/2 - tw/2 - 2 + offsetX, 44,
      SCREEN_WIDTH/2 - tw/2 + 2 + offsetX, 52,
      SSD1306_WHITE
    );
  } else {
    // down arrow
    display.fillTriangle(
      SCREEN_WIDTH/2 - tw/2 - 8 + offsetX, 44,
      SCREEN_WIDTH/2 - tw/2 - 2 + offsetX, 52,
      SCREEN_WIDTH/2 - tw/2 + 2 + offsetX, 44,
      SSD1306_WHITE
    );
  }
  display.setCursor(SCREEN_WIDTH/2 - tw/2 + 6 + offsetX, 46);
  display.print(changeStr);

  // ── Bottom: last updated ─────────────────────────────────
  display.setTextSize(1);
  display.setCursor(0 + offsetX, 57);
  display.print("upd ");
  display.print((millis() - lastFetch) / 1000);
  display.print("s ago");

  display.display();
}

// ─────────────────────────────────────────────────────────────
String formatPrice(float price) {
  if (price >= 1000) {
    return "$" + String((int)price);   // e.g. $67432
  } else if (price >= 1) {
    return "$" + String(price, 2);     // e.g. $183.45
  } else {
    return "$" + String(price, 4);     // sub-dollar coins
  }
}

// ─────────────────────────────────────────────────────────────
void showBootScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(14, 10);
  display.print("CRYPTO");
  display.setTextSize(1);
  display.setCursor(30, 34);
  display.print("TICKER v1.0");
  display.setCursor(20, 50);
  display.print("ESP32 + SSD1306");
  display.display();
  delay(1500);
}

// ─────────────────────────────────────────────────────────────
void showFetchingScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(28, 24);
  display.print("Updating...");
  display.setCursor(16, 38);
  display.print("fetching prices");
  display.display();
}
