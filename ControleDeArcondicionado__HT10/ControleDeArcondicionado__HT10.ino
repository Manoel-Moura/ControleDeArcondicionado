// INCLUSÃO DE BIBLIOTECAS
#include <ESP8266WiFi.h>
#include "seguranca.h"
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <Adafruit_AHTX0.h>




//OBJETO AHTX0 PARA LEITURA DO SENSOR
Adafruit_AHTX0 aht;

// DECLARAÇÃO DE VARIÁVEIS GLOBAIS
const long tempoDeEnvio = 300000; // 5 min
unsigned long tempoAnterior = 0;
double temperatura;
double umidade;
int cont = 0;             //Contador para enviar 3 mensagens de "Ar-condicionado desligou!"
#define BOT_TOKEN "2063266093:AAEd_NNik81K4gCj_G2TiJIuh2H_3AD9We4"  //chave Token Bot Telegram
String ADMINISTRADOR = "1425097270";  // ID DO BOT DO ADMINISTRADOR
#define R1 2    // GPIO_14 
int Bot_mtbs = 1000; 
long Bot_lasttime;   
int amostra = 10;
sensors_event_t humidity, temp;
int IRledPin =  13; 



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

}

void loop() {
  unsigned long tempoAtual = millis();
  temperatura =  MediaTemperatura(amostra); // Leitura da Temperatura
  umidade = MediaUmidade(amostra); //Leitura da Umidade
// /

//  if (temperatura >= 24 && cont < 3) { 
//    bot.sendMessage(ADMINISTRADOR, "Ar-condicionado desligou! ", "");
//    cont++;
//  }
//  if (temperatura < 23 && cont == 3) {
//    bot.sendMessage(ADMINISTRADOR, "Ar-condicionado foi ligado!", "");
//    cont = 0;
//  }

  if (millis() > Bot_lasttime + Bot_mtbs) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {

      for (int i = 0; i < numNewMessages; i++) {

        String chat_id = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;
        String from_name = bot.messages[i].from_name;
        int novaTemp = text.toInt();
        String mensagem = "\t\t\tLista de Comandos\n\n(TEMPERATURA,Temperatura,temperatura) - retorana a temperatura;\n\n(UMIDADE,Umidade,umidade) - Retorana a umidade;\n\n(STATUS,Status,status) - Retorana a temperatura e umidade;\n\n\t\t\tCOMANDOS DO ADMINISTRADOR\n\n(LIGAR,Ligar,ligar) - Liga o ar-condicionado\n\n(DESLIGAR,Desligar,desligar) - Desliga o ar-condicionado\n\n(Valores entre 17 e 22) - Muda a temperatura do ar-condicionado\n";
          
         
         
         


        if (chat_id != ADMINISTRADOR) {
          bot.sendMessage(ADMINISTRADOR, from_name + " solicitou " + text, "");
        }
        if (chat_id == ADMINISTRADOR && (novaTemp >= 17 && novaTemp <= 22)) {
         
          bot.sendMessage(ADMINISTRADOR, "Temperatura atualizada para " + (String)novaTemp +" °C", "");
          setTemp(novaTemp);
        }
         if (chat_id == ADMINISTRADOR && (novaTemp < 17 || novaTemp > 22) && novaTemp != 0) {
          bot.sendMessage(ADMINISTRADOR, " Por segurança a temperatura só pode variar entre 17 e 22 °C", "");
        }
        if (chat_id == ADMINISTRADOR && (text == "ligar" || text == "Ligar" || text == "LIGAR")) {
          liga();
          bot.sendMessage(ADMINISTRADOR, " Arcondicionado Ligado!", "");
        }
        if (chat_id == ADMINISTRADOR && (text == "DESLIGAR" || text == "Desligar" || text == "desligar")) {
          desliga();
          bot.sendMessage(ADMINISTRADOR, " Arcondicionado Desligado!", "");
        }
        

        if (text == "TEMPERATURA" || text == "Temperatura" || text == "temperatura") {
          bot.sendMessage(chat_id, from_name + "\nTemperatura: " + (String)temperatura + " °C", "");
        }
        if (text == "/start") {
          bot.sendMessage(chat_id,(String)mensagem, "");
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
   sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  Serial.println( temp.temperature);
  for (int i = 0 ; i < amostra; i++) {
    media +=  temp.temperature;
  }
  return media / amostra;
}

double MediaUmidade(int amostra) { //Função para calcular a media de 20 leituras de Umidade
  double media = 0.0;
   sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  Serial.println(humidity.relative_humidity);
  for (int i = 0 ; i < amostra; i++) {
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
