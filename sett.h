#define C_VER F("3.230217dev")
bool dbg = 1;


#define INCLUDE_vTaskDelayUntil 1
#define HTTPCLIENT_DEFAULT_TCP_TIMEOUT 15000
#define SENSOR_HISTORY_COUNT 72   // Точек истории показаний внешнего датчика
#define MAX_DEVICES 4             // Максимальное количество модулей в сборке
#define LED_MAX_BUF 512           // Размер буфера LED
#define HOLIDAY_COUNT 30          // Количество своих праздников
#define ALL_ALARM_COUNT 4         // Количство будильников

// Пины
#define CLK_PIN 18                // CLK LED PING Matrix
#define CS_PIN 19                 // CS LED PING Matrix
#define DIN_PIN 23                // Din LED PING Matrix
#define ANALOG_PIN 36             // VP, Добавить подтяжку ~1 МОм на GND
#define BT_PIN 39                 // Кнопка
#define SCL_PIN 22                // SCL DS3231, SCD30
#define SDA_PIN 21                // SDA DS3231, SCD30
#define DS18_PIN 15               // DS18B20
#define DFPlayerRX_PIN 17         // RX DFPlayer
#define DFPlayerTX_PIN 16					// TX DFPlayer
#define DFPlayerBusyu_PIN 13      // Status DFPlayer
#define BUSSER_PIN 32             // BUSSER
#define DEV_PIN 33             		// DEV


// Библиотеки
#include <CRMui3.h>
CRMui3 crm;


#include <Ticker.h>
Ticker brightTicker;
Ticker metricTicker;


#define EB_DEB 50
#define EB_CLICK 400
#include <EncButton2.h>
EncButton2<EB_BTN> bt(INPUT_PULLUP, BT_PIN);


#include <MD5Builder.h>


#include <HTTPClient.h>
#include <HTTPUpdate.h>


#include <MD_Parola.h>
#include "fonts.h"
MD_Parola M = MD_Parola(MD_MAX72XX::FC16_HW, DIN_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);  //FC16_HW
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))


#include <Wire.h>
#define RTC_CLOCK_ADRESS 0x68 // Адрес RTC модуля


#include <AsyncUDP.h>
AsyncUDP *udp;


#include "SparkFun_SCD30_Arduino_Library.h"
SCD30 scd30;  // адрес 0x61


#include <SparkFun_Qwiic_Humidity_AHT20.h>
AHT20 ath20;


#include <GyverBME280.h>
GyverBME280 bme;


#include <DallasTemperature.h>
#include <OneWire.h>
OneWire oneWireDS18(DS18_PIN);
DallasTemperature ds18(&oneWireDS18);


#include "DFRobotDFPlayerMini.h"
HardwareSerial dfSerial(1);
DFRobotDFPlayerMini dfPlayer;




// Главные
bool mutex = 1;
bool CNFW, FWAU;
struct {
  String str;
  bool check = 1;
  bool update;
  bool newVer;
} fw;
uint32_t ledTimeCorrect;



// Время и Дата
Ticker timeTicTicker;
Ticker timeSyncTicker;
struct tm TD;
uint32_t _WT;
bool NightMode, NM, RTCm;
uint8_t HPIK;
int TZ, NMS, NME;


// API
struct {
  String *mes = nullptr;
  uint32_t timer;
  uint32_t pause;
  bool status;
} apiMes;


// Праздники
bool DAT, DATY, HDNF, MYHD;
byte HDNL, TSUR, RGHD;


// Датчики
Ticker sensorTicker;
bool SNDNM, SCHRT;
uint8_t ESENS, dsCount;
int HOFFS, POFFS;
uint16_t ALTDE, SUDTI;
float TOFFS, T2OFFS;
struct {
  float T1;
  float T2;
  float H;
  uint16_t P;
  uint16_t CO2;
} sensor;
struct sensorHistory {
  uint16_t Time[SENSOR_HISTORY_COUNT];
  int T1[SENSOR_HISTORY_COUNT];
  int T2[SENSOR_HISTORY_COUNT];
  uint16_t H[SENSOR_HISTORY_COUNT];
  uint16_t P[SENSOR_HISTORY_COUNT];
  uint16_t CO2[SENSOR_HISTORY_COUNT];
} history;
sensorHistory* his = &history;


// Погода
Ticker weatherTicker;
bool weatherStatus;
uint8_t WTHR;
uint32_t WINT;
struct {
  String D;
  String WD;
  float T;
  float TR;
  float W;
  float WG;
  uint16_t P;
  uint8_t H;
} weather;


// Народный мониторинг
Ticker narmonTicker;
float narodmonVol[10][2];
bool NMON, nmNotEmpty, nmStatus;
uint32_t NMINT;


// UDP
bool UDP;


// Будильник
bool AM[ALL_ALARM_COUNT], alarmBeepActive, alarmMp3PlayStatus;
byte AMACT[ALL_ALARM_COUNT], AMPIN[ALL_ALARM_COUNT], AMFPS[ALL_ALARM_COUNT],
     AMSND[ALL_ALARM_COUNT];
uint32_t AMT[ALL_ALARM_COUNT], AMPTM[ALL_ALARM_COUNT];
bool AMD[ALL_ALARM_COUNT][7];
Ticker alarmTick[ALL_ALARM_COUNT];


// MP3
Ticker mp3Ticker;
int mp3InitStatus[12], APLS;
uint8_t PIKS, AVOL, AEQ, ATSND, APLF;


// LED
char data[LED_MAX_BUF];
bool BrightCorrect, fade;
float ADEEP;
uint8_t fontID, sMode, Bright, BMIN, BMAX, ASENS;
uint16_t BrightRAW, SMIN, SMAX, MMBR;
byte STYLE, TSPD, SSPD, VIEWT;
bool SEC, PMIG, HPIC, DBLH;
typedef struct {
  textEffect_t  effect;
  uint16_t      speed;
} sCatalog;
sCatalog  catalog[] = {
  {PA_SLICE, 5},               //"SLICE", 8
  {PA_MESH, 80},               //"MESH",
  {PA_FADE, 120},              //"FADE",
  {PA_WIPE, 20},               //"WIPE",
  {PA_WIPE_CURSOR, 20},        //"WIPE_CURSOR",
  {PA_OPENING, 24},            //"OPENING",
  {PA_OPENING_CURSOR, 24},     //"OPENING_CURSOR",
  {PA_CLOSING, 24},            //"CLOSING",
  {PA_CLOSING_CURSOR, 24},     //"CLOSING_CURSOR",
  {PA_BLINDS, 30},             //"BLIND",
  {PA_DISSOLVE, 120},          //"DISSOLVE",
  {PA_SCROLL_UP, 60},          //"SC_U",
  {PA_SCROLL_DOWN, 60},        //"SC_D",
  {PA_SCROLL_LEFT, 20},        //"SC_L",
  {PA_SCROLL_RIGHT, 20},       //"SC_R",
  {PA_SCROLL_UP_LEFT, 50},     //"SC_UL",
  {PA_SCROLL_UP_RIGHT, 50},    //"SC_UR",
  {PA_SCROLL_DOWN_LEFT, 50},   //"SC_DL",
  {PA_SCROLL_DOWN_RIGHT, 50},  //"SC_DR",
  {PA_RANDOM, 10},             //"PA_RANDOM"
  {PA_SCAN_HORIZ, 20},         //"SCANH",
  {PA_SCAN_VERT, 40},          //"SCANV",
  {PA_GROW_UP, 60},            //"GRW_U",
  {PA_GROW_DOWN, 60},          //"GRW_D",
};
