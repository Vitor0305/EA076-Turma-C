# EA076-Turma-C

//Projeto 01 da disciplina EA076 - Semáforo
// Turma C - Joel Figueiredo de Rezende(155957)/ Vitor Mendes do Amaral(157555)

#include <TimerOne.h>

#define LEDverdeCarro 10
#define LEDamarelo 9
#define LEDvermelhoCarro 11
#define LEDverdePed 7
#define LEDvermelhoPed 8
#define botao 6
#define LDR A0

int botaoON=0, timer=0, meiosegundo=0;

//Contadores: timer incrementado a cada 10ms e meiosegundo incrementado a cada 0,5s.
void ISR_timer(){
    timer++;
    if(timer%50==0){
      meiosegundo++;
    }
}

// Função setup é executada uma única vez ao inicializar ou reinicializar o arduíno.
void setup() {
    Timer1.initialize(10000);
    Timer1.attachInterrupt(ISR_timer);
    pinMode(LEDverdeCarro, OUTPUT);
    pinMode(LEDamarelo, OUTPUT);
    pinMode(LEDvermelhoCarro, OUTPUT);
    pinMode(LEDvermelhoPed, OUTPUT);
    pinMode(LEDverdePed, OUTPUT);
    pinMode(botao, INPUT);
    Serial.begin(9600);
}

//Função que é responsável por fechar o semáforo dos carros e abrir para o do pedestre, quando o botão é acionado.
void ciclopedestre(){
  meiosegundo=0;
  while(meiosegundo<4){                       //Espera 2s após o acionamento do botão.
    Serial.println("Botao Acionado");
  }
  digitalWrite(LEDverdeCarro, LOW);           
  digitalWrite(LEDamarelo, HIGH);
  while(meiosegundo<11){                      //Espera 3,5s na luz amarela para os carros.
    Serial.println("Espere");
  }
  digitalWrite(LEDamarelo, LOW);
  digitalWrite(LEDvermelhoCarro, HIGH);
  digitalWrite(LEDvermelhoPed, LOW);
  digitalWrite (LEDverdePed, HIGH);
  while(meiosegundo<31){                      //Permanece 10s aberto para a passagem do pedestre.
    Serial.println("Atravesse");
  }
  digitalWrite(LEDverdePed, LOW);
  int i=0;  
  do{                                         //Pisca vermelho para alertar o pedestre.
     digitalWrite(LEDvermelhoPed, HIGH);
     while(meiosegundo<(32+2*i)){
       Serial.println("Pare");
     }
     digitalWrite(LEDvermelhoPed, LOW);
     while(meiosegundo<(33+2*i)){
       Serial.println("PARE");
     }
     i++;
    }while(i<4);
  digitalWrite(LEDvermelhoCarro, LOW);
}

//Função responsável por verificar se de fato é noite e, caso seja, chamar a função piscaamarelo().
void estadonoite(){
  meiosegundo=0;
  while(meiosegundo<8){                     
    Serial.println("Verificando se é noite");
    botaoON = (digitalRead(botao) || botaoON);
    if(botaoON || (analogRead(LDR)<700)){      //Se o botão for acionado ou não for de fato noite, retorna.
      break;
    }
  }  
  if(meiosegundo >= 8){                        //Após 4s de confirmação do estado noite, chama a função piscaamarelo().
      piscaamarelo();
      botaoON=0;
  }
}

//Função responsável por acionar o modo noturno do semáforo.
void piscaamarelo(){
  int dia=0;
  digitalWrite(LEDverdeCarro, LOW);
  digitalWrite(LEDvermelhoPed, LOW);
  do{
    meiosegundo=0;
    while(meiosegundo<1){
      Serial.println("Amarelo piscando");
    }
    digitalWrite(LEDamarelo, HIGH);
    while(meiosegundo<2){
      Serial.println("Amarelo piscando");
    }
    digitalWrite(LEDamarelo, LOW);
    if(analogRead(LDR)<700){                  
      dia++;
    }
    else{
      dia=0;
    }
  }while(dia<4);                               //Condição para retornar ao estado dia (padrão).
}

//Estado dia (padrão)
void loop() {
  digitalWrite(LEDverdeCarro, HIGH);   
  digitalWrite(LEDvermelhoPed, HIGH);
  if(analogRead(LDR)>700){                     //Chamada da função estadonoite() quando há diminuição da luminosidade no LDR 
      estadonoite();
  }
  Serial.println("Estado Dia");
  botaoON = (botaoON || digitalRead(botao));   //Verifica se o botão foi acionado
  if(botaoON){
     ciclopedestre();
     botaoON = 0;
  }
}
