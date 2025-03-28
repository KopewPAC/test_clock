// Core: Arduino IDE - ESP32 1.0.6

#include "sett.h"


void setup() {
  //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  // WDT Power
  btInit();
  crm.version(C_VER);
  crm.contacts(F("crm.dev@bk.ru"), F("user624"), F("https://t.me/s/CRMdevelop"));
  crm.begin(F("WiFi-CLOCK 3"), interface, update, api, 115200);
  crm.setApiKey(crm.var(F("APIK")));
  crm.setWebAuth(crm.var(F("WEBL")), crm.var(F("WEBP")));

  Wire.begin(SDA_PIN, SCL_PIN);
  alarmInit();
  timeInit();
  ledInit();
  mp3Init();
}


void loop() {
  apiRunMessage();

  weatherRun();
  sendNarMon();
  narodmonRun();
  checkNewFW();
  fwHttpUpdate();

  dbgRun();
}


void checkNewFW() {
  if (CNFW && fw.check && !fw.newVer && mutex && WiFi.status() == WL_CONNECTED) {
    Serial.println(F("[FW] Поиск новой версии"));
    fw.check = 0;
    mutex = 0;
    WiFiClientSecure *client = new WiFiClientSecure;
    if (client) {
      client->setInsecure();
      HTTPClient h;
      h.begin(*client, F("https://raw.githubusercontent.com/WonderCRM/wondercrm.github.io/main/manifest_32.json"));
      h.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
      int hCode = h.GET();
      String l(F("[FW] Код ответа: "));
      l += hCode;
      Serial.println(l);
      if (hCode == 200) {
        String pl(h.getString());
        DynamicJsonDocument doc(1000);
        DeserializationError e = deserializeJson(doc, pl);
        if (e) {
          if (dbg) {
            Serial.print(F("[FW] Ошибка DeserializeJson: "));
            Serial.println(e.c_str());
          }
        } else {
          const char* v = doc["version"];
          String gitV(v);
          gitV.replace(".", "");
          String localV(C_VER);
          localV.replace(".", "");
          if (gitV.toInt() > localV.toInt()) {
            fw.newVer = 1;
            fw.str = v;
            String l(F("[FW] Найдена новая версия ПО: "));
            l += v;
            Serial.println(l);
            crm.webUpdate();
          } else Serial.println(F("[FW] Версия ПО актуальна"));
        }
      }
      h.end();
    }
    if (dbg) Serial.println(F("[FW] Конец"));
    delete client;
    mutex = 1;
  }
}


void fwHttpUpdate() {
  if ((fw.update || FWAU) && fw.newVer && mutex && WiFi.status() == WL_CONNECTED) {
    Serial.println(F("[FW UPDATE] Запуск обновления"));
    mutex = 0;
    WiFiClientSecure *client = new WiFiClientSecure;
    if (client) {
      client->setInsecure();
      crm.webNotif("i", F("Запущено обновление прошивки.<br>Процесс может занять несколько минут."), 120, 1);
      httpUpdate.setLedPin(2, HIGH);
      httpUpdate.update(*client, "https://raw.githubusercontent.com/WonderCRM/wondercrm.github.io/main/fw/32/0x10000_WiFi-CLOCK_3.bin");
    }
    delete client;
    if (dbg) Serial.println(F("[FW UPDATE] Конец"));
    mutex = 1;
  }
}
