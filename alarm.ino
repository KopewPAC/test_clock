void alarmInit() {
  for (byte i = 0; i < ALL_ALARM_COUNT; i++) {
    if (AM[i] && AMACT[i] == 3 && AMPIN[i] > 0) {
      pinMode(AMPIN[i], OUTPUT);
      digitalWrite(AMPIN[i], AMFPS[i]);
    }
  }
}


void alarmRun(uint32_t sec) {
  for (int b = 0; b < ALL_ALARM_COUNT; b++) {
    if (AM[b] && AMD[b][TD.tm_wday - 1] && sec == AMT[b]) {
      String n(F("Будильник "));
      n += b + 1;
      n += F(" активирован");
      crm.webNotif("yellow", n, 5, 1);
      if (AMACT[b] == 0) {
        if (!alarmBeepActive) alarmTick[b].once_ms(500, alarmBeep, b);
      } else if (AMACT[b] == 1) {
        if (mp3InitStatus[0] > 0) {
          alarmMp3PlayStatus = 1;
          dfPlayer.playFolder(11, AMSND[b]);
          alarmTick[b].once_ms(1000, alarmMp3Play, b);
        }
      } else if (AMACT[b] == 2) {
        alarmTick[b].once_ms(500, alarmGet, b);
      } else if (AMACT[b] == 3 && AMPIN[b] > 0) {
        alarmTick[b].once_ms(500, alarmPinChange, b);
      }
    }
  }
}



void alarmMp3Play(int b) {
  if (mp3InitStatus[0] < 1) {
    alarmMp3PlayStatus = 0;
    return;
  }
  int a = dfPlayer.available();
  int r = dfPlayer.readType();
  if (a && r == 5) {
    alarmMp3PlayStatus = 0;
  }
  if (alarmMp3PlayStatus) alarmTick[b].once_ms(100, alarmMp3Play, b);
}


void alarmPinChange(int b) {
  digitalWrite(AMPIN[b], !digitalRead(AMPIN[b]));
  alarmTick[b].once_ms(AMPTM[b], alarmPinTimeEnd, b);
}
void alarmPinTimeEnd(int b) {
  digitalWrite(AMPIN[b], AMFPS[b]);
}


void alarmBeep(int b) {
  uint16_t F = 3000;
  uint16_t pause = 2000;
  static byte m = 0;
  static uint32_t lastCall = 0, timer = 0;
  if (millis() - lastCall > 3000) {
    alarmBeepActive = 1;
    timer = millis();
    m = 0;
  }
  lastCall = millis();
  switch (m) {
    case 0: {
        toneAction(F);
        pause = 200;
        m++;
      } break;
    case 1: {
        toneAction(0);
        pause = 100;
        m++;
      } break;
    case 2: {
        toneAction(F);
        pause = 70;
        m++;
      } break;
    case 3: {
        toneAction(0);
        pause = 100;
        m++;
      } break;
    case 4: {
        toneAction(F);
        pause = 70;
        m++;
      } break;
    case 5: {
        toneAction(0);
        pause = 100;
        m++;
      } break;
    case 6: {
        toneAction(F);
        pause = 70;
        m++;
      } break;
    case 7: {
        toneAction(0);
        if (millis() - timer > 60000) alarmBeepActive = 0;
        else m = 0;
      } break;
    default:
      alarmBeepActive = 0;
      break;
  }
  if (alarmBeepActive) alarmTick[b].once_ms(pause, alarmBeep, b);
}
void toneAction(uint16_t f) {
  if (f) {
    ledcSetup(0, f, 8);
    ledcAttachPin(BUSSER_PIN, 0);
    ledcWrite(0, (1 << 7));
  } else {
    ledcWrite(0, 0);
    ledcDetachPin(BUSSER_PIN);
    pinMode(BUSSER_PIN, INPUT);
  }
}



void alarmGet(int b) {
  if (WiFi.status() == WL_CONNECTED) {
    if (mutex) {
      mutex = 0;
      HTTPClient h;
      String url(crm.var("AMGET" + String(b)));
      url.replace(" ", "%20");
      if (!url.startsWith("http://") && !url.startsWith("https://")) {
        url = "http://" + url;
      }
      if (url.length() > 12) {
        h.begin(url);
        h.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
        int hCode = h.GET();
        if (hCode == -1) {
          WiFi.reconnect();
        }
        String l(F("[ALARM "));
        l += b + 1;
        l += F("] Код ответа: ");
        l += hCode;
        l += F("\n[ALARM ");
        l += b + 1;
        l += F("] Ответ: ");
        l += h.getString();
        Serial.println(l);
        h.end();
      } else {
        String l(F("[ALARM "));
        l += b + 1;
        l += F("] Некорректный URL: ");
        Serial.print(l);
        Serial.println(url);
      }
      mutex = 1;
    } else alarmTick[b].once_ms(1000, alarmGet, b);
  }
}



String alarmStr() {
  String d[] = {"Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс"};
  String s("");
  for (byte b = 0; b < ALL_ALARM_COUNT; b++) {
    if (AM[b]) {
      if (s != "") s += "<br>";
      s += b + 1;
      s += ":  ";
      s += crm.var("AMT" + String(b)).substring(0, 5);
      s += "  •  ";
      String dn("");
      for (byte j = 0; j < 7; j++) {
        if (AMD[b][j]) {
          if (dn != "") dn += ", ";
          dn += d[j];
        }
      }
      s += (dn != "" ? dn : "<b style='color:red'>!!!</b>");
    }
  }
  return s;
}



void tone32(uint16_t f, uint32_t d) {
  ledcSetup(1, f, 8);
  ledcAttachPin(BUSSER_PIN, 1);
  ledcWrite(1, (1 << 7));
  delay(d);
  ledcWrite(1, 0);
  ledcDetachPin(BUSSER_PIN);
  pinMode(BUSSER_PIN, INPUT);
}
