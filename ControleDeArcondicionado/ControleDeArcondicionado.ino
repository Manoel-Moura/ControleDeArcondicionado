sta#include <UniversalTelegramBot.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "uFire_SHT20.h"
int cont = 0;             //Contador para enviar 3 mensagens de "Ar-condicionado desligou!"
float temperatura = 0.0;  
float umidade = 0.0;
String manoel = "1425097270";  // id do meu chat_bot

uFire_SHT20 sht20;

char ssid[] = "NPD-Estagiarios";               // nome do seu roteador WIFI (SSID)
char password[] = "tanaporta";    // senha do roteador WIFI

#define BOT_TOKEN "2063266093:AAEd_NNik81K4gCj_G2TiJIuh2H_3AD9We4"  // sua chave Token Bot Telegram
// Para obter o Chat ID, acesse Telegram => usuario IDBot => comando /getid

// cliente SSL necessario para a Biblioteca
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

#define R1 2    // GPIO_14 
int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done

//***************************************************************************************

void setup()
{
  Serial.begin(115200);

  // Inicializa as entradas e saidas

  pinMode(R1, OUTPUT);
  digitalWrite(R1, LOW);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  //***************************************************************************************

  // Inicializa conexão Wifi

  WiFi.mode(WIFI_AP_STA);   // Configura o WIFI para modo estação e Acess point
  WiFi.disconnect();        // desconecta o WIFI
  delay(100);               // atraso de 100 milisegundos

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)    // aguardando a conexão WEB
  {
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
  }

  // Conecta cliente com SSL

  client.setFingerprint("F2:AD:29:9C:34:48:DD:8D:F4:CF:52:32:F6:57:33:68:2E:81:C1:90");
  while (!client.connect("api.telegram.org", 443))
  {
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
  }
  bot.sendMessage(manoel, "Estou ligado e pronto para transmitir! ", ""); //Informa quando estiver ligado e pronto para transmitir

}

float temp() {   //Função para calcular a media de 20 leituras de Temperatura
  float media = 0.0;

  for (int i = 0 ; i < 20; i++) {
    media +=  sht20.temperature();
  }
  return media / 20;
}

float umi() { //Função para calcular a media de 20 leituras de Umidade
  float media = 0.0;

  for (int i = 0 ; i < 20; i++) {
    media +=  sht20.humidity();
  }
  return media / 20;
}

void loop()
{
  temperatura = temp(); // Leitura da Temperatura
  umidade = umi(); //Leitura da Umidade
  {
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
  }
  if (temperatura >= 24 && cont < 3) { 
    bot.sendMessage(manoel, "Ar-condicionado desligou! ", "");
    cont++;
  }
  if (temperatura < 23 && cont == 3) {
    bot.sendMessage(manoel, "Ar-condicionado foi ligado!", "");
    cont = 0;
  }

  if (millis() > Bot_lasttime + Bot_mtbs) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {

      for (int i = 0; i < numNewMessages; i++) {

        String chat_id = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;
        String from_name = bot.messages[i].from_name;


        if (chat_id != manoel) {
          bot.sendMessage(manoel, from_name + " solicitou " + text, "");
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
}
