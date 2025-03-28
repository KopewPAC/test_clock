void apiRunMessage() {
  if (apiMes.status && millis() - apiMes.timer >= apiMes.pause) {
    apiMes.status = false;
    message(*apiMes.mes);
    if (apiMes.mes) {
      delete apiMes.mes;
      apiMes.mes = nullptr;
    }
  }
}



void api(String p) {
  Serial.print(F("[API] "));
  Serial.println(p);

  DynamicJsonDocument doc(600);
  DeserializationError e = deserializeJson(doc, p);

  if (!e) {

    // Сообщение на экран
    const char* mes = doc[F("mes")];
    if (mes != NULL) {
      Serial.println((byte)mes[0]);
      if (!apiMes.mes) {
        apiMes.mes = new String;
      }
      *apiMes.mes = mes;
      uint16_t sf = doc[F("sf")];
      if (sf) {
        uint32_t sd = doc[F("sd")];
        if (!sd) sd = 100;
        tone32(sf, sd);
      }

      if (strlen(mes) > 0) {
        uint16_t p = doc[F("p")];
        apiMes.pause = 1000UL * p;
        apiMes.timer = millis();
        apiMes.status = true;
      }
      crm.apiResponse(F("status"), F("OK"));
    }

    // Данные с датчиков
    const char* sens = doc[F("sensor")];
    if (sens != NULL) {
      String s(sens);
      s.toLowerCase();
      if (s.indexOf("t2") != -1 ) crm.apiResponse(F("temp2"), String(sensor.T2, 1));
      if (s.indexOf("t1") != -1 || s.indexOf("t") != -1) crm.apiResponse(F("temp1"), String(sensor.T1, 1));
      if (s.indexOf("h") != -1 ) crm.apiResponse(F("hum"), String(sensor.H, 1));
      if (s.indexOf("p") != -1 ) crm.apiResponse(F("pres"), String(sensor.P));
      if (s.indexOf("co2") != -1 ) crm.apiResponse(F("co2"), String(sensor.CO2));
      if (s.indexOf("l") != -1 ) crm.apiResponse(F("light"), String(analogRead(ANALOG_PIN)));
    }

    // отображение данных с датчиков на экране
    const char* view = doc[F("view")];
    if (view != NULL) {//strlen(host);
      String s("");
      {
        int l = strlen(view);
        const String &r(F("  ***  "));
        for (byte i = 0; i < l; i++) {
          if (ESENS && (view[i] == 's' || view[i] == 'S')) {
            if (s != "") s += r;
            s += sensorString(1);
            continue;
          }
          if (NMON && (view[i] == 'n' || view[i] == 'N')) {
            if (s != "") s += r;
            s += narmonString(1);
            continue;
          }
          if (WTHR && (view[i] == 'w' || view[i] == 'W')) {
            if (s != "") s += r;
            s += weatherString(1);
            continue;
          }
          if ((view[i] == 'd' || view[i] == 'D')) {
            if (s != "") s += r;
            s += curData(DATY);
            continue;
          }
          if (MYHD && (view[i] == 'h' || view[i] == 'H')) {
            s += myHoliday((s != "" ? 1 : 0));
            s += holiday(RGHD, TD.tm_year + 1900, TD.tm_mon + 1, TD.tm_mday, TD.tm_wday, (s != "" ? 1 : 0));
            continue;
          }
        }
      }
      if (!apiMes.mes) {
        apiMes.mes = new String;
      }
      *apiMes.mes = s;
      apiMes.pause = 0;
      apiMes.timer = millis();
      apiMes.status = true;
      crm.apiResponse(F("status"), F("OK"));
    }


    // Управление яркостью матрицы
    const char* light = doc[F("light")];
    if (light != NULL) {
      BMIN = BMAX = constrain(atoi(light), 0, 15);
      crm.var("BMIN", String(BMIN), 0);
      crm.var("BMAX", String(BMAX), 0);
      crm.webUpdate("BMIN", String(BMIN));
      crm.webUpdate("BMAX", String(BMAX));
      crm.apiResponse(F("light"), String(BMIN));
      BrightCorrect = true;
    }

    // Управление яркостью матрицы
    const char* saytime = doc[F("saytime")];
    if (saytime != NULL) {
      speechTime(4);
      crm.apiResponse(F("saytime"), F("OK"));
    }


  } else crm.apiResponse(F("deserializeError"), String(e.f_str()));
}
