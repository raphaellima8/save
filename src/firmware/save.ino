/*
 * Copyright (C) 2015 Raphael Lima
 * SAVE - Autonomous System for Parked Vehicles
 *
 * This file is part of SAVE.
 *
 * SAVE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SAVE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

 /*
  Project: SAVE - Autonomous System for Parked Vehicles
  @author Raphael Lima
  @version 0.1
 */

  #define echoPinFront 13
  #define triggerPinFront 12
  #define echoPinBack 11
  #define triggerPinBack 10
  #define dip_1 9
  #define dip_2 8
  #define ledOn 7
  #define ledOff 6
  #define ledWindow 5
  #define buzzer 4
  const int weightPin = A0;

  int distance = 0;
  int state = 0;
  int distances[30];
  const double CONFIDENCE_LEVEL = 1.96;
  double initialDistanceFront = 0;
  double initialDistanceBack = 0;
  double finalDistanceFront = 0;
  double finalDistanceBack = 0;
  boolean cicle = true;
  double confidenceInterval[2];
  int filtratedDistance[30];
  int weight = 0;

  int bin2int(int numValues, ...);
  int powint(int base, int exponent);
  void carStatus(int action);
  void activateActuators(boolean atdr);
  void shutdown();
  int refineDistanceValues(int distance[], int size);
  int getDistance(int trigger, int echo);
  double calculateVariance(int distancesAverages[], int average);
  double standardDeviation(double variance);
  void getConfidenceInterval(int dists[]);
  int calculateAverage(int dist[]);
  int toMeasureDistance(int trigger, int echo);
  int getWeight();

  void setup()
  {
    Serial.begin (9600);
    pinMode(triggerPinBack, OUTPUT);
    pinMode(echoPinBack, INPUT);
    pinMode(triggerPinFront, OUTPUT);
    pinMode(echoPinFront, INPUT);
    pinMode(ledOn, OUTPUT);
    pinMode(ledOff, OUTPUT);
    pinMode(buzzer, OUTPUT);
    pinMode(ledWindow, OUTPUT);
    pinMode(dip_1, INPUT_PULLUP);
    pinMode(dip_2, INPUT_PULLUP);
  }
/*
  Read the DIP switch configuration and convert the result to binary
  @param amount of pins that will be read
  @param pins that will be read
  @return the numerical value of the color code.
*/
  int bin2int ( int numValues, ... )
  {
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

  /*
    Indicates if the car is on or off.
    Case @param equal 0, vehicle is Off, but if @param equal 1, vehicle is on
    @param int 0 or 1 condition that will indicate if vehicle is on or off
  */
  void carStatus(int action)
  {
    switch(action){
      case 0:
        digitalWrite(ledOn, LOW);
        digitalWrite(ledOff, HIGH);
      break;

      case 1:
        digitalWrite(ledOn, HIGH);
        digitalWrite(ledOff, LOW);
      break;
    }
  }

  /*
  Activates the actuators as the buzzer and LED's. Only stop when @param atdr is
  false and DIP switch is at low logic level.
  @param boolean adtr
  */
  void activateActuators(boolean atdr)
  {
    digitalWrite(ledWindow, HIGH);
    delay(6000);
    digitalWrite(ledWindow, LOW);
    while(atdr) {
      tone(buzzer, 1500);
      delay(300);
      noTone(buzzer);
      digitalWrite(ledOff, LOW);
      delay(600);
      digitalWrite(ledOff, HIGH);

      state = bin2int(2, dip_1,dip_2);

      if(state == B010){
        atdr = false;
        cicle = true;
      }
    }
  }

  /*
  Turn off the system and reset the values of the variables
  */
  void shutdown()
  {
    distance = 0;
    initialDistanceFront = 0;
    initialDistanceBack = 0;
    finalDistanceFront = 0;
    finalDistanceBack = 0;
    cicle = true;
  }

  int getWeight()
  {
      return analogRead(weightPin);
  }

  int refineDistanceValues(int distance[], int size)
  {
        int index = 0, average = 0, sum = 0, i = 0;

        if(size == 10){
           while(index < 5){
            distance[index]  =  (distance[i] + distance[i+1]) / 2;
            index++;
           }
           for(int j = 0; j < 5; j++){
            sum = sum + distance[j];
           }
           average = sum / 5;
           return average;
        } else {
            while(index < 10){
              distance[index]  =  (distance[i] + distance[i+1] + distance[i+2]) / 3;
              index++;
              i = i+3;
            }
            refineDistanceValues(distance, 10);
        }
  }

  int getDistance(int trigger, int echo)
  {
    for(int i = 0; i < 30; i++){
        distances[i] = toMeasureDistance(trigger, echo);
        Serial.print(distances[i]);
        Serial.print(" | ");
    }
    int averages = refineDistanceValues(distances, 30);
    Serial.print("Average: ");
    Serial.println(averages);
    return averages;
  }

  double calculateVariance(int distancesAverages[], int average)
  {
    double variance = 0;
    for(int i = 0; i < 30; i++){
       variance = variance + pow((distancesAverages[i] - average), 2);
       Serial.print(variance);
       Serial.print(" | ");
    }
    Serial.println();
    Serial.print("Variance ");
    Serial.println(variance/30);
    return variance/30;
  }

  double standardDeviation(double variance)
  {
    Serial.print("Standard Deviation ");
    Serial.println(sqrt(variance));
    return sqrt(variance);
  }

  void getConfidenceInterval(int dists[])
  {
    int average = refineDistanceValues(dists, 30);
    Serial.println("==========================================================");
    Serial.print("Total Average: " );
    Serial.println(average);
    double variance = calculateVariance(dists, average);
    double deviation = standardDeviation(variance);

    confidenceInterval[0] = (average - CONFIDENCE_LEVEL) * (variance / deviation);
    confidenceInterval[1] = (average + CONFIDENCE_LEVEL) * (variance / deviation);
  }

  int calculateAverage(int dist[])
  {
    int sum = 0;
    for(int i = 0; i < 30; i++){
        sum = sum + dist[i];
    }
    return (sum/30);
  }

  void loop()
  {
    state = bin2int(2, dip_1,dip_2);

    switch(state){
      case(B011)://Dois pinos nivel alto
          if(cicle){
            carStatus(1);
            for(int i = 0; i < 30; i++){
              filtratedDistance[i] = getDistance(triggerPinFront, echoPinFront);
              delay(3000);
            }
            getConfidenceInterval(filtratedDistance);
            Serial.print("Inferior Limit: ");
            Serial.println(confidenceInterval[0]);
            Serial.print("Upper Limit: ");
            Serial.println(confidenceInterval[1]);
            initialDistanceFront = confidenceInterval[0];
            for(int i = 0; i < 30; i++){
              filtratedDistance[i] = getDistance(triggerPinBack, echoPinBack);
              delay(3000);
            }
            getConfidenceInterval(filtratedDistance);
            Serial.print("Inferior Limit: ");
            Serial.println(confidenceInterval[0]);
            Serial.print("Upper Limit: ");
            Serial.println(confidenceInterval[1]);
            initialDistanceBack = confidenceInterval[0];
            weight = getWeight();
            cicle = false;
          }
      break;

      case(B000): //Dois pinos nivel baixo
        carStatus(0);
        delay(5000); //5 segundos, o ideal seria por volta de 2 minutos.
            for(int i = 0; i < 30; i++){
              filtratedDistance[i] = getDistance(triggerPinFront, echoPinFront);
              delay(3000);
            }
            getConfidenceInterval(filtratedDistance);
            Serial.print("Inferior Limit: ");
            Serial.println(confidenceInterval[0]);
            Serial.print("Upper Limit: ");
            Serial.println(confidenceInterval[1]);
            finalDistanceFront = confidenceInterval[0];
            for(int i = 0; i < 30; i++){
              filtratedDistance[i] = getDistance(triggerPinBack, echoPinBack);
              delay(3000);
            }
            getConfidenceInterval(filtratedDistance);
            Serial.print("Inferior Limit: ");
            Serial.println(confidenceInterval[0]);
            Serial.print("Upper Limit: ");
            Serial.println(confidenceInterval[1]);
            finalDistanceBack = confidenceInterval[0];
            weight = getWeight();
            if(finalDistanceFront > initialDistanceFront && finalDistanceBack <= initialDistanceBack && weight > 0){
                activateActuators(true);
            } else if (finalDistanceFront > initialDistanceFront && finalDistanceBack > initialDistanceBack && weight > 0){
                activateActuators(true);
            } else {
                shutdown();
            }
      break;

      case(B010):
            shutdown();
      break;

      case(B001):

      break;

      }
    delay(1000);
  }

  int toMeasureDistance(int trigger, int echo)
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
