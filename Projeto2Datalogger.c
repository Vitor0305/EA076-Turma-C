  /* PROJETO 2 - EA076 - DATALLOGER
   *  JOEL FIGUEIREDO DE REZENDE      RA 155957
   *  VITOR MENDES DO AMARAL          RA 157555
   *  
   * Créditos:
   * Prof. Tiago Tavares : Código que demonstra um kernel de tempo real e implementação de protocolos de comunicação.
   * Site ArduinoeCia (http://www.arduinoecia.com.br): Cógido base do teclado matricial
 */

#include <stdio.h>
#include <Wire.h>
#include <Keypad.h>
#include <TimerOne.h>

//Definicao da quantidade de linhas e colunas
const byte LINHAS = 4;
const byte COLUNAS = 3;

//Matriz de caracteres
char matriz_teclas[LINHAS][COLUNAS] = 
{
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'#','0','*'}
};

//Definicao dos pinos das linhas
byte PinosLinhas[LINHAS] = {5, 7, 6, 8};
//Definicao dos pinos das colunas
byte PinosColunas[COLUNAS] = {2, 4, 3};

//Inicializa o teclado
Keypad meuteclado = Keypad( makeKeymap(matriz_teclas), PinosLinhas, PinosColunas, LINHAS, COLUNAS);    
 
/* Rotina auxiliar para comparacao de strings */
int str_cmp(char *s1, char *s2, int len) {
  int i;
  for (i=0; i<len; i++) {
    if (s1[i] != s2[i]) return 0;
    if (s1[i] == '\0') return 1;
  }
  return 1;
}

#define LDR A0
#define LED 11 
/* Buffer de dados recebidos */
#define MAX_BUFFER_SIZE 15
#define MAX_ARRAY_SIZE 10

typedef struct {
  char data[MAX_BUFFER_SIZE];
  unsigned int tam_buffer;
} serial_buffer;

serial_buffer Buffer;

// Limpa buffer
void buffer_clean() {
  Buffer.tam_buffer = 0;
}

// Adiciona caractere ao buffer
int buffer_add(char c_in) {
  if (Buffer.tam_buffer < MAX_BUFFER_SIZE) {
    Buffer.data[Buffer.tam_buffer++] = c_in;
    return 1;
  }
  return 0;
}

/* Flags globais para controle de processos da interrupcao */
int flag_check_command = 0, pisca_flag=0, automatic_flag=0, evenON=0, first_time=1;

//Ao receber evento da UART
void serialEvent() {
  char c;
  while (Serial.available()) {
    c = Serial.read();
    if (c=='\n') {
      buffer_add('\0'); /* Se recebeu um fim de linha, coloca um terminador de string no buffer */
      flag_check_command = 1;
    } else {
     buffer_add(c);
    }
  }
}

#define END_EXT 0x50    //endereço do 24LC256  B1010 000

// Funções de escrita e leitura da memória externa
void writeEEPROM(int deviceaddress, unsigned int eeaddress, byte data ) {
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress));   // MSB
  Wire.write(data);
  Wire.endTransmission();
  delay(5);
}
byte readEEPROM(int deviceaddress, unsigned int eeaddress )  {
  byte rdata = 0xF;
 
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress));   // MSB
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress,1);
  if (Wire.available()) rdata = Wire.read();
  return rdata;
}

//Declaracao dos contadores
int timer=0;

void ISR_timer(){
    timer++;
}

unsigned int indice = 0, t = 0;
char teclas[3];

void setup() {
  buffer_clean();
  Timer1.initialize(10000);
  Timer1.attachInterrupt(ISR_timer);
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  Wire.begin(); 
  indice = (int)readEEPROM(END_EXT, 0); 
}

void loop() {
  int valueldr, N, i, flag_write = 0, possible_command_flag=0;
  char out_buffer[30], *s;
  byte c;

//Quando um comando é escrito na entrada serial, verifica-se o que fazer
  if (flag_check_command) {
    if (str_cmp(Buffer.data, "PING", 4) ) {
      sprintf(out_buffer, "PONG\n");
      flag_write = 1;
    }
    if (str_cmp(Buffer.data, "ID", 2) ) {
      sprintf(out_buffer, "DATALOGGER DO JOEL E DO VITOR\n");
      flag_write = 1;
    }
    if (str_cmp(Buffer.data, "MEASURE", 7) ) {
      valueldr = (analogRead(LDR)/4);
      sprintf(out_buffer, "LDRvalue = %d\n", valueldr );
      flag_write = 1;
    }
    if (str_cmp(Buffer.data, "RECORD", 6) ) {
      valueldr = (analogRead(LDR)/4);
      sprintf(out_buffer, "LDRvalue = %d\n", valueldr );
      writeEEPROM(END_EXT, indice+1, valueldr);
      indice++;
      writeEEPROM(END_EXT, 0, indice);
      flag_write = 1;
    }
    if (str_cmp(Buffer.data, "GET N", 4) ){
     s = Buffer.data;
     for (int i=0; i<1024; i++){                         //Cria um string "s" com apenas o numero N                          
        s[i] = s[i+4];
        if(s[i+4]=='\0'){
          break;
        }
     }
     N = atoi(s);
     if(N<=indice){
      c = readEEPROM(END_EXT, N);
      sprintf(out_buffer, "memOUT%d: %d\n", N, c);
      flag_write = 1;
     }
     else{
      sprintf(out_buffer, "Error, this is trash\n");
      flag_write = 1;
     }
    }
    if (str_cmp(Buffer.data, "RESET", 5) ){
      indice=0;
      writeEEPROM(END_EXT, 0, indice);
    }
    if (str_cmp(Buffer.data, "MEMSTATUS", 9) ){
      sprintf(out_buffer, "Number of elements %d\n", indice);
      flag_write = 1;
    }
    buffer_clean();
    flag_check_command = 0;
  }

//Verifica se alguma tecla foi pressionada
char tecla_pressionada = meuteclado.getKey();

//Se uma tecla foi pressionada, armazena o caracter em um vetor
  if (tecla_pressionada){
    teclas[t] = tecla_pressionada;
    t++;
  }
  
//Quando foi escrito 3 caracteres no vetor, verifica se é um comando válido
  if(t==3){
    Serial.println(teclas);
    //Aciona flag de possivel comando
    if(teclas[0]== '#' && isdigit(teclas[1]) && teclas [2] == '*'){
      possible_command_flag = 1;
      t=0;
    }
    if(teclas[1]== '#' && isdigit(teclas[2])){
      teclas[0] = teclas[1];
      teclas[1] = teclas[2];
      t=2;
    }
    if(teclas[2]== '#'){
      teclas[0] = teclas[2];
      t=1;
    }
    if(t==3){
      t=0;
    }
  }

//Quando a flag de possivel comando for igual a 1, verifica qual o digito do comando
  if(possible_command_flag){
    switch(teclas[1]){
      case '1':
        pisca_flag=1;
        break;
      case '2':
        valueldr = (analogRead(LDR)/4);
        sprintf(out_buffer, "LDRvalue = %d\n", valueldr );
        writeEEPROM(END_EXT, indice+1, valueldr);
        indice++;
        writeEEPROM(END_EXT, 0, indice);
        flag_write = 1;
        break;
      case '3':
        automatic_flag=1;
        break;
     case '4':
        automatic_flag=0;
        break;
    }
  }
  
//Verifica a cada 1 segundo se a flag "pisca_flag" é igual a 1
  if(timer%100==0){
    if(first_time){                       //A variavel "timer" é de 0,01s, no entanto a frequencia do loop é maior. A variavel "first_time" sincroniza o tempo.
      if(pisca_flag){
       switch(evenON){
       case 0:
         evenON++;
         digitalWrite(LED, HIGH);
         break;
       case 1:
         evenON++;
         digitalWrite(LED, LOW);
         break;
       case 2:
         evenON++;
         digitalWrite(LED, HIGH);
         break;
       case 3:
         evenON=0;
         pisca_flag=0;
         digitalWrite(LED, LOW);
         break;
       }
       first_time=0;
     } 
   }
 }
 else{
  first_time=1; 
 }

//Quando "automatic_flag" é 1, inicializa a medicao automatica 
 if(automatic_flag){
   if(timer%100==0){
     valueldr = (analogRead(LDR)/4);
     sprintf(out_buffer, "LDRvalue = %d\n", valueldr );
     writeEEPROM(END_EXT, indice+1, valueldr);
     indice++;
     writeEEPROM(END_EXT, 0, indice);
     flag_write = 1;
   }
 }

//Se "flag_write" é 1, escreve na saida serial
  if (flag_write == 1) {
    Serial.write(out_buffer);
    flag_write = 0;
  } 
}
