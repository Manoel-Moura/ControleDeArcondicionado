// INCLUSÃO DE BIBLIOTECAS
#include <ESP8266WiFi.h>
#include "seguranca.h"
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include "uFire_SHT20.h"



//OBJETO SHT20 PARA LEITURA DO SENSOR
uFire_SHT20 sht20;

// DECLARAÇÃO DE VARIÁVEIS GLOBAIS
const long tempoDeEnvio = 600000;
unsigned long tempoAnterior = 0;
double temperatura;
double umidade;
int cont = 0;             //Contador para enviar 3 mensagens de "Ar-condicionado desligou!"
#define BOT_TOKEN "2063266093:AAEd_NNik81K4gCj_G2TiJIuh2H_3AD9We4"  //chave Token Bot Telegram
String ADMINISTRADOR = "1425097270";  // ID DO BOT DO ADMINISTRADOR
#define R1 2    // GPIO_14 
int Bot_mtbs = 1000; 
long Bot_lasttime;   
int amostra = 20;



// DECLARAÇÃO DE VARIÁVEIS PARA WIFI
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;
const char* host =  SECRET_HOST;




// cliente SSL necessario para a Biblioteca
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);





// DECLARAÇÃO DE FUNÇÕES
void InitWiFi();
void  EnviaProBD();
void ConexaoTelegram();
double MediaTemperatura(int amostra);
double MediaUmidade(int amostra);


void setup() {
  Serial.begin(115200);
  pinMode(R1, OUTPUT);
  digitalWrite(R1, LOW);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // ESPERA O SERIAL ESTAR PRONTO
  while (!Serial);

  InitWiFi();
  ConexaoTelegram();

}

void loop() {
  unsigned long tempoAtual = millis();
  temperatura =  MediaTemperatura(amostra); // Leitura da Temperatura
  umidade = MediaUmidade(amostra); //Leitura da Umidade

  if (temperatura >= 24 && cont < 3) { 
    bot.sendMessage(ADMINISTRADOR, "Ar-condicionado desligou! ", "");
    cont++;
  }
  if (temperatura < 23 && cont == 3) {
    bot.sendMessage(ADMINISTRADOR, "Ar-condicionado foi ligado!", "");
    cont = 0;
  }

  if (millis() > Bot_lasttime + Bot_mtbs) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {

      for (int i = 0; i < numNewMessages; i++) {

        String chat_id = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;
        String from_name = bot.messages[i].from_name;


        if (chat_id != ADMINISTRADOR) {
          bot.sendMessage(ADMINISTRADOR, from_name + " solicitou " + text, "");
        }

        if (text == "TEMPERATURA" || text == "Temperatura" || text == "temperatura") {
          bot.sendMessage(chat_id, from_name + "\nTemperatura: " + (String)temperatura + " °C", "");
        }

        if (text == "UMIDADE" || text == "Umidade" || text == "umidade") {
          bot.sendMessage(chat_id, from_name +  "\nUmidade: " + (String)umidade + " RH%", "");
        }
        if (text == "STATUS" || text == "Status" || text == "status") {
          bot.sendMessage(chat_id, from_name +  "\nTemperatura: " + (String)temperatura + " °C\n" + "Umidade: " + (String)umidade + " RH%", "");
        }


        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }

      Bot_lasttime = millis();
    }


  }
 
  if(tempoAtual - tempoAnterior >= tempoDeEnvio){
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
  // print out info about the connection:
  Serial.print("My IP address is: ");
  Serial.println(WiFi.localIP());
}


void  EnviaProBD() {



  WiFiClient client;
  const int httpPort = 8080;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  client.print(String("GET http://10.107.4.5/iot_mysql/conexaoFred.php?") +
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
void ConexaoTelegram(){
     // Conecta cliente com SSL

  client.setFingerprint("F2:AD:29:9C:34:48:DD:8D:F4:CF:52:32:F6:57:33:68:2E:81:C1:90");
  while (!client.connect("api.telegram.org", 443))
  {
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
  }
  bot.sendMessage(ADMINISTRADOR, "Estou ligado e pronto para transmitir! ", ""); //Informa quando estiver ligado e pronto para transmitir

}


double MediaTemperatura(int amostra) {   //Função para calcular a media de 20 leituras de Temperatura
  double media = 0.0;

  for (int i = 0 ; i < amostra; i++) {
    media +=  sht20.temperature();
  }
  return media / amostra;
}

double MediaUmidade(int amostra) { //Função para calcular a media de 20 leituras de Umidade
  double media = 0.0;

  for (int i = 0 ; i < amostra; i++) {
    media +=  sht20.humidity();
  }
  return media / amostra;
}
