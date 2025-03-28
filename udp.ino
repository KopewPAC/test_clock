
void udpInit() {
  if (!udp) udp = new AsyncUDP;
  if (udp->listen(1234)) {
    udp->onPacket([](AsyncUDPPacket packet) {
      String d((char*)packet.data());
      Serial.print(F("[UDP] "));
      Serial.print(packet.remoteIP());
      Serial.print(F(": "));
      Serial.println(d);

      DynamicJsonDocument j(2000);
      DeserializationError e = deserializeJson(j, d);
      if (!e) {

        // Сообщение на экран
        if (j["mes"]) {
          const char* m = j["mes"];
          if (!apiMes.mes) {
            apiMes.mes = new String;
          }
          *apiMes.mes = m;
          uint16_t sf = j[F("sf")];
          if (sf) {
            uint32_t sd = j[F("sd")];
            if (!sd) sd = 100;
            tone32(sf, sd);
          }
          uint16_t p = j[F("p")];
          apiMes.pause = 1000UL * p;
          apiMes.timer = millis();
          apiMes.status = true;
        }

        // Датчики
        {
          String sn("sensor");
          if (j[sn]) {
            if (j[sn]["T2"]) {
              sensor.T2 = j[sn]["T2"];
            }
            if (j[sn]["T1"]) {
              sensor.T1 = j[sn]["T1"];
            } else if (j[sn]["T"]) {
              sensor.T1 = j[sn]["T"];
            }
            if (j[sn]["H"]) {
              sensor.H = j[sn]["H"];
            }
            if (j[sn]["P"]) {
              sensor.P = j[sn]["P"];
            }
            if (j[sn]["CO2"]) {
              sensor.CO2 = j[sn]["CO2"];
            }
          }
        }


      } else {
        Serial.print(F("[UDP] Ошибка DeserializeJson: "));
        Serial.println(e.f_str());
      }
    });
  }
}
