#define base 6
#define rpm 2
#define MAX_BUFFER_SIZE 15

#include <stdio.h>
#include <Wire.h>
#include <TimerOne.h>

int flag_check_command = 0, checkspdcounter=0;

void ISR_timer(){
    checkspdcounter=1;
}

void setup() {
  Timer1.initialize(1000000);                                         //Interrupção gerada a cada 1 segundo 
  Timer1.attachInterrupt(ISR_timer);
  pinMode(rpm, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(rpm), spdcounter, FALLING);   //Interrupção gerada a cada borda de descida no pino 2 (rpm) para contar o numero de voltas
  pinMode(base, OUTPUT);
  Serial.begin(9600);
}

//Declaração de uma estrutura de buffer
typedef struct {
  char data[MAX_BUFFER_SIZE];
  unsigned int tam_buffer;
} serial_buffer;

serial_buffer Buffer;

// Limpa buffer
void buffer_clean() {
  Buffer.tam_buffer = 0;
}

//Adiciona caracter a caracter no buffer
int buffer_add(char c_in) {
  if (Buffer.tam_buffer < MAX_BUFFER_SIZE) {
    Buffer.data[Buffer.tam_buffer++] = c_in;
    return 1;
  }
  return 0;
}

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

unsigned int vmotor, rpminput, velocity, erro, spd=0;
float kp;

//Função interrompida a cada borda de descida (quando o sensor é bloqueado) para contar o numero de voltas que as hélices passaram pelo sensor
void spdcounter(){
  spd++;
}

void loop() {
  //Se a flag for um, significa que foi digitado um valor de RPM no entrada serial
  if(flag_check_command){
    rpminput= atoi(Buffer.data);
    vmotor=map(rpminput, 0, 5600, 0, 255);                                    //Faz a conversão do valor em RPM para a escala de 0 a 255 variando a tensão no motor
    flag_check_command=0;
    buffer_clean();
  }
  analogWrite(base, vmotor);                                                  //Manda o valor do motor para o pino 6 (base) que regula a tensão
  //A cada 1 segundo, checkspdcounter==1, logo mede-se a velocidade do motor, convertendo para RPM (dividido por 2, pois há duas hélices)
  if(checkspdcounter){
    velocity = spd*60/2;
    erro=rpminput-velocity;
    kp=0,005;                                                                 //Sistema realimentado para controlar o erro
    velocity=velocity+kp*erro;
    Serial.println(velocity);
    checkspdcounter=0;
    spd=0;
  }
}
