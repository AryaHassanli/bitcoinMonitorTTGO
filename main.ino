#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include <Button2.h>
#include <EEPROM.h>
#include <HTTPClient.h>
#include <string>

using namespace std;

#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN 0x10
#endif

#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS 5
#define TFT_DC 16
#define TFT_RST 23

#define TFT_BL 4
#define BUTTON_1 35
#define BUTTON_2 0

char ssid[20]="MyFi";
char pass[20]="1375/04/03";

TFT_eSPI tft = TFT_eSPI(135, 240);

void espDelay(int ms)
{
    esp_sleep_enable_timer_wakeup(ms * 1000);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_light_sleep_start();
}

void waitForConnection()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(1);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        tft.setTextSize(2);
        tft.drawStringDelayed("Searching", TFT_WIDTH / 2, 80);
        tft.fillRect(0, 90, 135, 20, TFT_BLACK);
        tft.drawStringDelayed(" . ", TFT_WIDTH / 2, 100);
        espDelay(100);
        tft.fillRect(0, 90, 135, 20, TFT_BLACK);
        tft.drawStringDelayed(" ..", TFT_WIDTH / 2, 100);
        espDelay(100);
        tft.fillRect(0, 90, 135, 20, TFT_BLACK);
        tft.drawStringDelayed("...", TFT_WIDTH / 2, 100);
        espDelay(100);
    }
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.drawStringDelayed("Connected", TFT_WIDTH / 2, TFT_HEIGHT / 2);
    espDelay(500);
    tft.fillScreen(TFT_BLACK);
}


void setup()
{
    Serial.begin(115200);
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(0, 0);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);

    if (TFT_BL > 0)
    {                                           
        pinMode(TFT_BL, OUTPUT);                
        digitalWrite(TFT_BL, TFT_BACKLIGHT_ON); 
    }

    tft.setSwapBytes(true);
    espDelay(1000);
    tft.setRotation(3);
    tft.setTextColor(TFT_GREEN);
    tft.textbgcolor = TFT_BLACK;
    waitForConnection();
}

void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        http.begin("https://cex.io/api/order_book/BTC/EUR/?depth=1");
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        int httpResponseCode = http.GET();
        if (httpResponseCode > 0)
        {
            int a;
            double b,c,t;
            String response = http.getString();
            sscanf(response.c_str(), "{\"timestamp\":%d,\"bids\":[[%lf,%lf]],\"asks\":[[%lf", &a, &b,&t,&c);
            char out1[10];
            char out2[10];
            sprintf(out1, "%.1f", b);
            sprintf(out2, "%.1f", c);
            
            tft.setTextSize(4);
            tft.drawString(out1, TFT_HEIGHT / 2, TFT_WIDTH / 2 - 25);
            tft.drawString(out2, TFT_HEIGHT / 2, TFT_WIDTH / 2 + 25);
            
            Serial.println(httpResponseCode);
            Serial.println(response);
        }
        else
        {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    }
    else
    {
        Serial.println("Error in WiFi connection");
        waitForConnection();
    }
}