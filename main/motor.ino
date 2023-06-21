// colocar aviso sonoro no alarme (buzzer?)
// verificar se o alarme é dispardo borda subida,descida...
// interrupçao na esp dif arduino
// checar niveis logicos e sentido de rotação

#define dirPin    33
#define stepPin   31
#define enablePin 34
#define alarmPin  32
#define pendPin   30 // indicativo de fim de ciclo do encoder
#define ledPin    35 // led vermelho indicador do alarme

boolean dir                  = false;
int t_high                   = 2000;
int vel                      = 50;
const int stepsPerRevolution = 400; // muda de acordo com o chaveamento

void setup() {
    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    pinMode(enablePin, OUTPUT);

    digitalWrite(dirPin, HIGH);
    digitalWrite(enablePin, LOW);
    digitalWrite(stepPin, LOW);

    attachInterrupt(digitalPinToInterrupt(alarmPin), alarme, CHANGE); // conf de interrupção do alarme. temos que ver como é feito na esp (pino, rotina...)
}

void loop() {
    for (int i = 0; i < stepsPerRevolution; i++) {
        passo();
    }

} // end loop

//======================================================================================================

// rotação antihorario
void counter_clock() {
    digitalWrite(dirPin, LOW);
}

// rotação horario
void clockwise() {
    digitalWrite(dirPin, HIGH);
}

// habilita o motor, forçando enable = 00
void enabled() {
    digitalWrite(enablePin, LOW);
    delay(100);
}

// desabilita o motor, forçando enable = 11
void disabled() {
    digitalWrite(enablePin, HIGH);
    delay(100);
}

// acelera
void speedUp(int vel) {
    vel = vel + 100;
}

// desacelera (usar a mesma função de speedUp , esta para fins de teste)
void speedDown(int vel) {
    vel = vel - 100;
}

// rotina que será chamada na interupção do alarme
// indicativo de alarme : Se o Led vermelho piscar uma vez em um intervalo de 3s, indica sobrecorrente ou curto-circuito entre as fases do motor.
//                       Se o Led vermelho piscar duas vezes em um intervalo de 3s, indica sobretensão.
//                       Se o Led vermelho piscar três vezes em um intervalo de 3s, indica diferença de posição entre motor e encoder ou cabo do encoder desconectado.
void alarme() {
    digitalWrite(ledPin, HIGH);
}

// indicativo fim de ciclo aberto indica que o ciclo não terminou
void pend() {
}

// executa um passo (depende de qual será o mais ajustado)
void passo() {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(t_high);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(t_high);
}

// Calcular Pulsos, PPS(passos por segundo) e RPM (rotação por minuto)
// Calcula o numero de pulsos multiplicando o numero de passos por volta (PPR) por volta do motor
// Calcula a frequencia de pulsos por segundo f=1/T *  10^6
// Calcula a quantidade de rotação por minuto  multiplicando passos por volta por 60
/*
void frequencia()
{
  Pulsos = PPR * Voltas;             // Quantidade total de Pulsos (PPR = pulsos por volta)
  PPS = 1000000 / (2 * MeioPeriodo); // Frequencia Pulsos por segundo
  RPM = (PPS * 60) / PPR;            // Calculo do RPM
}
*/