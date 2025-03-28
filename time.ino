
void timeInit() {
  delay(100);
  timeTicTicker.attach_ms(1000, timeTic);
  timeSync();
  if (RTCm) RTCtoLocalTime();
}



void timeSync() {
  uint32_t pause = 21600000UL;
  auto s = WiFi.status();
  if (s == WL_CONNECTED) {
    Serial.println(F("[CLOCK] Синхронизация с NTP"));
    configTime(TZ * 60, 0, crm.var("NTP").c_str(), "pool.ntp.org", "time.nist.gov");
    if (TD.tm_year < 120) pause = 10000UL;
    else if (RTCm) LocalTimeToRTC();
  } else {
    if (RTCm && millis() > _WT) {
      Serial.println(F("[CLOCK] Синхронизация с внешним RTC"));
      RTCtoLocalTime();
    } else pause = 5000UL;
  }
  timeSyncTicker.once_ms(pause, timeSync);
}


void timeTic() {
  getLTime(&TD);
  checkNightMode();

  uint32_t sec = TD.tm_hour * 3600UL + TD.tm_min * 60 + TD.tm_sec;

  if (crm.webConnStatus()) {
    crm.webUpdate("TM", curTime(1, 0, 0));
    crm.webUpdate("DT", curData(1));
    crm.webUpdate("BRNES", String(Bright));
    crm.webUpdate("ANALOG", String(BrightRAW));
    if (sec == 0) {
      fw.check = 1;
      String b("");
      if (MYHD) b += myHoliday(0);
      if (RGHD) {
        if (b != "") b += "  ";
        b += holiday(RGHD, TD.tm_year + 1900, TD.tm_mon + 1, TD.tm_mday, TD.tm_wday, 0);
      }
      if (b == "") b += (TD.tm_wday < 6 ? "Обычный день" : "Выходной");
      crm.webUpdate("HDV", b);
    }
  }
  crm.run(true);

  ledTimeCorrect = millis();

  // Ежечасный пик
  if (HPIK && !NightMode && TD.tm_min == 0 && TD.tm_sec == 0) {
    if (HPIK == 1) tone32(3000, 50);
    else if (HPIK == 2) speechTime(2);
    else if (HPIK == 3) speechTime(3);
    else if (HPIK == 4) speechTime(4);
  }

  // Будильник
  alarmRun(sec);

}


bool getLTime(struct tm * info) {
  time_t now;
  time(&now);
  localtime_r(&now, info);
  if (info->tm_year > 120) {
    return true;
  }
  return false;
}


void RTCtoLocalTime() {
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
  if (year > 2100 || month > 12 || hour > 24) return;
  setLocalTime(year, month, day, hour, minute, second);
}


void LocalTimeToRTC() {
  if (!getLTime(&TD)) return;
  Wire.beginTransmission(RTC_CLOCK_ADRESS);
  Wire.write(byte(0x00));
  Wire.write(decToBcd(TD.tm_sec));
  Wire.write(decToBcd(TD.tm_min));
  Wire.write(decToBcd(TD.tm_hour));
  Wire.write(decToBcd(TD.tm_wday));
  Wire.write(decToBcd(TD.tm_mday));
  Wire.write(decToBcd(TD.tm_mon + 1));
  Wire.write(decToBcd(TD.tm_year - 100));
  Wire.endTransmission();
}


void setLocalTime(String s) {
  uint8_t second, minute, hour, day, month;
  uint16_t year;
  year = s.substring(0, 4).toInt();
  month = s.substring(5, 7).toInt();
  day = s.substring(8, 10).toInt();
  hour = s.substring(11, 13).toInt();
  minute = s.substring(14, 16).toInt();
#ifdef DBG
  Serial.printf("[SET TIME] Set: %d.%d.%d %d:%d\n", year, month, day, hour, minute);
#endif
  setLocalTime(year, month, day, hour, minute, 0);
  delay(1000);
  LocalTimeToRTC();
  crm.webNotif("i", "Дата и время обновлены<br>" + curData(1) + " " + curTime(1, 0, 0), 10, 1);
}


void setLocalTime(uint16_t Y, uint8_t M, uint8_t D, uint8_t h, uint8_t m, uint8_t s) {
  time_t now = time(nullptr);
  tm* newTime = localtime(&now);
  newTime->tm_year = Y - 1900;
  newTime->tm_mon = M - 1;
  newTime->tm_mday = D;
  newTime->tm_hour = h;
  newTime->tm_min = m;
  newTime->tm_sec = s;
  now = mktime(newTime);
  timeval tv = {now, 0};          // в сек
  timezone tz = {TZ, 0};          // в мин
  settimeofday(&tv, &tz);
  getLTime(&TD);
}



void setTimeZone(long offset) { //sec
  int daylight = 0; //зимнее время
  char cst[17] = {0};
  char cdt[17] = "DST";
  char tz[33] = {0};

  if (offset % 3600) {
    sprintf(cst, "UTC%ld:%02u:%02u", offset / 3600, abs((offset % 3600) / 60), abs(offset % 60));
  } else {
    sprintf(cst, "UTC%ld", offset / 3600);
  }
  if (daylight != 3600) {
    long tz_dst = offset - daylight;
    if (tz_dst % 3600) {
      sprintf(cdt, "DST%ld:%02u:%02u", tz_dst / 3600, abs((tz_dst % 3600) / 60), abs(tz_dst % 60));
    } else {
      sprintf(cdt, "DST%ld", tz_dst / 3600);
    }
  }
  sprintf(tz, "%s%s", cst, cdt);
  setenv("TZ", tz, 1);
  tzset();
}


void checkNightMode() {
  if (NM) {
    int HM = TD.tm_hour * 60 + TD.tm_min;
    if (NMS <= NME) {
      if (HM >= NMS && HM < NME) NightMode = true;
      else NightMode = false;
    } else {
      if ((HM >= max(NMS, 0) && HM <= 1439) || (HM >= 0 && HM < max(NME, 0))) NightMode = true;
      else NightMode = false;
    }
  } else NightMode = false;
}


String curTime(bool sec, bool zero, bool mig) {
  getLTime(&TD);
  String s = String();
  if (zero && TD.tm_hour < 10) s = "0";
  s += String(TD.tm_hour);
  s += mig ? (TD.tm_sec % 2 == 0 ? ":" : " ") : ":";
  if (TD.tm_min < 10) s += "0";
  s += String(TD.tm_min);
  if (sec) {
    s += mig ? (TD.tm_sec % 2 == 0 ? ":" : " ") : ":";
    if (TD.tm_sec < 10) s += "0";
    s += String(TD.tm_sec);
  }
  return s;
}


String curData(bool y) {
  String d = weekDay() + ", " + String(TD.tm_mday) + " " + fullMonth();
  if (y) d += " " + String(TD.tm_year + 1900) + " г.";
  return d;
}


String weekDay() {
  switch (TD.tm_wday) {
    case 0: return "Воскресенье";
    case 1: return "Понедельник";
    case 2: return "Вторник";
    case 3: return "Среда";
    case 4: return "Четверг";
    case 5: return "Пятница";
    case 6: return "Суббота";
    default: return "";
  }
}


String fullMonth() {
  switch (TD.tm_mon + 1) {
    case 1: return "января";
    case 2: return "февраля";
    case 3: return "марта";
    case 4: return "апреля";
    case 5: return "мая";
    case 6: return "июня";
    case 7: return "июля";
    case 8: return "августа";
    case 9: return "сентября";
    case 10: return "октября";
    case 11: return "ноября";
    case 12: return "декабря";
    default: return "";
  }
}


uint32_t timeToSec(const String & t) {
  return t.substring(0, 2).toInt() * 3600 + t.substring(3, 5).toInt() * 60 + t.substring(6).toInt();
}


byte bcdToDec(byte val) {
  return ((val / 16 * 10) + (val % 16));
}

byte decToBcd(byte val) {
  return ((val / 10 * 16) + (val % 10));
}
