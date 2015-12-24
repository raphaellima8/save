  /*
 * Copyright (C) 2015 Raphael Lima
 * S.A.V.E - It is a software that manages the hardware that make up an entire autonomic system.
 *
 * This file is part of S.A.V.E.
 *
 * S.A.V.E is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * S.A.V.E is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
  #define pinoEchoFrente 13
  #define pinoTriggerFrente 12
  #define pinoEchoTras 11
  #define pinoTriggerTras 10
  #define dip_1 9
  #define dip_2 8
  #define ledLigado 7
  #define ledDesligado 6
  #define ledVidro 5
  #define buzzer 4
  
  int distancia = 0;
  int estado = 0;
  int distancias[30];
  double nivelConfianca = 1.96;
  double distanciaInicialFrontal = 0;
  double distanciaInicialTraseira = 0;
  double distanciaFinalFrontal = 0;
  double distanciaFinalTraseira = 0;
  boolean ciclo = true;
  double intervaloConfianca[2];
  int distanciaFiltrada[30];
  
  void setup() 
  {
    Serial.begin (9600);
    pinMode(pinoTriggerTras, OUTPUT);
    pinMode(pinoEchoTras, INPUT);
    pinMode(pinoTriggerFrente, OUTPUT);
    pinMode(pinoEchoFrente, INPUT);
    pinMode(ledLigado, OUTPUT);
    pinMode(ledDesligado, OUTPUT);
    pinMode(buzzer, OUTPUT);
    pinMode(ledVidro, OUTPUT);
    pinMode(dip_1, INPUT_PULLUP);
    pinMode(dip_2, INPUT_PULLUP);
  }
  
  int bin2int(int numValues, ...)
  { //Primeiro parametro e a quantidade de pinos que serao lidos, os "n" seguintes sao os pinos a serem lidos
    int total = 0;
    va_list values;
    va_start(values, numValues);
    
    for( ; numValues > 0; numValues--)
          if(!(digitalRead(va_arg(values, int))) )
              total += powint(2, numValues-1);
              
    va_end(values);
    return total;
  }
  
  int powint(int base, int exponent)
  {
    int result = 1;
    for( ; exponent > 0; exponent--)
          result *= base;
    return result;
  }
  
  void statusCarro(int acao)
  {
    switch(acao){
      case 0:
        digitalWrite(ledLigado, LOW);
        digitalWrite(ledDesligado, HIGH);
        break;
        
        case 1:
          digitalWrite(ledLigado, HIGH);
          digitalWrite(ledDesligado, LOW);
          break;
    }
  }
  
  void acionarAtuadores(boolean atdr)
  {
    digitalWrite(ledVidro, HIGH);
    delay(6000);
    digitalWrite(ledVidro, LOW);
      while(atdr) {
        tone(buzzer, 1500);
        delay(300);
        noTone(buzzer);
        digitalWrite(ledDesligado, LOW);
        delay(600);
        digitalWrite(ledDesligado, HIGH);
        
        estado = bin2int(2, dip_1,dip_2);
       
        if(estado == B010){
          atdr = false;
          ciclo = true;
        }
      }
  }
  
   
  
  void desligarSistema()
  {
    int distancia = 0;
    double distanciaInicialFrontal = 0;
    double distanciaInicialTraseira = 0;
    double distanciaFinalFrontal = 0;
    double distanciaFinalTraseira = 0;
    boolean ciclo = true;
  }
  
  int filtrarValoresDistancia(int distancia[], int tamanho)
  {
        int indice = 0;
        int media = 0;
        int soma = 0;
        int i = 0;      
        
        if(tamanho == 10){
           
           while(indice < 5){
               distancia[indice]  =  (distancia[i] + distancia[i+1]) / 2;
               indice++;
           }
           
           for(int j = 0; j < 5; j++){
             soma = soma + distancia[j];
           }   
        
           media = soma / 5;
           return media;
        
        } 
        else
        {
          while(indice < 10){
            distancia[indice]  =  (distancia[i] + distancia[i+1] + distancia[i+2]) / 3;
            indice++;
            i = i+3;
          }
             filtrarValoresDistancia(distancia, 10);
        }
  }
  
int obterDistancia(int trigger, int echo){
    for(int i = 0; i < 30; i++){
        distancias[i] = medirDistancia(trigger, echo);
        Serial.print(distancias[i]);
        Serial.print(" | ");
    }
    int medias = filtrarValoresDistancia(distancias, 30);
    Serial.print("Media: ");
    Serial.println(medias);
    return medias;
  }
  
  double calcularVariancia(int mediasDistancia[], int media){
    double variancia = 0;
    for(int i = 0; i < 30; i++){
       variancia = variancia + pow((mediasDistancia[i] - media), 2);
       Serial.print(variancia);
       Serial.print(" | ");
    }
    Serial.println();
    Serial.print("Variancia ");
    Serial.println(variancia/30);
    return variancia/30;
  }
  
  double desvioPadrao(double variancia){  
    Serial.print("Desvio Padrao ");
    Serial.println(sqrt(variancia));
    return sqrt(variancia);
  }
  
  void intervaloDeConfianca(int dists[]){
    int media = filtrarValoresDistancia(dists, 30);
    Serial.println("==================================================================================================");
    Serial.print("MEDIA TOTAL: " );
    Serial.println(media);
    double variancia = calcularVariancia(dists, media);
    double desvio = desvioPadrao(variancia);
    
    intervaloConfianca[0] = (media - nivelConfianca) * (variancia / desvio);
    intervaloConfianca[1] = (media + nivelConfianca) * (variancia / desvio);
  }
  
  int calcularMedia(int dist[]){
    int soma = 0;
    for(int i = 0; i < 30; i++){
        soma = soma + dist[i];
    }
    return (soma/30);
  }
  
  void loop() 
  { 
    estado = bin2int(2, dip_1,dip_2);

    switch(estado){
      case(B011)://Dois pinos nivel alto
          if(ciclo){
            statusCarro(1);
            for(int i = 0; i < 30; i++){
              distanciaFiltrada[i] = obterDistancia(pinoTriggerFrente, pinoEchoFrente);
              delay(3000);
            }
            intervaloDeConfianca(distanciaFiltrada);
            Serial.print("Limite inferior: ");
            Serial.println(intervaloConfianca[0]);
            Serial.print("Limite superior: ");
            Serial.println(intervaloConfianca[1]);
            distanciaInicialFrontal = intervaloConfianca[0];
            for(int i = 0; i < 30; i++){
              distanciaFiltrada[i] = obterDistancia(pinoTriggerTras, pinoEchoTras);
              delay(3000);
            }
            intervaloDeConfianca(distanciaFiltrada);
            Serial.print("Limite inferior: ");
            Serial.println(intervaloConfianca[0]);
            Serial.print("Limite superior: ");
            Serial.println(intervaloConfianca[1]);
            distanciaInicialTraseira = intervaloConfianca[0];
            ciclo = false;
          }
      break;
  
      case(B000): //Dois pinos nivel baixo
        statusCarro(0);
        delay(5000); //5 segundos, o ideal seria por volta de 2 minutos.
            for(int i = 0; i < 30; i++){
              distanciaFiltrada[i] = obterDistancia(pinoTriggerFrente, pinoEchoFrente);
              delay(3000);
            }
            intervaloDeConfianca(distanciaFiltrada);
            Serial.print("Limite inferior: ");
            Serial.println(intervaloConfianca[0]);
            Serial.print("Limite superior: ");
            Serial.println(intervaloConfianca[1]);
            distanciaFinalFrontal = intervaloConfianca[0];
            for(int i = 0; i < 30; i++){
              distanciaFiltrada[i] = obterDistancia(pinoTriggerTras, pinoEchoTras);
              delay(3000);
            }
            intervaloDeConfianca(distanciaFiltrada);
            Serial.print("Limite inferior: ");
            Serial.println(intervaloConfianca[0]);
            Serial.print("Limite superior: ");
            Serial.println(intervaloConfianca[1]);
            distanciaFinalTraseira = intervaloConfianca[0];
            if(distanciaFinalFrontal > distanciaInicialFrontal && distanciaFinalTraseira <= distanciaInicialTraseira){
                acionarAtuadores(true);
            }
      break;
      
      case(B010):
            desligarSistema();      
      break;
      
      case(B001):

      break;
        
      }
    delay(1000);
  }
  
  int medirDistancia(int trigger, int echo)
  {
    long duration, distance;
    
    digitalWrite(trigger, LOW);  // Added this line
    
    delayMicroseconds(2); // Added this line
    
    digitalWrite(trigger, HIGH);
    
    delayMicroseconds(10); // Added this line
    
    digitalWrite(trigger, LOW);
    
    duration = pulseIn(echo, HIGH);
    distance = (duration/2) / 29.1;
    
    return distance;
  }
