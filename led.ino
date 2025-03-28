void ledInit() {
  pinMode(ANALOG_PIN, INPUT);
  brightTicker.attach_ms(50, BrightRun);
  M.begin();                              //Иициализация панели
  M.displayClear();                       //Очистить дисплей
  M.setInvert(false);                     //Инверсия панели
  M.setIntensity(Bright);                 //Яркость панели 0-15
  mFont(0);
  M.displayAnimate();
  sMode = 100;
  xTaskCreatePinnedToCore(ledRun, "ledRun", 6000, NULL, 1, NULL, 1); //tskNO_AFFINITY
}


void ledRun(void *pvParameters) {
  //TickType_t xLastWakeTime = xTaskGetTickCount();
  int kucha = uxTaskGetStackHighWaterMark(NULL);

  for (;;) {
    if (M.displayAnimate()) {
      static uint16_t tv_count = 1;

      switch (sMode) {
        // Время 0-2
        case 0: {
            mFont(STYLE);
            curTime(SEC, DBLH, PMIG).toCharArray(data, LED_MAX_BUF);
            byte e = random(0, ARRAY_SIZE(catalog));
            M.displayClear();
            M.displayText(data, PA_CENTER, map(TSPD, 1, 20, catalog[e].speed, 1), 0, catalog[e].effect, PA_NO_EFFECT);
            tv_count = 1;
            sMode++;
          } break;
        case 1: {
            mFont(STYLE);
            curTime(SEC, DBLH, PMIG).toCharArray(data, LED_MAX_BUF);
            int p = constrain((1000 - (millis() - ledTimeCorrect)), 500, 1000);
            M.displayText(data, PA_CENTER, 0, p, PA_PRINT, PA_NO_EFFECT);
            if (tv_count >= VIEWT) {
              tv_count = 1;
              sMode++;
            } else if (!NightMode && serviceState()) tv_count++;
          } break;
        case 2: {
            mFont(STYLE);
            curTime(SEC, DBLH, PMIG).toCharArray(data, LED_MAX_BUF);
            byte e = random(0, ARRAY_SIZE(catalog));
            M.displayText(data, PA_CENTER, map(TSPD, 1, 20, catalog[e].speed, 1), 0, PA_PRINT, catalog[e].effect);
            sMode = 10;
          } break;

        // Сервисы
        case 10: {
            static uint8_t serviceCount = 0;
            switch (serviceCount) {

              // Дата и праздники
              case 0: {
                  if (DAT) {
                    mFont(0);
                    String s(curData(DATY));
                    if (MYHD) s += myHoliday(1);
                    if (RGHD) s += holiday(RGHD, TD.tm_year + 1900, TD.tm_mon + 1, TD.tm_mday, TD.tm_wday, 1);
                    utf2rus(s).toCharArray(data, LED_MAX_BUF);
                    M.displayScroll(data, PA_LEFT, PA_SCROLL_LEFT, SSPD);
                    sMode = 0;
                  }
                  serviceCount++;
                } break;

              // Внешние датчики
              case 1: {
                  if (ESENS) {
                    mFont(0);
                    utf2rus(sensorString(1)).toCharArray(data, LED_MAX_BUF);
                    M.displayScroll(data, PA_LEFT, PA_SCROLL_LEFT, SSPD);
                    sMode = 0;
                  }
                  serviceCount++;
                } break;

              // Погода
              case 2: {
                  if (WTHR > 0) {
                    mFont(0);
                    utf2rus(weatherString(1)).toCharArray(data, LED_MAX_BUF);
                    M.displayScroll(data, PA_LEFT, PA_SCROLL_LEFT, SSPD);
                    sMode = 0;
                  }
                  serviceCount++;
                } break;

              // Народный мониторинг
              case 3: {
                  if (NMON) {
                    mFont(0);
                    utf2rus(narmonString(1)).toCharArray(data, LED_MAX_BUF);
                    M.displayScroll(data, PA_LEFT, PA_SCROLL_LEFT, SSPD);
                    sMode = 0;
                  }
                  serviceCount = 0;
                } break;

              default:
                sMode = 0;
                serviceCount = 0;
                break;
            }
          } break;


        // ЛОГО
        case 100: {
            String s(F("WiFi-ЧАСЫ "));
            s += C_VER;
            s += F("  by CRM/DEV");
            utf2rus(s).toCharArray(data, LED_MAX_BUF);
            M.displayScroll(data, PA_LEFT, PA_SCROLL_LEFT, 33);
            sMode++;
          } break;

        // WiFi
        case 101: {
            if (WiFi.status() == WL_CONNECTED) {
              mFont(0);
              String s(F("  Подключение к WiFi сети "));
              s += WiFi.SSID().c_str();
              s += F(" выполнено! IP адрес устройства ");
              s += WiFi.localIP().toString();
              utf2rus(s).toCharArray(data, LED_MAX_BUF);
              M.displayScroll(data, PA_LEFT, PA_SCROLL_LEFT, 33);
              sMode = 0;
            } else {
              static bool c_init = false;
              static byte wm = crm.var("_wm").toInt();
              static int timer = 0;
              if (!c_init) {
                c_init = true;
                timer = crm.var("_wt").toInt() - 7;
                setMBright(0);
              }
              if (timer && wm != 2) {
                static byte i = 33;
                mFont(2);
                String s((char)i);
                s += --timer;
                i < 36 ? i++ : i = 33;
                s.toCharArray(data, LED_MAX_BUF);
                M.displayText(data, PA_CENTER, 0, 1000, PA_PRINT, (timer ? PA_NO_EFFECT : PA_CLOSING));
              } else {
                mFont(0);
                String s(F("  Подключитесь к WiFi: "));
                s += crm.var("_as");
                s += F(" и откройте в браузере адрес http://192.168.4.1 для настройки. ");
                utf2rus(s).toCharArray(data, LED_MAX_BUF);
                M.displayScroll(data, PA_LEFT, PA_SCROLL_LEFT, 33);
                sMode = 0;
              }
            }
          } break;


        default:
          sMode = 0;
          break;

      }
    }

    vTaskDelay(5);
    //vTaskDelayUntil(&xLastWakeTime, 5);

    if (dbg && kucha > uxTaskGetStackHighWaterMark(NULL)) {
      kucha = uxTaskGetStackHighWaterMark(NULL);
      String l(F("[HEAP] ledRun: "));
      l += kucha;
      Serial.println(l);
    }
  }
}


bool serviceState() {
  return (DAT || ESENS || WTHR || NMON);
}


void message(const String &m, byte e, uint32_t t) {
  fade = true;
  while (Bright) delay(10);
  M.displayClear();
  mFont(0);
  utf2rus(m).toCharArray(data, LED_MAX_BUF);
  if (e) M.displayText(data, PA_CENTER, map(TSPD, 1, 20, catalog[e].speed, 1), t, catalog[e].effect, catalog[e].effect);
  else M.displayScroll(data, PA_LEFT, PA_SCROLL_LEFT, SSPD);
  BrightCorrect = true;
  sMode = 0;
}
void message(const String &m) {
  message(m, 0, 0);
}


void mFont(uint8_t n) {
  if (n == 0) M.setFont(CRMrusTxt);
  else if (n == 1) M.setFont(CRMrus1Small);
  else if (n == 2) M.setFont(CRMrus2Big);
  else if (n == 3) M.setFont(CRMrus3Big);
  else if (n == 4) M.setFont(CRMrus4Big);
  else if (n == 5) M.setFont(CRMrus5Big);
  else if (n == 6) M.setFont(CRMrus6Big);
  else if (n == 7) M.setFont(CRMrus7Big);
  else if (n == 8) M.setFont(CRMrus2Small);
  else if (n == 9) M.setFont(CRMrus8Big);
  else if (n == 10) M.setFont(CRMrus9);
  else if (n == 11) M.setFont(num1micro);
  else if (n == 12) M.setFont(CRMrus10);
  else M.setFont(CRMrusTxt);
}



// Управление яркостью матрицы
void setMBright(byte b) {
  Bright = constrain(b, 0, 15);
  BrightCorrect = true;
}



void BrightRun() {
  static uint16_t BrightRAWOld = 0;
  BrightRAW = f_Median(f_expRA(analogRead(ANALOG_PIN)));
  if (BrightCorrect || abs(BrightRAW - BrightRAWOld) >= ASENS || fade || !MMBR) {
    BrightRAWOld = BrightRAW;
    BrightCorrect = true;
    uint8_t BrightNew = (fade ? 0 : (MMBR ? map(constrain(BrightRAW, SMIN, SMAX), SMIN, SMAX, BMIN, BMAX) : (NightMode ? BMIN : BMAX)));
    if (Bright != BrightNew) {
      Bright > BrightNew ? M.setIntensity(--Bright) : M.setIntensity(++Bright);
      if (Bright == BrightNew) fade = BrightCorrect = false;
      //Serial.printf("BR: %d\tBN: %d\tB: %d\n", BrightRAW, BrightNew, Bright);
    } else fade = BrightCorrect = false;
  }
}
/*void BrightRun() {
  if (MMBR > 0) {
    static uint16_t BrightRAWOld = 0;
    BrightRAW = f_Median(f_expRA(analogRead(ANALOG_PIN)));
    if (BrightCorrect || abs(BrightRAW - BrightRAWOld) >= ASENS || fade) {
      BrightRAWOld = BrightRAW;
      BrightCorrect = true;
      uint8_t BrightNew = (fade ? 0 : map(constrain(BrightRAW, SMIN, SMAX), SMIN, SMAX, BMIN, BMAX));
      if (Bright != BrightNew) {
        Bright > BrightNew ? M.setIntensity(--Bright) : M.setIntensity(++Bright);
        if (Bright == BrightNew) fade = BrightCorrect = false;
        //Serial.printf("BR: %d\tBN: %d\tB: %d\n", BrightRAW, BrightNew, Bright);
      } else fade = BrightCorrect = false;
    }
  } else {
    if (NightMode) {
      if (Bright != BMIN) {
        TRACE();
        //Bright > BMIN ? --Bright : ++Bright;
        M.setIntensity(Bright = BMIN);
      }
    } else {
      if (Bright != BMAX) {
        TRACE();
        //Bright < BMAX ? ++Bright : --Bright;
        M.setIntensity(Bright = BMAX);
      }
    }
  }
  }*/


// Фильтры для фоторезистора
uint16_t f_Median(uint16_t newVal) {
  static uint16_t buf[3] = {newVal};
  static byte c = 0;
  buf[c] = newVal;
  if (++c > 2) c = 0;
  return (max(buf[0], buf[1]) == max(buf[1], buf[2])) ? max(buf[0], buf[2]) : max(buf[1], min(buf[0], buf[2]));
}

// ADEEP - коэффициент фильтрации, 0.0-1.0
uint16_t f_expRA(uint16_t newVal) {
  static float filVal = newVal;
  filVal += (newVal - filVal) * ADEEP;
  return round(filVal);
}


//UTF to RUS
String utf2rus(const String & source) {
  int i = 0, k = source.length();
  String target = String();
  unsigned char n;
  char m[2] = { '0', '\0' };
  while (i < k) {
    n = source[i];
    i++;
    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
            n = source[i];
            i++;
            if (n == 0x81) {
              n = 0xA8;
              break;
            }
            if (n >= 0x90 && n <= 0xBF) n += 0x30;
            break;
          }
        case 0xD1: {
            n = source[i];
            i++;
            if (n == 0x91) {
              n = 0xB8;
              break;
            }
            if (n >= 0x80 && n <= 0x8F) n += 0x70;
            break;
          }
      }
    }
    m[0] = n;
    target += m;
  }
  return target;
}
