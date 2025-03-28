void weatherTask() {
  weatherStatus = true;
}


void weatherRun() {
  if (weatherStatus) {
    weatherStatus = false;
    uint32_t pause = 5000UL;
    if (mutex && WiFi.status() == WL_CONNECTED) {
      mutex = 0;
      if (dbg) Serial.println(F("\n[HTTP] Погода - start"));
      HTTPClient h;



      switch (WTHR) {
        // AccuWeather +     
        case 1: {
            h.begin("http://dataservice.accuweather.com/currentconditions/v1/" +
                    crm.var("WID") + "?apikey=" + crm.var("WKEY") + "&language=ru-ru&details=true");
          } break;

        // WeatherStack +       
        case 5: {
            h.begin("https://weatherstack.com/ws_api.php?ip=" + crm.var("WID"));
          } break;
        // OpenWeatherMap +   
        case 6: {
            h.begin("https://openweathermap.org/data/2.5/weather?id=" + crm.var("WID") + "&appid=439d4b804bc8187953eb36d2a8c26a02");
          } break;

        default: break;
      }

      h.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);

      int hCode = h.GET();
      if (dbg) Serial.printf("[HTTP] Статус код: %d %s\n", hCode, h.errorToString(hCode).c_str());
      if (hCode == 200) {
        String pl(h.getString());

        if (dbg) {
          Serial.println(F("-["));
          Serial.println(pl);
          Serial.println(F("]-"));
        }

        switch (WTHR) {
          case 1: parserAW(pl); break;     // AccuWeather
          case 5: parserWS(pl); break;     // WeatherStack
          case 6: parserOWM(pl); break;    // OpenWeatherMap
          default: break;
        }
        Serial.println(weatherString(0));

        pause = (WTHR == 1 && WINT < 1800000UL ? 1800000UL : WINT);
        //if (dbg) Serial.println(F("\n[HTTP] конец файла или соединение закрыто."));
      } else pause = 60000UL;
      h.end();
      if (dbg) Serial.println(F("[HTTP] Погода - end"));
      if (crm.webConnStatus()) crm.webUpdate("WTRV", weatherString(0));
      mutex = 1;
    }
    if (WTHR > 0) weatherTicker.once_ms(pause, weatherTask);
  }
}


String weatherString(bool onScreen) {
  String s = crm.var("WTMP");
  if (s == "") return F("Шаблон погоды не задан");
  s.replace("%TR1", String(weather.TR, 1));
  s.replace("%TR", String(weather.TR, 0));
  s.replace("%T1", String(weather.T, 1));
  s.replace("%T", String(weather.T, 0));
  s.replace("%D", UtoL(weather.D));
  s.replace("%WD", weather.WD);
  s.replace("%WG1", String(weather.WG, 1));
  s.replace("%WG", String(weather.WG, 0));
  s.replace("%W1", String(weather.W, 1));
  s.replace("%W", String(weather.W, 0));
  s.replace("%H", String(weather.H));
  s.replace("%P", String(weather.P));
  if (onScreen) {
    s.replace(F("°"), F("`"));
    s.replace(F(" "), F(" "));
  }
  s.replace("\"", "");
  return s;
}


void parserAW(const String &s) {
  DynamicJsonDocument doc(7000);
  DeserializationError e = deserializeJson(doc, s.substring(s.indexOf("[{"), s.lastIndexOf("}]") + 2));
  if (e) {
    if (dbg) {
      Serial.print(F("[AW] Ошибка DeserializeJson: "));
      Serial.println(e.c_str());
    }
  } else {
    const char* WeatherText = doc[0]["WeatherText"];
    weather.D = WeatherText;
    weather.T = doc[0]["Temperature"]["Metric"]["Value"];
    weather.TR = doc[0]["RealFeelTemperature"]["Metric"]["Value"];
    weather.H = int(doc[0]["RelativeHumidity"]);
    weather.W = float(doc[0]["Wind"]["Speed"]["Metric"]["Value"]) / 3.6;
    weather.WG = float(doc[0]["WindGust"]["Speed"]["Metric"]["Value"]) / 3.6;
    const char* direct = doc[0]["Wind"]["Direction"]["Localized"];
    weather.WD = direct;
    weather.P = int(doc[0]["Pressure"]["Metric"]["Value"]) * 0.750064;
  }
}


void parserWS(const String &s) {
  DynamicJsonDocument doc(8500);
  DeserializationError e = deserializeJson(doc, s.substring(s.indexOf("{"), s.lastIndexOf("}}}") + 3));
  if (e) {
    if (dbg) {
      Serial.print(F("[WS] Ошибка DeserializeJson: "));
      Serial.println(e.c_str());
    }
  } else {
    JsonObject current = doc["current"];
    const char* descr = current["weather_descriptions"][0];
    weather.D = toRus(descr);
    weather.T = current["temperature"];
    weather.TR = current["feelslike"];
    weather.W = float(current["wind_speed"]) / 3.6;
    weather.WG = 0;
    const char* wind_dir = current["wind_dir"];
    weather.WD = toRus(wind_dir);
    weather.H = int(current["humidity"]);
    weather.P = int(current["pressure"]) * 0.750064;
  }
}


void parserOWM(const String &s) {
  DynamicJsonDocument doc(1500);
  DeserializationError e = deserializeJson(doc, s.substring(s.indexOf("{"), s.lastIndexOf("}") + 1));
  if (e) {
    if (dbg) {
      Serial.print(F("[OWM] Ошибка DeserializeJson: "));
      Serial.println(e.c_str());
    }
  } else {
    const char* descr = doc["weather"][0]["description"];
    weather.D = toRus(descr);
    JsonObject main = doc["main"];
    weather.T = main["temp"];
    weather.TR = main["feels_like"];
    weather.H = int(main["humidity"]);
    weather.P = int(main["pressure"]) * 0.750064;
    JsonObject wind = doc["wind"];
    weather.W = float(wind["speed"]);
    weather.WG = float(wind["gust"]);
    weather.WD = wDirect(wind["deg"]);
  }
}


String toRus(const String &s) {
  if (s == "sunny") return F("солнечно");
  else if (s == "Light snow shower") return F("небольшой снегопад");
  else if (s == "Clear") return F("ясно");
  else if (s == "Partly cloudy") return F("переменная облачность");
  else if (s == "Cloudy") return F("облачно");
  else if (s == "Mist") return F("дымка");
  else if (s == "Light snow") return F("небольшой снег");
  else if (s == "Snow") return F("снег");
  else if (s == "Moderate snow") return F("умеренный снег");
  else if (s == "Heavy snow") return F("снегопад");
  else if (s == "Overcast") return F("пасмурная погода");
  else if (s == "Freezing fog") return F("ледяной туман");
  else if (s == "few clouds") return F("небольшая облачность");
  else if (s == "Fog") return F("туман");
  else if (s == "NNE") return F("ССВ");
  else if (s == "ENE") return F("ВСВ");
  else if (s == "ESE") return F("ВЮВ");
  else if (s == "SSE") return F("ЮЮВ");
  else if (s == "SSW") return F("ЮЮЗ");
  else if (s == "WSW") return F("ЗЮЗ");
  else if (s == "WNW") return F("ЗСЗ");
  else if (s == "NNW") return F("ССЗ");
  else if (s == "NE") return F("СВ");
  else if (s == "SE") return F("ЮВ");
  else if (s == "SW") return F("ЮЗ");
  else if (s == "NW") return F("СЗ");
  else if (s == "N") return F("С");
  else if (s == "E") return F("В");
  else if (s == "S") return F("Ю");
  else if (s == "W") return F("З");
  else if (s == "C") return F("Ш");
  else if (s == "ШТЛ") return "";
  else return s;
}


String wDirect(int d) {
  if (337 < d || d <= 22) return F("С");          //n
  else if (22 < d && d <= 67) return F("СВ");     //ne
  else if (67 < d && d <= 112) return F("В");     //e
  else if (112 < d && d <= 157) return F("ЮВ");   //se
  else if (157 < d && d <= 202) return F("Ю");    //s
  else if (202 < d && d <= 247) return F("ЮЗ");   //sw
  else if (247 < d && d <= 292) return F("З");    //w
  else if (292 < d && d <= 337) return F("СЗ");   //nw
  return "";
}


String UtoL(const String &s) {
  String _s(s);
  _s.replace("Ё", "ё");
  _s.replace("Й", "й");
  _s.replace("Ц", "ц");
  _s.replace("У", "у");
  _s.replace("К", "к");
  _s.replace("Е", "е");
  _s.replace("Н", "н");
  _s.replace("Г", "г");
  _s.replace("Ш", "ш");
  _s.replace("Щ", "щ");
  _s.replace("З", "з");
  _s.replace("Х", "х");
  _s.replace("Ъ", "ъ");
  _s.replace("Ф", "ф");
  _s.replace("Ы", "ы");
  _s.replace("В", "в");
  _s.replace("А", "а");
  _s.replace("П", "п");
  _s.replace("Р", "р");
  _s.replace("О", "о");
  _s.replace("Л", "л");
  _s.replace("Д", "д");
  _s.replace("Ж", "ж");
  _s.replace("Э", "э");
  _s.replace("Я", "я");
  _s.replace("Ч", "ч");
  _s.replace("С", "с");
  _s.replace("М", "м");
  _s.replace("И", "и");
  _s.replace("Т", "т");
  _s.replace("Ь", "ь");
  _s.replace("Б", "б");
  _s.replace("Ю", "ю");
  return _s;
}
