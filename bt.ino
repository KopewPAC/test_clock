void btInit() {
  pinMode(BT_PIN, INPUT); //PULLUP на pin 39 нет
  xTaskCreatePinnedToCore(btRun, "btRun", 3000, NULL, 1, NULL, 1); //tskNO_AFFINITY
}


void btRun(void *pvParameters) {
  int kucha = uxTaskGetStackHighWaterMark(NULL);
  for (;;) {
    //WEB
    if (crm.btnSwStatus()) {
      crm.btnCallback(F("RBT"), btReboot);
      crm.btnCallback(F("FRF"), ForcedRecalibrationFactor);
      crm.btnCallback(F("SETDT"), setDataTime);
      crm.btnCallback(F("BTFW"), btUpdateNow);
      audioBT();
    }

    //HW
    bt.tick();

    if (bt.press()) {
      tone32(3000, 2);
    }

    if (bt.held()) {
      SPLN("Кнопка. Удерживание");
      tone32(3000, 10);
    }

    switch (bt.hasClicks()) {
      case 1: {
          if (alarmBeepActive || alarmMp3PlayStatus) {     // модификация библиотеки Ticker.h
            if (alarmBeepActive) {
              for (byte b = 0; b < ALL_ALARM_COUNT; b++) {
                if (AM[b] && AMACT[b] == 0 && alarmTick[b].status()) {
                  alarmTick[b].detach();
                }
              }
              alarmBeepActive = 0;
            }
            if (alarmMp3PlayStatus) {
              aStop();
              alarmMp3PlayStatus = 0;
            }
            message("ВЫКЛ", 1, 2500);   // MESH
          }

          // Показать строку по шаблону из раздела Система
          else {
            if (sMode == 0) message("");
            else {
              tone32(3000, 10);
              const String &r(F("  ***  "));
              String t(crm.var(F("BTTMP")));
              t.toLowerCase();
              if (t == "") t = F("snw");
              int l = t.length();
              String s("");
              for (byte i = 0; i < l; i++) {
                if (ESENS && t[i] == 's') {
                  if (s != "") s += r;
                  s += sensorString(1);
                  continue;
                }
                if (NMON && t[i] == 'n') {
                  if (s != "") s += r;
                  s += narmonString(1);
                  continue;
                }
                if (WTHR && t[i] == 'w') {
                  if (s != "") s += r;
                  s += weatherString(1);
                  continue;
                }
                if (t[i] == 'd') {
                  if (s != "") s += r;
                  s += curData(DATY);
                  continue;
                }
                if (MYHD && t[i] == 'h') {
                  s += myHoliday((s != "" ? 1 : 0));
                  s += holiday(RGHD, TD.tm_year + 1900, TD.tm_mon + 1, TD.tm_mday, TD.tm_wday, (s != "" ? 1 : 0));
                  continue;
                }
              }
              message((s == "" ? F("Шаблон не задан!") : (s.startsWith(F("  ***  ")) ? s.substring(7) : s)));
            }
          }
        } break;
      case 2: {
          if (mp3InitStatus[0] > 0) speechTime(4);//2
        } break;
      case 3: {
          tone32(3000, 10);
          String s(F("IP адрес "));
          s += WiFi.localIP().toString();
          message(s);
        } break;
      case 4: {
          tone32(3000, 10);
          M.displayClear();
          SPLN("4 клика");
        } break;
      case 5: {
          tone32(3000, 300);
          btReboot();
        } break;
      case 10: {
          tone32(3000, 300);
          crm.cfgDelete();
        } break;
      default: break;
    }

    vTaskDelay(5);
    if (dbg && kucha > uxTaskGetStackHighWaterMark(NULL)) {
      kucha = uxTaskGetStackHighWaterMark(NULL);
      String l(F("[HEAP] btRun: "));
      l += kucha;
      SPLN(l);
    }
  }
}


void btReboot() {
  crm.webNotif("i", F("Часы перезагружаются"), 5, 1);
  crm.espReboot();
}



void btUpdateNow() {
  fw.update = 1;
}


void ForcedRecalibrationFactor() {
  if (ESENS == 2) {
    uint32_t s = crm.upTime(1).toInt();
    if (s > 3600) {
      scd30.setForcedRecalibrationFactor();
      crm.webNotif(F("Green"), F("Калибровка выполена!"), 5, 1);
    } else {
      String m(F("Модуль находится в режиме Автокалибровки.<br>Перекалибровка будет доступна через: "));
      m += timeEnd(3600 - s);
      crm.webNotif("i", m, 10, 1);
    }
  }
}



void setDataTime() {
  setLocalTime(crm.var(F("MDT")));
}



String timeEnd(uint32_t s) {
  String b = String();
  if ((s / 86400 % 365) != 0) {
    b += s / 86400 % 365;
    b += F(" дн,  ");
  }
  b += s / 3600 % 24;
  b += ":";
  if ((s / 60 % 60) < 10) b += "0";
  b += s / 60 % 60;
  b += ":";
  if ((s % 60) < 10) b += "0";
  b += s % 60;
  return b;
}
