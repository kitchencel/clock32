#include <lvgl.h>
#include <LovyanGFX.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "env.h"
#include "ui_main.h"

/*     NOTE TO SELF
init screen1:    void ui_main_screen_init(void);
destroy screen1: void ui_main_screen_destroy(void);
objects: 
- ui_Screen1
- ui_time
- ui_date
- ui_wifiStrength
- ui_quote
TO BE REMOVED IN PROD
*/

static lv_display_t* display;
const long utc_offset = 1 * 3600;  // if your timezone is UTC, make it 0. (x * 60 * 60, where x is the amount of hours)
const long dst_offset = 2 * 3600;  // if your timezone is UTC, make it 1. (x * 60 * 60, where x is the amount of hours)
const char* ntp = "pool.ntp.org";

String city = "London";
const char* weather_api_adress = "http://api.weatherapi.com";
String weather_api_key = "2ea72f609a294d61a6b125325261502";
const unsigned int weather_api_port = 80;

WiFiClient wifi;
HTTPClient http;

void setup() {
  Serial.begin(6700);

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, KEY);

  String url = String(weather_api_adress) + "?key=" + weather_api_key + "&q=" + city + "&days=9&aqi=no&alerts=no";

  http.begin(url);          // Start connection
  int httpCode = http.GET(); // Send GET request

  if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.println(payload);
      }
  } else {
      Serial.print("HTTP error: ");
      Serial.println(http.errorToString(httpCode));
  }

  http.end();

  configTime(utc_offset, dst_offset, ntp);

  // init LVGL, timer, and display
  lv_init();
  lv_tick_set_cb([]() -> uint32_t {
    return millis();
  });
  display = lv_display_create(800, 480);

  // init buffer
  static uint8_t buf[800 * 480 / 10 * 2];
  lv_display_set_buffers(display, buf, NULL, 800 * 480 / 10, LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(display, dummy_flush_cb);  // feed the data to a dummy

  ui_main_screen_init();  // Finally, it's loading!!!

  scheduleClockUpdate();
}

void updateScreen() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }

    // Time and date
    lv_label_set_text_fmt(ui_time, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    lv_label_set_text_fmt(ui_date, "%02d/%02d", timeinfo.tm_mday, timeinfo.tm_mon + 1);

    // Wi-Fi RSSI
    int rssi = WiFi.RSSI();
    lv_color_t color = lv_color_hex(0xFFFFFF);

    if (rssi >= -70) {
        color = lv_color_hex(0x018A01);
    } else if (rssi >= -80) {
        color = lv_color_hex(0xE8D66F);
    } else if (rssi >= -90) {
        color = lv_color_hex(0xFF8728);
    } else {
        color = lv_color_hex(0xE8132B);
    }

    lv_obj_set_style_text_color(ui_wifiStrength, color, 0);
    lv_label_set_text_fmt(ui_wifiStrength, "%d dBm", rssi);
}

void scheduleClockUpdate() {
  updateScreen();  // first immediate update

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  int seconds_to_next_minute = 60 - timeinfo.tm_sec;

  // One-shot timer until next full minute
  lv_timer_create([](lv_timer_t* t) {
    updateScreen();

    // Now create a repeating timer every 60 seconds
    lv_timer_create([](lv_timer_t* t) {
      updateScreen();
    },
                    60000, nullptr);
  },
                  seconds_to_next_minute * 1000, nullptr);
}

void dummy_flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_buf) {
  Serial.printf("Flush: x%d-%d y%d-%d\n",
                area->x1, area->x2,
                area->y1, area->y2);

  lv_display_flush_ready(disp);
}

void loop() {
  lv_timer_handler();  // handles labels, timers, events
  delay(5);
}