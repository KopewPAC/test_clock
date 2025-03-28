void update() {
  static bool firstStart = false;

  { // Система
    CNFW = crm.var("CNFW") == "1";
    FWAU = crm.var("FWAU") == "1";
  }


  { // Время
    int _TZ = TZ;
    TZ = crm.var("TZ").toInt();
    if (_TZ != TZ) setTimeZone(-60 * TZ);
    NM = crm.var("NM") == "1";
    NMS = (crm.var("NMS")).substring(0, 2).toInt() * 60 + (crm.var("NMS")).substring(3, 5).toInt();
    NME = (crm.var("NME")).substring(0, 2).toInt() * 60 + (crm.var("NME")).substring(3, 5).toInt();
    DAT = crm.var("DAT") == "1";
    DATY = crm.var("DATY") == "1";
    HDNF = crm.var("HDNF") == "1";
    MYHD = crm.var("MYHD") == "1";
    RGHD = crm.var("RGHD").toInt();
    RTCm = crm.var("RTCm") == "1";
    _WT = 1000UL * crm.var("_wt").toInt();
  }


  { // Подсветка
    SMIN = crm.var("SMIN").toInt();
    SMAX = crm.var("SMAX").toInt();
    BMIN = crm.var("BMIN").toInt();
    BMAX = crm.var("BMAX").toInt();
    ASENS = crm.var("ASENS").toInt();
    ADEEP = crm.var("ADEEP").toFloat();
    MMBR = crm.var("MMBR").toInt();
    BrightCorrect = true;
  }


  { // Часы, внешний вид
    STYLE = crm.var("STYLE").toInt();
    if (STYLE == 6 || STYLE == 8 || STYLE == 11 || crm.var("MNUM").toInt() > 5) {
      SEC = crm.var("SEC") == "1";
    } else SEC = false;
    PMIG = crm.var("PMIG") == "1";
    HPIC = crm.var("HPIC") == "1";
    DBLH = crm.var("DBLH") == "1";
    if (sMode == 1) mFont(STYLE);
    VIEWT = crm.var("VIEWT").toInt();
    TSPD = crm.var("TSPD").toInt();
    SSPD = 110 - crm.var("SSPD").toInt();
  }


  { // Датчики
    SUDTI = 1000UL * crm.var("SUDTI").toInt();
    SNDNM = crm.var("SNDNM") == "1";
    SCHRT = crm.var("SCHRT") == "1";
    TOFFS = crm.var("TOFFS").toFloat();
    HOFFS = crm.var("HOFFS").toInt();
    POFFS = crm.var("POFFS").toInt();
    uint8_t _ESENS = ESENS;
    ESENS = crm.var("ESENS").toInt();
    if (_ESENS != ESENS) {
      if (_ESENS == 2 && crm.upTimeSec() > 10) scd30.StopMeasurement();
      if (_ESENS == 1) {
        dsCount = 0;
      }
      sensorTicker.once_ms(200, sensorRun);
    }
    uint16_t _ALTDE = ALTDE;
    ALTDE = abs(crm.var("ALTDE").toInt());
    if (firstStart && ESENS == 2 && _ALTDE != ALTDE) {
      scd30.setAltitudeCompensation(ALTDE);
      delay(200);
    }
    float _T2OFFS = T2OFFS;
    T2OFFS = crm.var("T2OFFS").toFloat();
    if (firstStart && ESENS == 2 && _T2OFFS != T2OFFS) {
      scd30.setTemperatureOffset(T2OFFS);
    }
  }


  { // Погода
    uint8_t _WTHR = WTHR;
    WTHR = crm.var("WTHR").toInt();
    if (_WTHR != WTHR && WTHR > 0) {
      weatherTicker.once_ms(1000, weatherTask);
    }
    WINT = 60000UL * crm.var("WINT").toInt();
  }


  { // Народный мониторинг
    NMINT = 60000UL * crm.var("NMINT").toInt();
    nmNotEmpty = crm.var("NMID") != "" && crm.var("NMKEY") != "";
    bool _NMON = NMON;
    NMON = crm.var("NMON") == "1";
    if (_NMON != NMON && NMON) {
      narmonTicker.once_ms(1000, narodmonTask);
    }
  }


  { // UDP
    bool _UDP = UDP;
    UDP = crm.var("UDP") == "1";
    if (_UDP != UDP) {
      if (UDP) udpInit();
      else if (udp) {
        udp->udpListenStop();
        delete udp;
        udp = nullptr;
      }
    }
  }


  { // Будильник
    for (byte i = 0; i < ALL_ALARM_COUNT; i++) {
      AM[i] = crm.var("AM" + String(i)) == "1";
      AMACT[i] = crm.var("AMACT" + String(i)).toInt();
      AMSND[i] = crm.var("AMSND" + String(i)).toInt();
      AMT[i] = timeToSec(crm.var("AMT" + String(i)));
      String d(crm.var("AMD" + String(i)));
      for (byte j = 0; j < 7; j++) {
        AMD[i][j] = d.substring(j, j + 1).toInt();
      }
      if (AMACT[i] == 3) {
        byte _AMPIN = AMPIN[i];
        AMPIN[i] = crm.var("AMPIN" + String(i)).toInt();
        if (_AMPIN != AMPIN[i] && AMPIN[i] > 0) {
          pinMode(AMPIN[i], OUTPUT);
          digitalWrite(AMPIN[i], AMFPS[i]);
        }
        byte _AMFPS = AMFPS[i];
        AMFPS[i] = crm.var("AMFPS" + String(i)).toInt();
        if (_AMFPS != AMFPS[i] && AMPIN[i] > 0) digitalWrite(AMPIN[i], AMFPS[i]);
        AMPTM[i] = crm.var("AMPTM" + String(i)).toDouble();
      }
    }
    if (crm.webConnStatus()) {
      crm.webUpdate("AMWEB", alarmStr());
    }
  }


  { // Аудио
    HPIK = crm.var("HPIK").toInt();
    PIKS = crm.var("PIKS").toInt();
    ATSND = crm.var("ATSND").toInt();
    byte _AVOL = AVOL;
    AVOL = crm.var("AVOL").toInt();
    if (mp3InitStatus[0] && _AVOL != AVOL) {
      dfPlayer.volume(AVOL);
    }
    byte _AEQ = AEQ;
    AEQ = crm.var("AEQ").toInt();
    if (mp3InitStatus[0] && _AEQ != AEQ) {
      dfPlayer.EQ(AEQ);
    }
    APLF = crm.var("APLF").toInt();
    APLS = crm.var("APLS").toInt();


  }

  firstStart = true;
}
