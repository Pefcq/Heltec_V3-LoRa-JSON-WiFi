//inclusão de bibliotecas
#include <SPI.h>
#include <Wire.h>
#include <RadioLib.h>
#include <U8g2lib.h>

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

#define BAND    915E6  //Escolha a frequência

// Define o radio
SX1262 radio = new Module(LoRa_nss, LoRa_dio1, LoRa_nrst, LoRa_busy);

//Define o OLED
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/oled_scl, /* data=*/oled_sda, /* reset=*/oled_rst);

//Protótipo da função de enviar
void sendPacket();

/*
    Nome da função: sendPacket
    objetivo: envia a Mensagem via LoRa.
*/
void sendPacket()
{
  int statusr = radio.transmit("Hellow!");
  Serial.print(statusr);
}

/******************* função principal (setup) *********************/
void setup()
{
  Serial.begin(115200);

  //Inicia a modulação da antena
  SPI.begin(LoRa_SCK, LoRa_MISO, LoRa_MOSI, LoRa_nss);
  u8g2.begin();
  radio.begin();  
  
  u8g2.firstPage();
  do
  {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0, 24, "Iniciado");
    u8g2.drawStr(0, 47,"com Sucesso!");
  } while (u8g2.nextPage());
  
  delay(1000);
}

/******************* função em loop (loop) *********************/
void loop()
{ 
  Serial.println("Enviando:");
  Serial.print("HeLLow!");
  Serial.println("--------");

  u8g2.firstPage();
  do
  {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0, 24, "Enviando dados:");
    u8g2.drawStr(0,47, "Hellow!");
  } while (u8g2.nextPage());

  delay(1000);

  sendPacket(); //Envia Mssg
}
