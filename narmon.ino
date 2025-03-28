
String narmonString(bool degreeConvert) {
  String tmp(crm.var(F("NMTMP")));
  int adr = tmp.indexOf("S");
  while (adr != -1) {
    int c = 1;
    if ((byte)tmp[adr + 1] >= 48 && (byte)tmp[adr + 1] <= 57) {
      while (1) {
        c++;
        byte b = (byte)tmp[adr + c];
        if (b < 48 || b > 57) break;
      }
      uint32_t id = tmp.substring(adr + 1, adr + c).toInt();
      for (uint8_t i = 0; i < 10; i++) {
        if (narodmonVol[i][0] == id) {
          tmp.replace(tmp.substring(adr, adr + c), String(narodmonVol[i][1], 1));
          break;
        }
      }
    }
    adr = tmp.indexOf("S", adr + c);
  }
  if (degreeConvert) tmp.replace(F("°"), F("`"));
  return tmp;
}


void narodmonTask() {
  nmStatus = true;
}


void narodmonRun() {
  if (nmStatus) {
    nmStatus = false;
    uint32_t pause = 5000;   // 5 sec

    if (nmNotEmpty && mutex && WiFi.status() == WL_CONNECTED) {
      mutex = 0;
      if (dbg) Serial.println(F("\n[HTTP] Народный мониторинг - start"));


      const String &NMID(crm.var(F("NMID")));
      const String &APIKEY(crm.var(F("NMKEY")));
      String u(F("http://narodmon.com/api/"));
      u += (NMID.startsWith(F("D")) ? F("sensorsOnDevice?devices=") : F("sensorsValues?sensors="));
      //u += F("sensorsOnDevice?devices=");
      u += NMID.substring(1);
      u += F("&uuid=");
      u += md5(APIKEY);
      u += F("&api_key=");
      u += APIKEY;
      u += F("&lang=en");

      HTTPClient h;
      h.begin(u);
      int hCode = h.GET();
      if (dbg) Serial.printf("[HTTP] Статус код: %d %s\n", hCode, h.errorToString(hCode).c_str());

      if (hCode == HTTP_CODE_OK) {
        String nm_json(h.getString());
        if (dbg) Serial.println(nm_json);


        if (nm_json.startsWith("{") && nm_json.endsWith("}]}")) {

          int u_index = nm_json.indexOf(F("\\u"));
          while (u_index > 0) {
            nm_json.remove(u_index, 6);
            u_index = nm_json.indexOf(F("\\u"));
          }

          DynamicJsonDocument doc(3100);
          DeserializationError e = deserializeJson(doc, nm_json);

          if (!e) {
            if (NMID.startsWith(F("D"))) {
              uint8_t arrCount = 0;
              for (JsonObject dev_sen : doc[F("devices")][0][F("sensors")].as<JsonArray>()) {
                narodmonVol[arrCount][0] = dev_sen[F("id")];
                narodmonVol[arrCount][1] = dev_sen[F("value")];
                arrCount++;
              }
            } else {
              narodmonVol[0][0] = doc[F("sensors")][0][F("id")];
              narodmonVol[0][1] = doc[F("sensors")][0][F("value")];
            }

            Serial.println(narmonString(0));
            if (crm.webConnStatus()) {
              crm.webUpdate("NMVW", narmonString(0));
            }
            pause = NMINT;
          } else {
            pause = 120000UL;  // 2 min
            Serial.print(F("[NM] Ошибка DeserializeJson: "));
            Serial.println(e.f_str());
          }
        } else {
          pause = 300000UL; // 5 min
          Serial.println(F("[NM] Данные в ответе некорректны."));
        }

      } else {
        pause = 120000UL;  // 2 min
      }

      h.end();
      if (dbg) Serial.println(F("[HTTP] Народный мониторинг - end"));
      mutex = 1;
    }

    if (NMON) narmonTicker.once_ms(pause, narodmonTask);
  }
}


String md5(const String &str) {
  MD5Builder md5Hash;
  md5Hash.begin();
  md5Hash.add(str);
  md5Hash.calculate();
  return md5Hash.toString();
}
