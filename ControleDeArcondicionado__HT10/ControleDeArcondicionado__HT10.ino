// INCLUSÃO DE BIBLIOTECAS
#include <ESP8266WiFi.h>
#include "seguranca.h"
#include <WiFiClientSecure.h>
#include <Adafruit_AHTX0.h>
//#include "Utilities.h" // for int64ToAscii() helper function

#include "CTBot.h"
CTBot myBot;


//OBJETO AHTX0 PARA LEITURA DO SENSOR
Adafruit_AHTX0 aht;

// DECLARAÇÃO DE VARIÁVEIS GLOBAIS
const long tempoDeEnvio = 120000; // 2 min
unsigned long tempoAnterior = 0;
double temperatura;
double umidade;
#define R1 2    // GPIO_14 
int amostra = 10;
sensors_event_t humidity, temp;
int IRledPin =  13;


//Variaveis do bot
String mensagem = "\t\t\tLista de Comandos\n\n- Temperatura\n\n- Umidade\n\n- Status\n\n\t\t\tCOMANDOS DO ADMINISTRADOR\n\n- Ligar\n\n- Desligar\n\n(Valores entre 17 e 22) - Muda a temperatura do ar-condicionado\n";
int ADMINISTRADOR = 1425097270;  // ID DO BOT DO ADMINISTRADOR
#define BOT_TOKEN "2063266093:AAEd_NNik81K4gCj_G2TiJIuh2H_3AD9We4"  //chave Token Bot Telegram




// DECLARAÇÃO DE VARIÁVEIS PARA WIFI
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;
const char* host =  SECRET_HOST;




// cliente SSL necessario para a Biblioteca
WiFiClientSecure client;





// DECLARAÇÃO DE FUNÇÕES
void InitWiFi();
void  EnviaProBD();
void ConexaoTelegram();
double MediaTemperatura(int amostra);
double MediaUmidade(int amostra);
void desliga();
void liga();
void setTemp (int temp);


void setup() {
  Serial.begin(115200);
  aht.begin();
  pinMode(R1, OUTPUT);
  digitalWrite(R1, LOW);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(IRledPin, OUTPUT);

  // ESPERA O SERIAL ESTAR PRONTO
  while (!Serial);
  InitWiFi();
  ConexaoTelegram();
  temperatura =  MediaTemperatura(50);
  umidade = MediaUmidade(50);
  EnviaProBD();
}

void loop() {
  unsigned long tempoAtual = millis();

  if (WiFi.status() != WL_CONNECTED) {
    InitWiFi();
    ConexaoTelegram();
  }

  TBMessage msg;
  //Testa se o bot recebeu menssagem
  if (CTBotMessageText == myBot.getNewMessage(msg)) {

    String MensagemRecebida = msg.text;
    MensagemRecebida.toUpperCase();
    int novaTemp = MensagemRecebida.toInt();

    //Lista de comandos
    if (MensagemRecebida == "/START") {
      myBot.sendMessage(msg.sender.id, (String)mensagem, "");
    }

    //Comandos publicos
    //=========================================================================================================================================================================================================
    else if (MensagemRecebida == "TEMPERATURA" || MensagemRecebida == "/TEMPERATURA") {
      temperatura =  MediaTemperatura(amostra);
      myBot.sendMessage(msg.sender.id, msg.sender.firstName + " " + msg.sender.lastName + "\nTemperatura: " + (String)temperatura + " °C");
    }
    else if (MensagemRecebida == "UMIDADE" || MensagemRecebida == "/UMIDADE") {
      umidade = MediaUmidade(amostra);//Leitura da Umidade
      myBot.sendMessage(msg.sender.id, msg.sender.firstName + " " + msg.sender.lastName +  "\nUmidade: " + (String)umidade + " RH%", "");
    }
    else if (MensagemRecebida == "STATUS" || MensagemRecebida == "/STATUS") {
      temperatura =  MediaTemperatura(amostra);
      umidade = MediaUmidade(amostra);
      myBot.sendMessage(msg.sender.id, msg.sender.firstName + " " + msg.sender.lastName +  "\nTemperatura: " + (String)temperatura + " °C\n" + "Umidade: " + (String)umidade + " RH%", "");
    } else
      myBot.sendMessage(msg.sender.id, msg.sender.firstName + " " + msg.sender.lastName +
                        "\nComando não encontrado, para acessar a lista de comandos envie '/start'");

    //=========================================================================================================================================================================================================

    //Comandos de Administrador
    if (msg.sender.id != ADMINISTRADOR) {
      myBot.sendMessage(ADMINISTRADOR, msg.sender.firstName + " " + msg.sender.lastName  + "\nChat ID: " + int(msg.group.id) + "\nSolicitou: " + MensagemRecebida, "");
    }
    else if (msg.sender.id == ADMINISTRADOR && (novaTemp >= 17 && novaTemp <= 22)) {

      myBot.sendMessage(ADMINISTRADOR, "Temperatura atualizada para " + (String)novaTemp + " °C", "");
      setTemp(novaTemp);
    }
    else if (msg.sender.id == ADMINISTRADOR && (novaTemp < 17 || novaTemp > 22) && novaTemp != 0) {
      myBot.sendMessage(ADMINISTRADOR, " Por segurança a temperatura só pode variar entre 17 e 22 °C", "");
    }
    else if (msg.sender.id == ADMINISTRADOR && (MensagemRecebida == "LIGAR" || MensagemRecebida == "/LIGAR")) {
      liga();
      myBot.sendMessage(ADMINISTRADOR, " Arcondicionado Ligado!", "");
    }
    else if (msg.sender.id == ADMINISTRADOR && (MensagemRecebida == "DESLIGAR" || MensagemRecebida == "/DESLIGAR")) {
      desliga();
      myBot.sendMessage(ADMINISTRADOR, " Arcondicionado Desligado!", "");
    }

  }


  // Envia para o Banco de Dados
  if (tempoAtual - tempoAnterior >= tempoDeEnvio) {
    temperatura =  MediaTemperatura(50);
    umidade = MediaUmidade(50);
    EnviaProBD();
    tempoAnterior = tempoAtual;
  }


}




// Inicia conexão Wifi
void InitWiFi() {
  Serial.println("\n\nConectando-se ao AP ...");
  // Inicia conexão com Wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
  }
  Serial.println("Conexão Wifi estabelecida");
  Serial.print("My IP address is: ");
  Serial.println(WiFi.localIP());
}


void  EnviaProBD() {

  WiFiClient client;
  const int httpPort = 8181;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  client.print(String("GET http://10.107.4.2/apiFredPiripiri.php?") +
               ("&temperatura=") + temperatura +
               ("&umidade=") + umidade +
               " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 1000 ) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.println(line);
  }


  Serial.println("\nclosing connection");
}
void ConexaoTelegram() {

  TBMessage msg;
  myBot.wifiConnect(ssid, password);

  myBot.setTelegramToken(BOT_TOKEN);
  if (myBot.testConnection()) {
    Serial.println("Conexao Ok!");
    myBot.sendMessage(ADMINISTRADOR, "Estou ligado e pronto para transmitir!", "");
  }
  else
    Serial.println("Falha na conexao!");
}


double MediaTemperatura(int amostra) {   //Função para calcular a media da Temperatura
  double media = 0.0;
  sensors_event_t humidity, temp;
  // Serial.println( temp.temperature);
  for (int i = 0 ; i < amostra; i++) {
    aht.getEvent(&humidity, &temp);

    media +=  temp.temperature;
  }
  return media / amostra;
}

double MediaUmidade(int amostra) { //Função para calcular a media da Umidade
  double media = 0.0;
  sensors_event_t humidity, temp;
  for (int i = 0 ; i < amostra; i++) {
    aht.getEvent(&humidity, &temp);
    media +=  humidity.relative_humidity;
  }
  return media / amostra;
}

void pulseIR(long microsecs) {
  // we'll count down from the number of microseconds we are told to wait

  cli();  // this turns off any background interrupts

  while (microsecs > 0) {
    // 38 kHz is about 13 microseconds high and 13 microseconds low
    digitalWrite(IRledPin, HIGH);  // this takes about 3 microseconds to happen
    delayMicroseconds(10);         // hang out for 10 microseconds
    digitalWrite(IRledPin, LOW);   // this also takes about 3 microseconds
    delayMicroseconds(10);         // hang out for 10 microseconds

    // so 26 microseconds altogether
    microsecs -= 26;
  }

  sei();  // this turns them back on
}
