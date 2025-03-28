void mp3Init() {
  pinMode(DFPlayerBusyu_PIN, INPUT);
  dfSerial.begin(9600, SERIAL_8N1, DFPlayerTX_PIN, DFPlayerRX_PIN);
  if (dfPlayer.begin(dfSerial, true, false)) {
    Serial.println(F("[MP3] DFPlayer инициализирован"));
    dfPlayer.setTimeOut(700);
    delay(1000);
    dfPlayer.volume(AVOL);
    dfPlayer.EQ(AEQ);
    dfPlayer.outputDevice(DFPLAYER_DEVICE_SD);
    mp3InitStatus[0] = dfPlayer.readFileCounts();
    if (mp3InitStatus[0] < 1) Serial.println(F("[MP3] Не удалось определить количество файлов на карте памяти"));
    else {
      Serial.print(F("[MP3] Найдено файлов: "));
      Serial.println(mp3InitStatus[0]);
    }
  } else {
    Serial.println(F("[MP3] DFPlayer не найден, проверьте соединение с ним и наличие установленной microSD карты"));
    Serial.println(dfPlayer.readType(), HEX);
    return;
  }


  for (int i = 1; i < 12; i++) {
    int n = dfPlayer.readFileCountsInFolder(i);
    if (n > 0) {
      mp3InitStatus[i] = n;
      //Serial.println(n);
    }
  }
}



void speechTime(int r) {
  if (mp3InitStatus[0] < 1) return;
  static byte c = 0;
  static uint32_t t = 0;
  if (r) {
    c = (r == 4 ? 0 : (r == 2 ? 1 : 0));
  }

  int H = TD.tm_hour;
  int M = TD.tm_min;
  if (ATSND < 1) ATSND = 1;
  if (PIKS < 1) PIKS = 1;

  if (digitalRead(DFPlayerBusyu_PIN) || r) {
    if (millis() - t > 100) {
      if (c == 0) dfPlayer.playFolder(10, PIKS);
      else if (c == 1) dfPlayer.playFolder(ATSND, (!H && !M ? 25 : (!H ? 24 : H)));    // H
      else if (c == 2 && !(H == 0 && M == 0)) dfPlayer.playFolder(ATSND, M + 100);             // M
      c++;
    }
    t = millis();
  }

  if (c < 3 && r != 3) mp3Ticker.once_ms(50, speechTime, 0);
}


void audioBT() {
  crm.btnCallback(F("APRV"), aPreview);
  crm.btnCallback(F("ASTP"), aStop);
  crm.btnCallback(F("APL"), aPlay);
  crm.btnCallback(F("ANPL"), aNext);
}


void aPreview() {
  if (mp3InitStatus[0] < 1) return;
  if ((APLS - 1) > 0) {
    APLS--;
    dfPlayer.previous();
  } else {
    APLS = mp3InitStatus[APLF];
    dfPlayer.playFolder(APLF, APLS);
  }
  crm.var("APLS", String(APLS), 0);
  crm.webUpdate("APLS", String(APLS), 1);
}


void aStop() {
  if (!mp3InitStatus[0]) return;
  dfPlayer.stop();
}


void aPlay() {
  if (mp3InitStatus[0] < 1) return;
  if (APLS > mp3InitStatus[APLF]) {
    APLS = 1;
    crm.var("APLS", String(APLS), 0);
    crm.webUpdate("APLS", String(APLS), 1);
  }
  dfPlayer.playFolder(APLF, APLS);
}


void aNext() {
  if (mp3InitStatus[0] < 1) return;
  if ((APLS + 1) <= mp3InitStatus[APLF]) {
    APLS++;
    dfPlayer.next();
  } else {
    APLS = 1;
    dfPlayer.playFolder(APLF, APLS);
  }
  crm.var("APLS", String(APLS), 0);
  crm.webUpdate("APLS", String(APLS), 1);
}
