//inclusão de bibliotecas
#include <SPI.h>
#include <Wire.h>
#include <RadioLib.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "ThingSpeak.h"

//Define a pinagem da tela OLED
#define oled_scl 18
#define oled_sda 17
#define oled_rst 21

//Pinagem do modulador do radio SX1262
#define LoRa_MOSI 10
#define LoRa_MISO 11
#define LoRa_SCK 9

// Pinagem do transmissor de radio SX1262
#define LoRa_nss 8
#define LoRa_dio1 14
#define LoRa_nrst 12
#define LoRa_busy 13

//WiFi
const char* ssid = "GAYA";          // your network SSID (name)
const char* password = "06042004";  // your network password

WiFiClient client;

//ThingSpeak
unsigned long myChannelNumber = 1;
const char* myWriteAPIKey = "NC5WVTK4C9TY2YOJ";

unsigned long lastTime = 0;
unsigned long timerDelay = 20000;

//ArduinoJson
DynamicJsonDocument doc(2048);

// Define o radio
SX1262 radio = new Module(LoRa_nss, LoRa_dio1, LoRa_nrst, LoRa_busy);

//Define o OLED
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, oled_scl, oled_sda, oled_rst);

//Conexao InfoData
int rssi = 0;
float snr = 0;
String packSize = "--";
bool flagReceive = false;

//Troca tela
int countSwap = 0;
int ind = 0;
char swap = 'a';
const char charSel[] = { 'a', 'b', 'c' };


/* Protótipo da função */
void LoRaDataRSSI();
void LoRaDataSNR();
void LoRaDataRate();
void cbk(int packetSize);
void setFlag();

/*
  Nome da função: LoRaDataRSSI
  objetivo: imprime a RSSI/Potencia do sinal e tamanho do pacote recebido no display.
*/

void setFlag(){
  flagReceive = true;
}

void LoRaDataRSSI() {

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x13_tf);
    u8g2.setCursor(5, 18);
    u8g2.print("Recebendo " + String(packSize) + " bytes");
    u8g2.setFont(u8g2_font_7x14_tf);
    u8g2.setCursor(5, 48);
    u8g2.print("RSSI " + String(rssi) + " dBm");
  } while (u8g2.nextPage());

  Serial.println("RSSI " + String(rssi) + " dBm");
}

/*
  Nome da função: LoRaDataSNR
  objetivo: imprime a SNR/Ruído do sinal e tamanho do pacote recebido no display.
*/

void LoRaDataSNR() {

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x13_tf);
    u8g2.setCursor(5, 18);
    u8g2.print("Recebendo " + String(packSize) + " bytes");
    u8g2.setFont(u8g2_font_7x14_tf);
    u8g2.setCursor(5, 48);
    u8g2.print("SNR " + String(snr) + " dB");
  } while (u8g2.nextPage());

  Serial.println("SNR " + String(snr) + " dB");
}

/*
  Nome da função: cbk
  recebe como parâmetos um inteiros (packetSize)
  objetivo: recebe a temperatura via LoRa e armazena na variável packet.
*/
void cbk(int packetSize, String dat) {
  packSize = String(packetSize, DEC);
  rssi = radio.getRSSI();
  snr = radio.getSNR();
  //Serial.println(dat);
  deserializeJson(doc, dat);
  String eixoX = doc["EixoX"];
  String eixoY = doc["EixoY"];
  String eixoZ = doc["EixoZ"];
  String alarm = doc["Alarme"];
  String packetS = doc["Packet"];

  int eixoXI = eixoX.toInt();
  int eixoYI = eixoY.toInt();
  int eixoZI = eixoZ.toInt();
  int alarmI = alarm.toInt();
  int packetSI = packetS.toInt();
  //serializeJson(doc,Serial);

  //Logica para seleção da tela de informação
  countSwap++;
  if (countSwap > 4) {
    countSwap = 0;
    ind++;
    if (ind > 1) {
      ind = 0;
    }
  }

  swap = charSel[ind];

  //Seleciona a informação p/ o Display do OLED
  switch (swap) {
    case 'a':
      LoRaDataRSSI();
      break;
    case 'b':
      LoRaDataSNR();
      break;
  }

  if ((millis() - lastTime) > timerDelay) {

    ThingSpeak.setField(1, eixoXI);
    ThingSpeak.setField(2, eixoYI);
    ThingSpeak.setField(3, eixoZI);
    ThingSpeak.setField(4, alarmI);
    ThingSpeak.setField(5, packetSI);
    ThingSpeak.setField(6, rssi);
    ThingSpeak.setField(7, snr);

    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if (x == 200) {
      Serial.println("Channel update successful.");
    } else {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    lastTime = millis();


  }
}

/******************* função principal (setup) *********************/
void setup() {
  pinMode(LED, OUTPUT);  //inicializa o LED
  u8g2.begin();

  Serial.begin(115200);

  u8g2.clear();

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x13_tf);
    u8g2.drawStr(0, 16, "Iniciado com Sucesso!");
    u8g2.drawStr(0, 42, "Aguardando dados...");
  } while (u8g2.nextPage());

  Serial.println("Iniciado com Sucesso!");
  Serial.println("Aguardando os dados...");
  delay(1000);

  WiFi.begin(ssid, password);
  delay(10000);

  ThingSpeak.begin(client);  // Initialize ThingSpeak

  radio.begin();  // Habilita o rádio LoRa para receber dados

  int state = radio.startReceive();

  if (state == RADIOLIB_ERR_RX_TIMEOUT) {

    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_7x14_tf);
      u8g2.setCursor(5, 18);
      u8g2.print("Queda na conexao!");
      u8g2.setFont(u8g2_font_6x13_tf);
      u8g2.setCursor(5, 48);
      u8g2.print("Esperando resposta...");
    } while (u8g2.nextPage());

    Serial.println("Perda na conexão");
    while(state == RADIOLIB_ERR_RX_TIMEOUT);
  }

  radio.setDio1Action(setFlag);

}

/******************* função em loop (loop) *********************/
void loop() {
  delay(1);

  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Attempting to connect");
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      delay(10000);
    }
    Serial.println("\nConnected.");
  }

  if (flagReceive) {  //Verifica se há dados chegando via LoRa
    flagReceive = false;

    String str;
    int packetSize;

    radio.readData(str, packetSize);

    cbk(packetSize, str);            // Espera 500 milissegundos
    Serial.println("Recebendo!");
  }
}
