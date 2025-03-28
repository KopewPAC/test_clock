
void sensorRun() {
  static byte lastSens = 0;
  uint32_t pause = SUDTI;

  // Инициализация
  if (lastSens != ESENS) {

    static bool hi = 1;
    if (hi) {
      for (int i = 0; i < SENSOR_HISTORY_COUNT; i++)
        his->Time[i] = 8888;
      hi = 0;
    }

    // DS18B20
    if (ESENS == 1) {
      pinMode(DS18_PIN, INPUT);
      ds18.begin();
      ds18.setWaitForConversion(false);
      dsCount = ds18.getDeviceCount();
      Serial.print(F("[DS18] Найдено датчиков: "));
      Serial.println(dsCount);
      if (dsCount) {
        lastSens = ESENS;
        sensor.H = sensor.P = sensor.CO2 = 0;
        if (ds18.getResolution() != 12) ds18.setResolution(12);
        pause = 1000UL;

      } else Serial.println(F("[DS18] Датчик не обнаружен."));
    }

    // SCD30
    else if (ESENS == 2) {
      if (crm.upTimeSec() > 10) {
        if (scd30.begin(Wire, true)) {
          delay(200);
          uint16_t a = scd30.getAltitudeCompensation();
          if (a != ALTDE && a < 3000) {
            crm.var("ALTDE", String(a));
            ALTDE = a;
            if (crm.webConnStatus()) crm.webUpdate("ALTDE", String(a));
          }
          delay(100);
          float t = scd30.getTemperatureOffset();
          if (t != T2OFFS && t < 50) {
            crm.var("T2OFFS", String(t, 1));
            T2OFFS = t;
            if (crm.webConnStatus()) crm.webUpdate("T2OFFS", String(t, 1));
          }
          delay(100);
          scd30.beginMeasuring(ALTDE);
          lastSens = ESENS;
          sensor.P = sensor.T2 = 0;
          Serial.println(F("[SCD30] Инициализация успешна."));
        } else Serial.println(F("[SCD30] Датчик не обнаружен."));
      }
    }

    // BME280
    else if (ESENS == 3) {
      auto address = 0x76;
      Wire.beginTransmission(119); //0x77
      if (!Wire.endTransmission()) address = 0x77;
      bme.setFilter(FILTER_COEF_8);                     // коофициент фильтрации
      bme.setTempOversampling(OVERSAMPLING_8);          // передискретизацию для датчика температуры
      bme.setPressOversampling(OVERSAMPLING_16);        // передискретизацию для датчика давления
      bme.setStandbyTime(STANDBY_1000MS);               // время сна между измерениями
      if (bme.begin(address)) {
        lastSens = ESENS;
        sensor.CO2 = sensor.T2 = 0;
        String l(F("[BME280] Инициализация успешна. Адрес датчика: 0x"));
        l += String(address, HEX);
        Serial.println(l);
      } else  Serial.println(F("[BME280] Датчик не обнаружен."));
    }

    // AHT20
    else if (ESENS == 4) {
      if (ath20.begin(Wire)) {
        lastSens = ESENS;
        sensor.P = sensor.T2 = sensor.CO2 = 0;
        Serial.println(F("[AHT20] Инициализация успешна."));
      } else Serial.println(F("[SCD30] Датчик не обнаружен."));
    } else {
      lastSens = ESENS;
    }


  }

  // Опрос датчиков
  else {

    // DS18B20
    if (ESENS == 1) {
      static bool m = 0;
      if (m) {
        sensor.T1 = ds18.getTempCByIndex(0) + TOFFS;
        if (dsCount > 1) {
          sensor.T2 = ds18.getTempCByIndex(1) + TOFFS;
        }
        pause -= 760UL;
        m = 0;
      } else {
        ds18.requestTemperatures();
        pause = 760UL;
        m = 1;
      }
    }

    // SCD30
    else if (ESENS == 2) {
      if (scd30.readMeasurement()) {
        sensor.T1 = scd30.getTemperature();
        sensor.H = scd30.getHumidity() + HOFFS;
        sensor.CO2 = co2filtr2(co2filtr1(scd30.getCO2()));
      }
    }

    // BME280
    else if (ESENS == 3) {
      sensor.T1 = bme.readTemperature() + TOFFS;
      sensor.H = bme.readHumidity() + HOFFS;
      sensor.P = pressureToMmHg(bme.readPressure()) + POFFS;
    }

    // AHT20
    else if (ESENS == 4) {
      if (ath20.available()) {
        sensor.T1 = (ath20.getTemperature()) + TOFFS;
        sensor.H = (ath20.getHumidity()) + HOFFS;
        pause -= 2000;
      } else pause = 2000;
    }


    sensHistoryWrite();

    if (crm.webConnStatus()) {
      String s(sensorString(0));
      crm.webUpdate("SENSV", s);

      static uint32_t chartTimer = 0;
      if (millis() - chartTimer >= 300000) {
        chartTimer = millis();
        crm.webUpdate("HTMP1", String(sensor.T1, 1));
        if (dsCount > 1) crm.webUpdate("HTMP2", String(sensor.T2, 1));
        if (ESENS > 1) crm.webUpdate("HHUM", String(sensor.H, 1));
        if (ESENS == 2 || ESENS == 100) crm.webUpdate("HCO2", String(sensor.CO2));
        if (ESENS == 3 || ESENS == 100) crm.webUpdate("PRES", String(sensor.P));
      }
    }
  }

  if (ESENS) sensorTicker.once_ms(pause, sensorRun);
}


String sensorString(bool onScreen) {
  String s(crm.var("SENTMP"));
  s.replace("%CO2", String(sensor.CO2));
  s.replace("%T11", String(sensor.T1, 1));
  s.replace("%T10", String(sensor.T1, 0));
  s.replace("%T21", String(sensor.T2, 1));
  s.replace("%T20", String(sensor.T2, 0));
  s.replace("%T1", String(sensor.T1, 1));
  s.replace("%T0", String(sensor.T1, 0));
  s.replace("%T", String(sensor.T1, 0));
  s.replace("%H1", String(sensor.H, 1));
  s.replace("%H", String(sensor.H, 0));
  s.replace("%P", String(sensor.P));
  if (onScreen) {
    s.replace(F("°"), F("`"));
    s.replace(F(" "), F(" "));
  }
  return s;
}


void sendNarMon() {
  if (ESENS && SNDNM) {
    static uint32_t timer = 0;
    if (millis() - timer > 299999UL) {
      timer = millis();
      if (WiFi.status() == WL_CONNECTED && mutex) {
        mutex = 0;
        String p("ID=");
        p += WiFi.macAddress();
        p += "&T1=";
        p += String(sensor.T1, 1);
        if (dsCount > 1) {
          p += "&T2=";
          p += String(sensor.T2, 1);
        }
        if (ESENS > 1) {
          p += "&H1=";
          p += String(sensor.H, 1);
        }
        if (ESENS == 3 || ESENS == 100) {
          p += "&BMPP1=";
          p += sensor.P;
        }
        if (ESENS == 2 || ESENS == 100) {
          p += "&CO2=";
          p += sensor.CO2;
        }
        if (dbg) Serial.println(F("[NM] Отправка данных на Народный мониторинг..."));
        WiFiClient client;
        HTTPClient h;
        String u(F("http://narodmon.com/get?"));
        u += p;
        h.begin(client, u);
        int c = h.GET();
        if (dbg) {
          Serial.print(F("[NM] Код ответа: "));
          Serial.println(c);
          Serial.println(h.getString());
        }
        h.end();
        mutex = 1;
        if (dbg) Serial.println(F("\n[NM] Соединение закрыто"));
      }
    }
  }
}


// Среднее арифметическое 5
uint16_t co2filtr2(uint16_t v) {
  static uint16_t a[5] = {400};
  static byte i = 0;
  a[i] = v;
  if (++i > 4) i = 0;
  return ((a[0] + a[1] + a[2] + a[3] + a[4]) / 5);
}
// Медианный фильтр
uint16_t co2filtr1(uint16_t newVal) {
  static uint16_t buf[3] = {newVal};
  static byte count = 0;
  //Serial.println("RAW CO2: " + String(newVal));
  if (newVal < 10000) {
    buf[count] = newVal;
    if (++count >= 3) count = 0;
  }
  return (max(buf[0], buf[1]) == max(buf[1], buf[2])) ? max(buf[0], buf[2]) : max(buf[1], min(buf[0], buf[2]));
}


void sensHistoryWrite() {
  if (SCHRT) {
    static uint32_t timer = 0;
    if (millis() - timer >= 1200000UL || timer == 0) {
      timer = millis();
      int n = SENSOR_HISTORY_COUNT - 1;

      for (int i = 0; i < n; i++) {
        his->Time[i] = his->Time[i + 1];
        his->T1[i] = his->T1[i + 1];
        if (dsCount > 1) his->T2[i] = his->T2[i + 1];
        if (ESENS > 1) his->H[i] = his->H[i + 1];
        if (ESENS == 3 || ESENS == 100) his->P[i] = his->P[i + 1];
        if (ESENS == 2 || ESENS == 100) his->CO2[i] = his->CO2[i + 1];
      }

      his->Time[n] = TD.tm_hour * 60 + TD.tm_min;
      his->T1[n] = sensor.T1 * 10;
      if (dsCount > 1) his->T2[n] = sensor.T2 * 10;
      if (ESENS > 1) his->H[n] = sensor.H * 10;
      if (ESENS == 3 || ESENS == 100) his->P[n] = sensor.P;
      if (ESENS == 2 || ESENS == 100) his->CO2[n] = sensor.CO2;
    }
  }
}


String sensHistoryRead(int d) {
  const String &z(",");
  String s("[");
  for (int i = 0; i < SENSOR_HISTORY_COUNT; i++) {
    if (his->Time[i] == 8888) continue;
    if (!s.endsWith("[")) s += z;
    if (d == 0) {
      s += "\"";
      s += (uint16_t)(his->Time[i] / 60);
      s += ":";
      byte sec = his->Time[i] % 60;
      if (sec < 10) s += "0";
      s += sec;
      s += "\"";
    } else if (d == 1) {
      s += (his->T1[i] * 0.1);
    } else if (d == 2) {
      s += (his->T2[i] * 0.1);
    } else if (d == 3) {
      s += (his->H[i] * 0.1);
    } else if (d == 4) {
      s += his->P[i];
    } else if (d == 5) {
      s += his->CO2[i];
    }
  }
  s += "]";
  return s;
}
