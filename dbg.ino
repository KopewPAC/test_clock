
void dbgRun() {
  if (Serial.available()) {
    String s = Serial.readString();
    Serial.println(F("\n[!DBG!] ****************** S T A R T ******************"));
    Serial.print(F("[SERIAL] "));
    Serial.println(s);
    //-------------------------------------------------------

    /*for (byte i = 0; i < ALL_ALARM_COUNT; i++) {
      Serial.print(AMPIN[i]);
      Serial.print("\t");
      Serial.print(digitalRead(AMPIN[i]));
      Serial.print("\t");
      Serial.println(AMFPS[i]);
      }*/

    /*for (int i = 0; i < SENSOR_HISTORY_COUNT; i++) {
      if (i) Serial.print(" ,");
      Serial.print(his->Time[i]);
      }
      Serial.println();
      for (int i = 0; i < SENSOR_HISTORY_COUNT; i++) {
      if (i) Serial.print(" ,");
      Serial.print(his->T1[i] * 0.1);
      }
      Serial.println();
      for (int i = 0; i < SENSOR_HISTORY_COUNT; i++) {
      if (i) Serial.print(" ,");
      Serial.print(his->H[i] * 0.1);
      }
      Serial.println();*/

    //-------------------------------------------------------
    if (s.startsWith(F("dbg"))) {
      dbg = true;
      Serial.println(F("\n[DBG] Advanced debugging enabled"));
    }

    if (dbg) {
      Serial.print(F("\n[RTC sec] "));
      time_t t;
      time(&t);
      Serial.println(t);

      //MUTEX
      Serial.print(F("\n[MUTEX] "));
      Serial.println(mutex);

      //MUTEX
      Serial.print(F("\n[BT PIN] "));
      Serial.println(digitalRead(BT_PIN));

      //RTC модуль
      Wire.beginTransmission(RTC_CLOCK_ADRESS);
      Wire.write(byte(0x00));
      Wire.endTransmission();
      Wire.requestFrom(RTC_CLOCK_ADRESS, 7);
      uint8_t second, minute, hour, dayOfWeek, day, month;
      uint16_t year;
      second = bcdToDec(Wire.read() & 0x7f);
      minute = bcdToDec(Wire.read());
      hour = bcdToDec(Wire.read() & 0x3f);
      dayOfWeek = bcdToDec(Wire.read());
      day = bcdToDec(Wire.read());
      month = bcdToDec(Wire.read());
      year = bcdToDec(Wire.read()) + 2000;
      Wire.beginTransmission(RTC_CLOCK_ADRESS);   // Температура внутреннего датчика в RTC
      Wire.write(0x11);
      Wire.endTransmission();
      Wire.requestFrom(RTC_CLOCK_ADRESS, 2);
      float T = (((int8_t)Wire.read() << 2) + (Wire.read() >> 6)) * 0.25f;
      Serial.printf("\n[RTC Module] %d.%d.%d %d:%d:%d\t\tT: %.1f °C\n", year, month, day, hour, minute, second, T);


      //i2c
      Serial.print(F("\n[I2C]"));
      for (uint8_t i = 1; i < 128; i++) {
        Wire.beginTransmission(i);
        if (!Wire.endTransmission()) {
          Serial.print(F("  0x"));
          Serial.print(i, HEX);
        }
      }
      Serial.println();

      //SCD30
      if (ESENS == 2 && crm.upTimeSec() > 20) {
        Serial.print(F("\n[SCD30]\nИнтервал измерения: "));
        Serial.print(scd30.getMeasurementInterval());
        Serial.print(F(" c\nВысота над уровнем моря: "));
        Serial.print(scd30.getAltitudeCompensation());
        Serial.print(F(" m\nТемпературная компенсация: "));
        Serial.print(scd30.getTemperatureOffset(), 1);
        Serial.print(F(" °C\nАвто калибровка: "));
        Serial.print((scd30.getAutoSelfCalibration() ? F("ВКЛ") : F("ВЫКЛ")));
        Serial.print(F("\nФактор принудительной перекалибровки: "));
        uint16_t ForcedRecalibr;
        scd30.getForcedRecalibration(&ForcedRecalibr);
        Serial.println(ForcedRecalibr);
      }

      //EXTERNAL SENSOR
      Serial.print(F("\n[SENSOR] CO2: "));
      Serial.print(sensor.CO2);
      Serial.print(F(" ppm  |  T1: "));
      Serial.print(sensor.T1, 1);
      Serial.print(F(" °C  |  T2: "));
      Serial.print(sensor.T2, 1);
      Serial.print(F(" °C   |  H: "));
      Serial.print(sensor.H, 1);
      Serial.print(F(" %  |  P: "));
      Serial.print(sensor.P);
      Serial.println(F(" mm рт.ст."));

      Serial.print(F("\n     "));
      Serial.println(sensHistoryRead(0));
      Serial.print(F(" T1: "));
      Serial.println(sensHistoryRead(1));
      Serial.print(F(" T2: "));
      Serial.println(sensHistoryRead(2));
      Serial.print(F("  H: "));
      Serial.println(sensHistoryRead(3));
      Serial.print(F("  P: "));
      Serial.println(sensHistoryRead(4));
      Serial.print(F("CO2: "));
      Serial.println(sensHistoryRead(5));


      // DFPlayer
      Serial.print(F("\n[MP3] Инициализация DFplayer: "));
      Serial.println((mp3InitStatus[0] ? 1 : 0));
      Serial.print(F("[MP3] Статус воспроизведения: "));
      Serial.println(!digitalRead(DFPlayerBusyu_PIN));
      if (mp3InitStatus[0]) {
        Serial.print(F("[MP3] Всего файлов на SD карте: "));
        Serial.print(mp3InitStatus[0]);
        Serial.print(F(" / "));
        Serial.println(dfPlayer.readFileCounts());
        for (int i = 1; i < 12; i++) {
          if (mp3InitStatus[i]) {
            String a(F("      папка "));
            if (i < 10) a += F("0");
            a += i;
            a += F(": ");
            a += mp3InitStatus[i];
            Serial.println(a);
          }
        }
      }

      Serial.print(F("\n[!DBG!] ******************** E N D ********************\n\n"));
    }
  }
}
