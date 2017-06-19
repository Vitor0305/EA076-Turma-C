#include <stdio.h>
#include <Wire.h>
#include <TimerOne.h>
#include <LiquidCrystal_I2C.h>

//Definicao do pino do sensor de temperatura
#define temp A0
//Definicao do pino de controle do aquecedor (rele)
#define rele 8
//Definicao do pino de controle do ventilador
#define vent 9
//Definicao do botao de standby
#define standby 4
//Definicao dos botoes de regulacao de faixa
#define up 5
#define down 7
//Definicao do pino do botao
#define botao 6
// Definicao do pino buzzer
#define pin_buzzer 10
// Definicoes dos pinos ligados ao sensor de gas
#define pin_gas A2
//Definicao do pino do sensor de umidade
#define umidade A3

//Ajuste do sensor de gas
int nivel_sensor = 200;

//Ajuste da faixa de temperatura
int tempmax = 33, tempmin = 30;

// Inicializa o display no endereco 0x27
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void setup()
{
  //Define o número de colunas e linhas do LCD
  lcd.begin(16, 2);
  // Define os pinos de leitura do sensor de gas como entrada
  pinMode(pin_gas, INPUT);
  // Define pino buzzer como saida
  pinMode(pin_buzzer, OUTPUT);
  //Define vent como saida
  pinMode(vent, OUTPUT);
  //Define controle do rele como saida
  pinMode(rele, OUTPUT);
  //Define standby como entrada
  pinMode(standby, INPUT);
  //Define controle de faixa como entrada
  pinMode(up, INPUT);
  pinMode(down, INPUT);
  //Define botao como entrada
  pinMode(botao, INPUT); 
  //Define umidade como entrada
  pinMode(umidade, INPUT);
  // Inicializa a serial
  Serial.begin(9600);
}

int nbotao = 0, flag_button = 0, flag_buttondown = 0, flag_buttonup=0, valor_analogico2, counter=0, nothing=0;
float temperatura;

void loop() {
  //Se o botao de standby for acionado, desliga-se o sistema de aquecimento e refrigeracao, para entrar no modo standby
  if(digitalRead(standby) && nothing==0){
    digitalWrite(rele, LOW);
    analogWrite(vent, 0);
    lcd.clear();
    nothing++;
  }
  //Sai do modo standby, apenas quando o botao for acionado novamente
  while(nothing>0 && nothing<4){
    if(digitalRead(standby)==0 && nothing==1) nothing++;
    if(digitalRead(standby) && nothing==2) nothing++;
    if(digitalRead(standby)==0 && nothing==3) nothing++;
    lcd.setCursor(0,0);
    lcd.print("STANDBY");
    Serial.println("STANDBY");
  }
  if(nothing==4) nothing=0;
  
  // Le os dados do pino analogico de temperatura
  int valor_analogico = analogRead(pin_gas);
  
  // Verifica o nivel de gas/fumaca detectado
  if (valor_analogico > nivel_sensor)
  {
    // Liga o buzzer
    digitalWrite(pin_buzzer, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("ALERTA");
  }
  else
  {
    // Desliga o buzzer
    digitalWrite(pin_buzzer, LOW);
  }
  
  //Regula faixa de temperatura
  if(digitalRead(up)){
    flag_buttonup=1;
  }
  if(flag_buttonup && digitalRead(up)==0){
    tempmax++;
    tempmin++;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("TempMin: ");
    lcd.print(tempmin);
    lcd.setCursor(0,1);
    lcd.print("TempMax: ");
    lcd.print(tempmax);
    counter=-10;
    flag_buttonup=0;
  }
  
  if(digitalRead(down)){
    flag_buttondown=1;
  }
  if(flag_buttondown && digitalRead(down)==0){
    tempmax--;
    tempmin--;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("TempMin: ");
    lcd.print(tempmin);
    lcd.setCursor(0,1);
    lcd.print("TempMax: ");
    lcd.print(tempmax);
    counter=-10;
    flag_buttondown=0;
  }

  //Sinaliza que botao foi apertado
  if (digitalRead(botao)) {
    flag_button = 1;
  }
  //Indica que botao foi solto apos ser apertado
  if (flag_button && digitalRead(botao) == 0) {
    nbotao++;
    if (nbotao == 2) {
      nbotao = 0;
    }
    flag_button = 0;
  }

  counter++;

  //Acoes realizadas a cada 1 segundo
  if(counter>10){
    temperatura = (float(analogRead(temp)) * 5 / (1023)) / 0.01;
    valor_analogico2 = analogRead(umidade);
    Serial.println(valor_analogico2);

    if (temperatura > tempmax) {
      analogWrite(vent, 255);
      lcd.setCursor(0, 1);
      lcd.print("VENT ACIONADO");
    }
    if(temperatura < (tempmax+1)) analogWrite(vent, 0);
    
    if (temperatura < tempmin) {
      digitalWrite(rele, HIGH);
      lcd.setCursor(0, 1);
      lcd.print("AQUEC ACIONADO");
    }
    if(temperatura > (tempmin+1))  digitalWrite(rele, LOW);
    
    if(temperatura > tempmin && temperatura < tempmax){
      lcd.setCursor(0,1);
      lcd.print("                ");
    }

      //Posiciona o cursor na coluna 0, linha 0;
      lcd.setBacklight(HIGH);
      lcd.setCursor(0, 0);
      switch (nbotao) {
        //Modo temperatura, mostra o valor da temperatura no LCD
        case 0:
          Serial.println(temperatura);
          lcd.print("Temp: ");
          lcd.setCursor(6, 0);
          lcd.print(temperatura);
          break;
        //Modo umidade, mostra o valor da umidade no LCD
        case 1:
          //Le o valor do pino A0 do sensor
          valor_analogico = analogRead(umidade);
          Serial.println(valor_analogico);
          if (valor_analogico < 500) lcd.print("Umid: Solo Umido");
          if (valor_analogico >= 500 && valor_analogico < 700) lcd.print("Umid: Moderada");
          else lcd.print("Umid: Solo Seco ");
          break;
      }
    
    counter=0;
  }
  //Apenas para diminuir a frequencia do loop!! (para 10Hz)
  delay(100);
}
