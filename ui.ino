void interface() {
  {
    crm.page("&#xe802; Монитор");
    crm.output({OUTPUT_HR, "1px", "20px 10% -31px"});
    crm.output({OUTPUT_TABL, "TM", "&#xe80b;", curTime(1, 0, 0), "fff"});
    crm.output({OUTPUT_TABL, "DT", "&#xe81e;", curData(1), "fff"});
    crm.output({OUTPUT_HR, "1px", "-3px 10% -15px"});
    for (byte i = 0; i < ALL_ALARM_COUNT; i++) {
      if (AM[i]) {
        crm.output({OUTPUT_TABL, "AMWEB", "&#xf0f3;", alarmStr(), "cfa"});
        break;
      }
    }
    {
      String b("");
      if (MYHD) b += myHoliday(0);
      if (RGHD) {
        if (b != "") b += "  ";
        b += holiday(RGHD, TD.tm_year + 1900, TD.tm_mon + 1, TD.tm_mday, TD.tm_wday, 0);
      }
      if (b == "") b = (TD.tm_wday < 6 ? "Обычный день" : "Выходной");
      crm.output({OUTPUT_TABL, "HDV", "&#xf0eb;", b, "cfa"});
    }
    if (ESENS) {
      crm.output({OUTPUT_HR, "0", "-20px"});
      crm.output({OUTPUT_TABL, "SENSV", "&#xf0e4;", sensorString(0), "9ff"});
    }
    if (WTHR) {
      crm.output({OUTPUT_HR, "0", "-20px"});
      crm.output({OUTPUT_TABL, "WTRV", "&#xf185;", weatherString(0), "ffb"});
    }
    if (NMON) {
      crm.output({OUTPUT_HR, "0", "-20px"});
      crm.output({OUTPUT_TABL, "NMVW", "&#xf201;", narmonString(0), "fcb"});
    }
    if (ESENS && SCHRT) {
      String t = sensHistoryRead(0);
      if (ESENS) {
        {
          String l("Температура");
          l += (ESENS == 1 ? " 1" : "");
          l += ", °C";
          crm.chart({CHART_L, "HTMP1", l, t,  sensHistoryRead(1), "#00dd00", "250"});
        }
        if (dsCount > 1) {
          crm.chart({CHART_L, "HTMP2", "Температура 2, °C", t,  sensHistoryRead(2), "#00dd00", "250"});
        }
      }
      if (ESENS > 1) {
        crm.chart({CHART_L, "HHUM", "Влажность, %", t,  sensHistoryRead(3), "#00dd00", "250"});
      }
      if (ESENS == 2 || ESENS == 100) {
        crm.chart({CHART_L, "HCO2", "CO2, ppm", t,  sensHistoryRead(5), "#00dd00", "250"});
      }
      if (ESENS == 3 || ESENS == 100) {
        crm.chart({CHART_L, "PRES", "Давление, мм рт.ст.", t,  sensHistoryRead(4), "#00dd00", "250"});
      }
    }
  }


  {
    crm.page("&#xe80b; Часы");
    {
      crm.selOpt({"Микро (§)", "11"});
      crm.selOpt({"Худыш (§)", "8"});
      crm.selOpt({"Худыш небольшой (§)", "6"});
      crm.selOpt({"Обычный", "1"});
      crm.selOpt({"Необычный", "10"});
      crm.selOpt({"ПолуЖирный", "4"});
      crm.selOpt({"ПолуЖирный небольшой", "7"});
      crm.selOpt({"ПолуЖирный с засечками", "5"});
      crm.selOpt({"Жирный", "2"});
      crm.selOpt({"Жирный+", "9"});
      crm.selOpt({"Жирный прямой", "3"});
      crm.selOpt({"Корявый", "12"});
      crm.select({"STYLE", "Стиль часов", "2"});
    }
    crm.output({OUTPUT_HR, "0", "40px"});
    crm.input({INPUT_CHECKBOX, "SEC", "Секунды (§)", "0"});
    crm.input({INPUT_CHECKBOX, "DBLH", "Ведущий ноль (06:00)", "0"});
    crm.input({INPUT_CHECKBOX, "PMIG", "Мигать двоеточками", "1"});
    crm.output({OUTPUT_HR, "0", "40px"});
    crm.input({INPUT_CHECKBOX, "NM", "Ночной режим (только время)", "0", "1"});
    if (NM) {
      crm.group(G_START);
      crm.input({INPUT_TIME, "NMS", "Начало ночного режима"});
      crm.input({INPUT_TIME, "NME", "Окончание ночного режима"});
      crm.group(G_END);
      crm.output({OUTPUT_HR, "0", "50px"});
    }
    crm.output({OUTPUT_HR, "0", "50px"});
    crm.range({"VIEWT", "Длительность показа времени", 25, 5, 360, 1});
    crm.range({"TSPD", "Скорость анимации времени", 10, 1, 20, 1});
    crm.range({"SSPD", "Скорость бегущей строки", 85, 1, 100, 1});
  }


  {
    crm.page("&#xf0f3; Будильник");
    for (byte i = 0; i < ALL_ALARM_COUNT; i++) {
      if (i && AM[i - 1]) crm.output({OUTPUT_HR, "1px", "55px 30%"});
      crm.input({INPUT_CHECKBOX, "AM" + String(i), "Будильник " + String(i + 1), "0", "1"});
      if (AM[i]) {
        crm.input({INPUT_TIME, "AMT" + String(i), "Время", "00:00"});
        crm.output({OUTPUT_HR, "0", "-45px"});
        crm.input({INPUT_WEEK, "AMD" + String(i), "", "0000000"});
        {
          crm.selOpt({"Сигнал (Зуммер)", "0"});
          if (mp3InitStatus[0] > 0) crm.selOpt({"Мелодия (папка 11)", "1"});
          crm.selOpt({"GET запрос", "2"});
          crm.selOpt({"Изменение состояние GPIO", "3"});
          crm.select({"AMACT" + String(i), "Действие", "0", "1"});
        }
        if (AMACT[i] == 1) {
          if (mp3InitStatus[0] > 0) {
            for (int i = 1; i <= mp3InitStatus[11]; i++) {
              String is(i);
              crm.selOpt({is, is});
            }
          }
          crm.select({"AMSND" + String(i), "Мелодия будильника", "1"});
        }
        else if (AMACT[i] == 2) crm.input({INPUT_TEXT, "AMGET" + String(i), "URL", ""});
        else if (AMACT[i] == 3) {
          crm.input({INPUT_NUMBER, "AMPIN" + String(i), "Пин GPIO", "27"});
          {
            crm.selOpt({"LOW", "0"});
            crm.selOpt({"HIGH", "1"});
            crm.select({"AMFPS" + String(i), "Начальное состояние", "0"});
          }
          crm.input({INPUT_NUMBER, "AMPTM" + String(i), "Переключать на, мсек", "1000"});
        }
      }
    }
  }


  {
    crm.page("&#xe81e; Дата и праздники");
    crm.input({INPUT_CHECKBOX, "DAT", "Строка с датой (§)", "1"});
    crm.input({INPUT_CHECKBOX, "DATY", "Отображать год", "1"});
    crm.output({OUTPUT_HR, "1px", "30px 10%"});
    {
      crm.selOpt({"Не показывать", "0"});
      crm.selOpt({"Россия", "1"});
      crm.selOpt({"Беларусь", "3"});
      crm.selOpt({"Казахстан", "4"});
      crm.selOpt({"Узбекистан", "5"});
      crm.selOpt({"Украина", "2"});
      crm.select({"RGHD", "Национальные праздники", "0"});
    }
    crm.output({OUTPUT_HR, "1px", "30px 10%"});
    crm.input({INPUT_CHECKBOX, "MYHD", "Свои праздники (§)", "0", "1"});
    if (MYHD) {
      crm.input({INPUT_CHECKBOX, "HDNF", "Напоминание за день", "1"});
      crm.output({OUTPUT_HR, "0", "50px"});
      for (int i = 1; i <= HOLIDAY_COUNT; i++) {
        crm.input({INPUT_TEXT, "HD" + String(i), "Свой " + String(i), ""});
      }
    }
  }


  {
    crm.page("&#xf108; Дисплей");
    crm.output({OUTPUT_HR, "1px", "20px 10% -31px"});
    crm.output({OUTPUT_TABL, "BRNES", "Яркость матрицы", "0", "fff"});
    crm.output({OUTPUT_TABL, "ANALOG", "Данные с датчика", "0", "fff"});
    crm.output({OUTPUT_HR, "1px", "-3px 10% 0"});
    {
      crm.selOpt({"По ночному режиму", "0"});
      crm.selOpt({"По датчику", "1"});
      crm.selOpt({"Вручную", "2"});
      crm.select({"MMBR", "Управление яркостью", "1"});
    }
    crm.group(G_START);
    crm.range({"BMAX", "Яркость &#xe826;", 15, 1, 15, 1});
    crm.range({"SMAX", "Порог датчика &#xe826;", 4095, 1, 4095, 1});
    crm.group(G_END);
    crm.group(G_START);
    crm.range({"BMIN", "Яркость &#xe823;", 0, 0, 14, 1});
    crm.range({"SMIN", "Порог датчика &#xe823;", 0, 0, 4094, 1});
    crm.group(G_END);
    crm.output({OUTPUT_HR, "1px", "40px 20% 10px"});
    crm.group(G_START);
    crm.range({"ASENS", "Фильтрация помех", 100, 0, 200, 1});
    crm.range({"ADEEP", "Коэффициент сглаживания", 0.25, 0.01, 1, 0.01});
    crm.group(G_END);
  }


  {
    crm.page("&#xe803; Аудио");
    {
      crm.selOpt({"Не использовать", "0"});
      crm.selOpt({"ПИК (Зуммер)", "1"});
      if (mp3InitStatus[0] > 0) {
        crm.selOpt({"Время", "2"});
        crm.selOpt({"Мелодия", "3"});
        crm.selOpt({"Мелодия + Время", "4"});
      }
      crm.select({"HPIK", "Ежечасный сигнал", "0", "1"});
    }
    if (mp3InitStatus[0] > 0) {
      if (HPIK > 2) {
        for (int i = 1; i <= mp3InitStatus[10]; i++) {
          String is(i);
          crm.selOpt({is, is});
        }
        crm.select({"PIKS", "Мелодия ежечасного сигнала (папка 10)", "1"});
        for (int i = 1; i < 10; i++) {
          if (mp3InitStatus[i]) {
            String is(i);
            crm.selOpt({is, is});
          }
        }
        crm.select({"ATSND", "Папка с голосом (01-09)", "1"});
      }
      crm.output({OUTPUT_HR, "1px", "40px 20% 10px"});
      crm.range({"AVOL", "Громкость mp3", 15, 0, 30, 1});
      crm.selOpt({"Нормальный", "0"});
      crm.selOpt({"Поп", "1"});
      crm.selOpt({"Рок", "2"});
      crm.selOpt({"Джаз", "3"});
      crm.selOpt({"Классика", "4"});
      crm.selOpt({"Басс", "5"});
      crm.select({"AEQ", "Эквалайзер", "0"});
      {
        crm.output({OUTPUT_HR, "1px", "40px 20% 10px"});
        crm.output({OUTPUT_LABEL, "", "&#xe803; Плеер мелодий", "center", "#fff", "20"});
        for (int i = 10; i < 12; i++) {
          if (mp3InitStatus[i]) {
            String is(i);
            if (i == 10) is += " - мелодия";
            else if (i == 11) is += " - будильник";
            crm.selOpt({is, String(i)});
          }
        }
        crm.select({"APLF", "Папки на карте", "11", "1"});
        for (int i = 1; i <= mp3InitStatus[crm.var("APLF").toInt()]; i++) {
          String is(i);
          crm.selOpt({is, is});
        }
        crm.select({"APLS", "Мелодия", "1"});
        crm.input({INPUT_BUTTON, "APRV", "&#xe816;", "5px 20px 5px 20px", "r", "35"});
        crm.input({INPUT_BUTTON, "ASTP", "&#xe812;", "5px 20px 5px 20px", "r", "35"});
        crm.input({INPUT_BUTTON, "APL", "&#xe811;", "5px 20px 5px 20px", "r", "35"});
        crm.input({INPUT_BUTTON, "ANPL", "&#xe815;", "5px 20px 5px 20px", "r", "35"});
      }
    }
  }


  {
    crm.page("&#xf0e4; Датчики");
    {
      crm.selOpt({"Не использовать", "0"});
      crm.selOpt({"AHT10/AHT20", "4"});
      crm.selOpt({"BME280/BMP280", "3"});
      crm.selOpt({"DS18B20 x2", "1"});
      crm.selOpt({"SCD30", "2"});
      crm.selOpt({"Сетевой UDP", "100"});
      crm.select({"ESENS", "Тип датчика", "0", "1"});
    }
    if (ESENS == 100 && !UDP) crm.webNotif("Red", F("Включите <b>UDP протокол</b> в разделе <b>Система</b>"), 10, 1);
    crm.input({INPUT_CHECKBOX, "SNDNM", "Отправлять на Народный мониторинг", "0", "1"});
    if (SNDNM) {
      crm.output({OUTPUT_HR, "0", "-20px"});
      {
        String t("ID датчика: ");
        t += WiFi.macAddress();
        crm.output({OUTPUT_TEXT, "", "", t, "#0f0"});
      }
      crm.output({OUTPUT_HR, "0", "30px"});
    }
    crm.input({INPUT_CHECKBOX, "SCHRT", "График (за 24 часа, 3 раза/час)", "0", "1"});
    crm.range({"SUDTI", "Частота опроса", 5, 5, 60, 1, " сек"});
    if (ESENS == 2) {
      crm.input({INPUT_NUMBER, "ALTDE", "Высота датчика над уровнем моря, м", "0"});
      crm.group(G_START);
      crm.range({"T2OFFS", "Температурная компенсация", 0, 0, 20, 0.1, " °C"});
    } else if (ESENS > 0) {
      crm.group(G_START);
      crm.range({"TOFFS", "> T <", 0, -15, 15, 0.1, " °C"});
    }
    if (ESENS > 1) {
      crm.range({"HOFFS", "> H <", 0, -20, 20, 1, " %"});
    }
    if (ESENS == 3) {
      crm.range({"POFFS", "> P <", 0, -100, 100, 1, " мм рт.ст."});
    }
    crm.group(G_END);
    if (ESENS == 2) crm.input({INPUT_BUTTON, "FRF", "Перекалибровка"});
    crm.input({INPUT_TEXT, "SENTMP", "Шаблон вывода", "Пример шаблона датчиков CO2 %CO2 ppm,  Темп1 %T11 °C,  Темп2 %T20 °C,  Влажн %H %,  Давл %P мм рт.ст."});
  }


  {
    crm.page("&#xf185; Погода");
    {
      crm.selOpt({"Не использовать", "0"});
      crm.selOpt({"AccuWeather", "1"});
      crm.selOpt({"OpenWeatherMap", "6"});
      crm.selOpt({"WeatherStack", "5"});
      crm.select({"WTHR", "Погодный сервис", "0", "1"});
    }
    crm.range({"WINT", "Интервал обновления", 20, 10, 60, 1, " мин"});
    if (WTHR == 1) crm.input({INPUT_TEXT, "WKEY", "API KEY", ""});
    crm.input({INPUT_TEXT, "WID", "ID местности", ""});
    crm.input({INPUT_TEXT, "WTMP", "Шаблон вывода", "Пример шаблона погоды %D, %T1 (%TR1) °C, ветер %WD %W1 (%WG1) м/с, влажность %H %, давление %P мм рт.ст."});
  }


  {
    crm.page("&#xf201; Народный мониторинг");
    crm.input({INPUT_CHECKBOX, "NMON", "Народный мониторинг", "0", "1"});
    crm.input({INPUT_TEXT, "NMID", "ID устройства (D)", ""});
    crm.input({INPUT_TEXT, "NMKEY", "API KEY", ""});
    crm.range({"NMINT", "Интервал обновления", 10, 5, 60, 1, " мин"});
    crm.input({INPUT_TEXT, "NMTMP", "Шаблон вывода", "Пример шаблона НарМон S2668 (S2653) ппгв"});
  }


  {
    crm.page("&#xf1de; Система");
    crm.group(G_START);
    crm.input({INPUT_TEXT, "NTP", "NTP сервер", "1.pool.ntp.org"});
    crm.input({INPUT_NUMBER, "TZ", "Временная зона, мин", "180"});
    crm.group(G_END);
    crm.input({INPUT_CHECKBOX, "RTCm", "Внешний RTC модуль (&#xe810;)", "0"});
    crm.input({INPUT_DATETIME, "MDT", "Дата и Время"});
    crm.input({INPUT_BUTTON, "SETDT", "Задать"});
    crm.output({OUTPUT_HR, "1px", "30px 20% 45px"});
    crm.input({INPUT_TEXT, "BTTMP", "Шаблон вывода по кнопке", "dhsnw"});
    crm.input({INPUT_CHECKBOX, "UDP", "UDP протокол", "0"});
    crm.input({INPUT_CHECKBOX, "CNFW", "Поиск новой версии ПО", "1", "1"});
    if (CNFW) {
      crm.input({INPUT_CHECKBOX, "FWAU", "Автоматическое обновление", "0"});
    }
    crm.output({OUTPUT_HR, "1px", "40px 20% 10px"});
    crm.input({INPUT_BUTTON, "RBT", "&#xe810;  Перезагрузить"});
    if (fw.newVer && !FWAU) {
      String n("Обновить до версии ");
      n += fw.str;
      crm.input({INPUT_BUTTON, "BTFW", n, "", "r"});
    }
  }


  {
    crm.page("&#xf1eb; Wi-Fi и Доступ");
    crm.wifiForm(WIFI_AP, "WiFi-CLOCK");
    crm.output({OUTPUT_HR, "1px", "40px 20% 10px"});
    crm.output({OUTPUT_LABEL, "", "Веб авторизация (&#xe810;)", "center", "#fff", "20"});
    crm.group(G_START);
    crm.input({INPUT_TEXT, "WEBL", "Логин", ""});
    crm.input({INPUT_PASSWORD, "WEBP", "Пароль", ""});
    crm.group(G_END);
    crm.input({INPUT_TEXT, "APIK", "API ключ", ""});
    crm.input({INPUT_BUTTON, "RBT", "&#xe810; Перезагрузить"});
  }


  {
    crm.page("&#xe80f; Справка");
    crm.output({OUTPUT_TEXT, "", F("&#xf21e; На печеньки и корм котейке!"), F(" <a href='http://qiwi.com/n/CRMDEV' target='_blank'>QIWI Кошелёк</a>"), "#0f0"});
    crm.output({OUTPUT_TEXT, "", F("Специальные символы"), F("&#xe810; - Параметры применятся только после перезагрузки.<br> <z>§</z> - Использование элемента зависит от состояния других."), "#ff5"});
    crm.output({OUTPUT_TEXT, "", F("Инструкция и Web установщик: <a href='https://wondercrm.github.io' target='_blank'>ОТКРЫТЬ</a>"), "", "#00bcd4"});
  }

}
