#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  // I2C address for the OLED

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define S0 18
#define S1 19
#define S2 23
#define S3 4
#define sensorOut 15
#define SOLENOID_PIN 5  // Pin for solenoid control

#define WIFI_SSID "Turned In"
#define WIFI_PASSWORD "punyazafir."

#define BOT_TOKEN "7185299144:AAFySvM6ZCBkeSooaun18_lGN35AlbeTw3k"
#define CHAT_ID "2029463653"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

int Red = 0;
int Green = 0;
int Blue = 0;
bool statusUang = false;
bool msg = false;
int Uang = 0;

void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);
  pinMode(SOLENOID_PIN, OUTPUT);
  digitalWrite(SOLENOID_PIN, LOW); // Ensure solenoid is off initially
  Serial.begin(115200);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, HIGH);

  Serial.print("Connecting to WiFi SSID: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  configTime(0, 0, "pool.ntp.org");
  while (time(nullptr) < 24 * 3600) {
    delay(100);
  }

  bot.sendMessage(CHAT_ID, "Bot mulai bos!", "");

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.display();
  delay(2000);
  display.clearDisplay();
  display.display();
}

void loop() {
  displayMessage("Infaq Yuk!", 2, 5, 0);

  Red = getColorValue(LOW, LOW);
  Green = getColorValue(HIGH, HIGH);
  Blue = getColorValue(LOW, HIGH);

  Serial.print("Red Freq = ");
  Serial.print(Red);
  Serial.print("   Green Freq = ");
  Serial.print(Green);
  Serial.print("   Blue Freq = ");
  Serial.println(Blue);

  checkAndUpdateUang();

  if (statusUang && !msg) {
    Serial.println(Uang);
    displayMessage("Jazakllahu\nKhairan!", 2, 5, 0);
    displayMessage("Saldo: " + String(Uang), 2, 5, 40);

    bot.sendMessage(CHAT_ID, "Saldo Infaq Hari Ini : " + String(Uang));

    msg = true;
  }

  displayMessage("Saldo Infaq", 1, 0, 0);
  displayMessage(String(Uang), 2, 0, 20);

  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    Serial.println("New message received");
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }

  delay(2000);
}

int getColorValue(int s2State, int s3State) {
  digitalWrite(S2, s2State);
  digitalWrite(S3, s3State);
  return pulseIn(sensorOut, LOW);
}

void displayMessage(String message, int textSize, int x, int y) {
  display.clearDisplay();
  display.setTextSize(textSize);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(x, y);
  display.println(message);
  display.display();
}

void checkAndUpdateUang() {
  if (isInRange(Red, 35, 40) && isInRange(Green, 32, 37) && isInRange(Blue, 25, 30) && !statusUang) {
    Serial.println("2rb");
    Uang += 2000;
    statusUang = true;
  } else if (isInRange(Red, 20, 28) && isInRange(Green, 25, 35) && isInRange(Blue, 25, 35) && !statusUang) {
    Serial.println("5rb");
    Uang += 5000;
    statusUang = true;
  } else if (isInRange(Red, 30, 37) && isInRange(Green, 35, 40) && isInRange(Blue, 25, 30) && !statusUang) {
    Serial.println("10rb");
    Uang += 10000;
    statusUang = true;
  } else if (isInRange(Red, 22, 27) && isInRange(Green, 18, 23) && isInRange(Blue, 17, 22) && !statusUang) {
    Serial.println("20rb");
    Uang += 20000;
    statusUang = true;
  } else if (isInRange(Red, 46, 55) && isInRange(Green, 33, 38) && isInRange(Blue, 25, 30) && !statusUang) {
    Serial.println("50rb");
    Uang += 50000;
    statusUang = true;
  } else if (isInRange(Red, 17, 22) && isInRange(Green, 23, 29) && isInRange(Blue, 19, 25) && !statusUang) {
    Serial.println("100rb");
    Uang += 100000;
    statusUang = true;
  } else if (Red > 100 && Green > 100 && Blue > 100) {
    statusUang = false;
    msg = false;
  }
}

bool isInRange(int value, int min, int max) {
  return value >= min && value <= max;
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    Serial.println("Received message: " + text);

    if (text == "/open") {
      digitalWrite(SOLENOID_PIN, HIGH); // Turn on solenoid
      bot.sendMessage(chat_id, "Solenoid is now OPEN.", "");
    } else if (text == "/close") {
      digitalWrite(SOLENOID_PIN, LOW); // Turn off solenoid
      bot.sendMessage(chat_id, "Solenoid is now CLOSED.", "");
    } else {
      bot.sendMessage(chat_id, "Unknown command. Use /open or /close.", "");
    }
  }
}