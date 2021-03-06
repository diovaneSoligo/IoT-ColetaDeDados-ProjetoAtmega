//*************Bibliotecas*****************
#include <SPI.h>
#include <Ethernet.h> //Ethernet Shield
#include "EmonLib.h"  //Sensor de Corrente
#include "Tensao.h"
//#include <LiquidCrystal.h>

//LiquidCrystal lcd(9, 8, 7, 6, 5, 3);

//************Config p/ Ethernet Shield****
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0x0C, 0xF1 }; //em exadecimal 0x + nº
byte ip[] = { 192 , 168 , 90 , 119 };                      // IP do protótipo
byte gateway[] = { 192 , 168, 80, 1 };                //IP do gateway
byte subnet[] = { 255, 255, 240, 0 };                //Máskara de Subrede
byte servidor[] = { 192 , 168 , 90 , 210 };            //Servidor JAVA
EthernetServer server(1000);                         //Acesso pela porta 1000
EthernetClient cliente;

//***********Entrada_Sensores**************
int CORRENTE_PIN = A5; //Sensor de Corrente no pino A4 (alterar pino projeto)
Tensao tensao(A4);
//**********Saidas*************************
int PIN_RELE = A2;
String id = "001"; //id do projeto
//***********Calibracao********************
EnergyMonitor corrente;
//****************************************
int val;
void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando programa...");
  //lcd.begin(16, 2);
  //lcd.print("PHANTOM DS  V1.0");
  //lcd.setCursor(0, 1);
  //lcd.print("INICIANDO ...");
  Ethernet.begin(mac, ip);// Inicializa o Server com o IP e Mac atribuido acima
  Serial.print("Server Ativo no IP: ");
  Serial.println(Ethernet.localIP());
  server.begin();

  pinMode(PIN_RELE, OUTPUT);
  delay(500);
  //Serial.println(">>>LIGA RELE<<<\n");
  //digitalWrite(PIN_RELE, HIGH); //Aciona o rele

  delay(500);
  //analogReference(INTERNAL);
  corrente.current(CORRENTE_PIN, 19); //Calibração sensor corrente
  Serial.println(">>>setup OK<<<\n");


}
void loop() {
  
/*******captura informações de corrente*********/
  int f = 0;
  double Irms;
  while (f < 10) {
    Irms = corrente.calcIrms(1480);
    f++;
  }

  if (Irms < 0.09) {
    Irms = 0.00;
  }
  Serial.print("\nvalor da corrente ");
  Serial.println(Irms);
  
/*******captura informações de tensão*********/
  float pino = tensao.media();
  int volts = 0;
  volts = pino;
  Serial.print("Tensao: ");
  Serial.print(pino); //tensão.valor() - valor lido pelo sensor real-time
  Serial.println(" volts");

  Serial.print("Potencia: ");
  Serial.print(volts * Irms);
  Serial.println(" Watts\n****************\n");

  //lcd.setCursor(0, 0);
  //lcd.print("V: ");
  //lcd.print(volts);
  //lcd.print(" | A: ");
  //lcd.print(Irms);

  //int portaRele = 0;
  //portaRele = digitalRead(PIN_RELE);

  //lcd.setCursor(0, 1);
  //lcd.print("               ");

  //lcd.setCursor(0, 1);
  //if (portaRele = HIGH) {
  //  lcd.print(" ON");
  //} else {
  //  lcd.print("OFF");
  //}

  //lcd.print(" | W: ");
  //lcd.print(volts * Irms);
  //lcd.print("    ");

/***converte os valores em string e os concatena no metodo GET para envialos ao servidor***/
  String V = String(volts);
  String C = String(Irms);

  String ENVIA = "GET /?ID=";
  ENVIA.concat(id);
  ENVIA.concat("&VOLTS=");
  ENVIA.concat(V);
  ENVIA.concat("&CORRENTE=");
  ENVIA.concat(C);

  //ENVIA = "GET /?ID=1@1&VOLTS=219&CORRENTE=0,1123";

  if (cliente.connect(servidor, 8080)) {
    Serial.println("CONECTADO...");

    cliente.print(ENVIA);
    cliente.println(" HTTP/1.1");

    cliente.println("Host: www.google.com");
    cliente.println("Connection: close");
    cliente.println();
  } else {
    Serial.println("ERRO NA CONEXAO...");
  }

  delay(1000);
  cliente.stop();

/***Verifica se existe robo conectado e sua acao***/


  EthernetClient client = server.available();
  if (client)
  {

    boolean continua = true; // A requisição HTTP termina com uma linha em branco Indica o fim da linha
    String linha;

    while (client.connected())
    {

      if (client.available())
      {

        char c = client.read(); //Variável para armazenar os caracteres que forem recebidos
        linha.concat(c); // Pega os valor após o IP do navegador ex: 192.168.1.2/0001        
        
        if (c == '\n' && continua)
        {
                            client.println("HTTP/1.1 200 OK");

                            // IMPORTANTE, ISSO FAZ O ARDUINO RECEBER REQUISIÇÃO AJAX DE OUTRO SERVIDOR E NÃO APENAS LOCAL.
                            client.println("Content-Type: text/javascript");
                            client.println("Access-Control-Allow-Origin: *");
                            client.println();

                            int iniciofrente = linha.indexOf("?");

                            if(iniciofrente>-1){ //verifica se existe comando

                              iniciofrente  = iniciofrente+6; // pega o caracter seguinte
                              int fimfrente = iniciofrente+3; //espera 3 caracteres
                              
                              String acao   = linha.substring(iniciofrente,fimfrente);//pega o valor do comando

                              if(acao == "001"){ digitalWrite(PIN_RELE, HIGH);val=1;}
                              else
                              if(acao == "002"){ digitalWrite(PIN_RELE, LOW);val=0;}
                              else{}

                              

                              client.print("dados({ rele : ");
                              client.print(val);
                              client.print(" })");
                              }
                            break;
                            
        } //Fecha if (c == '\n' && continua)

         if(c == '\n'){
                        continua = true;
                      }else
                      if(c != '\r'){
                        continua = false;
                      }
        
      } //Fecha if (client.available())
      
    } //Fecha While (client.connected())
    
    delay(1);// Espera um tempo para o navegador receber os dados
    client.stop(); // Fecha a conexão
    
  } //Fecha if(client)


  
}
