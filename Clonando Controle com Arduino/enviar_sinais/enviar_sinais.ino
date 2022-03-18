// CODIGO EM CC MODIFICADO POR NERDKINGTEAM
// VISITE: nerdking.com.br
// youtube.com/nerdkingteam
// This sketch will send out a Nikon D50 trigger signal (probably works with most Nikons)
// See the full tutorial at http://www.ladyada.net/learn/sensors/ir.html
// this code is public domain, please enjoy!
 
int IRledPin =  13;    // LED connected to digital pin 13
int Botao = 8;
int EstadoBotao = 0;
// The setup() method runs once, when the sketch starts
 
void setup()   {
// initialize the IR digital pin as an output:
pinMode(IRledPin, OUTPUT);
pinMode(Botao, INPUT); 
Serial.begin(9600);
}
 
void loop()
{
   EstadoBotao = digitalRead(true);
Serial.println("Sending IR signal");
if (EstadoBotao == HIGH){ 
SendChannelUpCode();
delay(500); // Altere este delay para melhores resultados
}
}
// This procedure sends a 38KHz pulse to the IRledPin
// for a certain # of microseconds. We'll use this whenever we need to send codes
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
 
void SendChannelUpCode() {
delayMicroseconds(27576);
pulseIR(9300);
delayMicroseconds(4600);
pulseIR(620);
delayMicroseconds(1680);
pulseIR(620);
delayMicroseconds(540);
pulseIR(620);
delayMicroseconds(560);
pulseIR(600);
delayMicroseconds(560);
pulseIR(620);
delayMicroseconds(540);
pulseIR(620);
delayMicroseconds(560);
pulseIR(600);
delayMicroseconds(560);
pulseIR(620);
delayMicroseconds(540);
pulseIR(620);
delayMicroseconds(540);
pulseIR(620);
delayMicroseconds(1700);
pulseIR(620);
delayMicroseconds(1680);
pulseIR(620);
delayMicroseconds(1700);
pulseIR(600);
delayMicroseconds(1700);
pulseIR(620);
delayMicroseconds(1680);
pulseIR(620);
delayMicroseconds(1680);
pulseIR(620);
delayMicroseconds(1700);
pulseIR(600);
delayMicroseconds(560);
pulseIR(620);
delayMicroseconds(540);
pulseIR(620);
delayMicroseconds(560);
pulseIR(600);
delayMicroseconds(1700);
pulseIR(620);
delayMicroseconds(1680);
pulseIR(620);
delayMicroseconds(560);
pulseIR(600);
delayMicroseconds(560);
pulseIR(640);
delayMicroseconds(520);
pulseIR(620);
delayMicroseconds(1700);
pulseIR(600);
delayMicroseconds(1700);
pulseIR(600);
delayMicroseconds(1700);
pulseIR(620);
delayMicroseconds(540);
pulseIR(620);
delayMicroseconds(560);
pulseIR(600);
delayMicroseconds(1700);
pulseIR(620);
delayMicroseconds(1680);
pulseIR(620);
delayMicroseconds(1700);
pulseIR(600);
delayMicroseconds(40960);
pulseIR(9300);
delayMicroseconds(2280);
pulseIR(620);
}
