//*************Bibliotecas*****************
#include <SPI.h>
#include <Ethernet.h> //Ethernet Shield
#include "EmonLib.h"  //Sensor de Corrente
#include "Tensao.h"

//************Config p/ Ethernet Shield****
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0x0C, 0xF1 }; //em exadecimal 0x + nº
byte ip[] = { 192 , 168 , 0 , 110 };                      // IP do protótipo
byte gateway[] = { 192 , 168, 0, 1 };                //IP do gateway
byte subnet[] = { 255, 255, 255, 0 };                //Máskara de Subrede
byte servidor[] = { 192 , 168 , 0 , 187 };            //Servidor JAVA
EthernetServer server(1000);                         //Acesso pela porta 1000
EthernetClient cliente;

//***********Entrada_Sensores**************
int CORRENTE_PIN = A5; //Sensor de Corrente no pino A4 (alterar pino projeto)
Tensao tensao(A4);
//**********Saidas*************************
int PIN_RELE = 7;
int id = 1; //id do projeto
//***********Calibracao********************
EnergyMonitor corrente;
//****************************************
void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando programa...");

  Ethernet.begin(mac, ip);// Inicializa o Server com o IP e Mac atribuido acima
  Serial.print("Server Ativo no IP: ");
  Serial.println(Ethernet.localIP());
  server.begin();

  pinMode(PIN_RELE, OUTPUT);
  delay(500);
  Serial.println(">>>LIGA RELE<<<\n");
  digitalWrite(PIN_RELE, HIGH); //Aciona o rele

  delay(500);
  //analogReference(INTERNAL);
  corrente.current(CORRENTE_PIN, 22); //Calibração sensor corrente
  Serial.println(">>>setup OK<<<\n");


}
void loop() {
  digitalWrite(PIN_RELE, HIGH);
  
  int f = 0;
  double Irms;
  while (f < 10) {
    Irms = corrente.calcIrms(1480);
    f++;
  }

  if(Irms < 0.09){
    Irms = 0.00;
    }
  Serial.print("\nvalor da corrente ");
  Serial.println(Irms);

  float pino = tensao.media();
  int volts = 0;
  volts = pino;
  Serial.print("Tensao: ");
  Serial.print(pino); //tensão.valor() - valor lido pelo sensor real-time
  Serial.println(" volts");

  //Serial.print("Tensao Armazena: ");
  //Serial.print(volts);
  //Serial.println(" volts");

  Serial.print("Potencia: ");
  Serial.print(volts * Irms);
  Serial.println(" Watts\n****************\n");



  if (cliente.connect(servidor, 8080)) {
    Serial.println("CONECTADO...");

    cliente.print("GET /serverIoT/recebe_dados?");
    cliente.print("VCA=");
    cliente.print(volts);
    cliente.print("&ACA=");
    cliente.print(Irms);
    cliente.print("&ID=");
    cliente.print(id);
    cliente.println(" HTTP/1.1");

    cliente.println("Host: www.google.com");
    cliente.println("Connection: close");
    cliente.println();

    //cliente.stop();
  } else {
    Serial.println("ERRO NA CONEXAO...");
    //cliente.stop();
  }

  delay(1000);
  cliente.stop();
}
