//Bibliotecas
#include "A4988.h"
#include "config.h"

//Instâncias e objetos
A4988_DRIVER MOTOR;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  Serial.println();
  MOTOR.FULL();          // Selecione aqui o modo de passo
  MOTOR.TesteMotor();
  MOTOR.Print_RPM();
}
