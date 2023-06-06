//Bibliotecas
#include "A4988.h"

A4988_DRIVER::A4988_DRIVER(void)
{
      const int m_RST = RST;
      int m_SLP = SLP;              
      int m_ENA = ENA;              
      int m_MS1 = MS1;              
      int m_MS2 = MS2;              
      int m_MS3 = MS3;              
      int m_DIR = DIR;             
      int m_STP ; 
    
      DDRD = DDRD | B11111100;  // Configura Portas D02 até D07 como saída
      DDRB = 0x0F;              // Configura Portas D08,D09,D10 e D11 como saída
}
/* 
  Função rst_A4988
  Objetivo: Resetar o A4988 desativando todos os drivers de saída
  Descrição: O pino RST do A4988 é colocado em nível baixo, o arduino fica em delay por 10 milisegundos 
             e depois este pino é colocado em nível lógico alto novamente
*/

void A4988_DRIVER::rst_A4988(void)
{
  digitalWrite(m_RST, LOW);     // Realiza o reset do A4988
  delay (10);                 // Atraso de 10 milisegundos
  digitalWrite(m_RST, HIGH);    // Libera o reset do A4988
  delay (10);                 // Atraso de 10 milisegundos
}


/* 
  Função disa_A4988
  Objetivo: Desativar o A1988 
  Descrição: Força o pino ENABLE do A1988 para nível lógico alto 
             desativando os drivers do motor
*/

void A4988_DRIVER::disa_A4988(void)
{
  digitalWrite(m_ENA, HIGH);    // Desativa o chip A4988
  delay (10);                 // Atraso de 10 milisegundos
}


/*
     Função ena_A4988
    Objetivo: Ativar o A4988
    Descrição: Força o pino ENABLE do A1988 para nível lógico baixo 
 */

void A4988_DRIVER::ena_A4988(void)
{
  digitalWrite(m_ENA, LOW);     // Ativa o chip A4988
  delay (10);                 // Atraso de 10 milisegundos
}


 /*
     Função: HOR
    Objetivo: Configurar a rotação do Motor no sentido horário
    Descrição: Configura um sinal alto no pino DIR do A4988
 */

void A4988_DRIVER::HOR(void)                     
{
  Serial.println(" Sentido Horario ");
  digitalWrite(m_DIR, HIGH);      // Configura o sentido HORÁRIO
}


/*
     Função: AHR
    Objetivo: Configurar a rotação do Motor no sentido anti horário
    Descrição: Configura um sinal baixo no pino DIR do A4988
 */

void A4988_DRIVER::AHR(void)                      
{
  Serial.println(" Sentido anti-Horario ");
  digitalWrite(m_DIR, LOW);       
}


 /*
    Função: PASSO
    Objetivo: Um sinal de transição de baixo para alto avança um passo
    Descrição: Mudo o pino STP do A4988 de baixo para alto fazendo com que essa transição
               execute um passo.
 */

void A4988_DRIVER::PASSO(void)                         // Pulso do passo do Motor
{
  digitalWrite(m_STP, LOW);            // Pulso nível baixo
  delayMicroseconds (MeioPeriodo);   // MeioPeriodo de X microsegundos
  digitalWrite(m_STP, HIGH);           // Pulso nível alto
  delayMicroseconds (MeioPeriodo);   // MeioPeriodo de X microsegundos
}



  /*
    Função: FREQUENCIA
    Objetivo: Calcular Pulsos, PPS(passos por segundo) e RPM (rotação por minuto)
    Descrição:Calcula o numero de pulsos multiplicando o numero de passos por volta (PPR) por volta do motor
              Calcula a frequencia de pulsos por segundo f=1/T *  10^6
              Calcula a quantidade de rotação por minuto  multiplicando passos por volta por 60
 */
void A4988_DRIVER::FREQUENCIA(void)                    
{
  Pulsos = PPR * Voltas;             // Quantidade total de Pulsos (PPR = pulsos por volta)
  PPS = 1000000 / (2 * MeioPeriodo); // Frequencia Pulsos por segundo
  RPM = (PPS * 60) / PPR;            // Calculo do RPM
}


  /*
     Função: FULL
    Objetivo: Executa uma volta completa
    Descrição: Cada passo corresponde a 1,8°requerendo 200 passos para uma volta completa(360°).
               Os pinos MS1,MS2 e MS3 do A4988 em nível baixo configuram o modo full step.
 */
void A4988_DRIVER::FULL(void)
{
  Serial.println(" Passo Completo  PPR = 200 ");
  PPR = 200;                 // PPR = pulsos por volta
  digitalWrite(m_MS1, LOW);    // Configura modo Passo completo (Full step)
  digitalWrite(m_MS2, LOW);
  digitalWrite(m_MS3, LOW);
}


 /*
     Função: HALF
    Objetivo: Executa meia volta
    Descrição: Cada passo corresponde a 1,8°requerendo 400 passos para uma meia volta (180°).
               Os pinos MS1 = 1,MS2 = 0 e MS3 = 0 do A4988 configuram o modo half step.
 */
void A4988_DRIVER::HALF(void)
{
  Serial.println(" Meio Passo  PPR = 400 ");
  PPR = 400;                  // PPR = pulsos por volta
  digitalWrite(m_MS1, HIGH);    // Configura modo Meio Passo (Half step)
  digitalWrite(m_MS2, LOW);
  digitalWrite(m_MS3, LOW);
}


 /*
     Função: P1_4
    Objetivo: Executar um quarto de volta.
    Descrição: Cada passo corresponde a 1,8°requerendo 800 passos para um quarto de volta (90°).
               Os pinos MS1 = 0,MS2 = 1 e MS3 = 0 do A4988 configuram o modo 1/4 step.
 */
void A4988_DRIVER::P1_4(void)
{
  Serial.println(" Micro-passo 1/4  PPR = 800 ");
  PPR = 800;                 // PPR = pulsos por volta
  digitalWrite(m_MS1, LOW);    // Configura modo Micro Passo 1/4
  digitalWrite(m_MS2, HIGH);
  digitalWrite(m_MS3, LOW);
}
  /*
    Função: P1_8
    Objetivo: Executar um oitavo de volta.
    Descrição: Cada passo corresponde a 1,8°requerendo 1600 passos para um oitavo de volta (45°).
               Os pinos MS1 = 1,MS2 = 1 e MS3 = 0 do A4988 configuram o modo 1/8 step.
 */
void A4988_DRIVER::P1_8(void)
{
  Serial.println(" Micro-passo 1/8  PPR = 1600 ");
  PPR = 1600;                 // PPR = pulsos por volta
  digitalWrite(m_MS1, HIGH);    // Configura modo Micro Passo 1/8
  digitalWrite(m_MS2, HIGH);
  digitalWrite(m_MS3, LOW);
}
  /*
    Função: P1_16
    Objetivo: Executar um dezesseis avos de volta.
    Descrição: Cada passo corresponde a 1,8°requerendo 3200 passos para um dezesseis avos (22,5°).
               Os pinos MS1 = 1,MS2 = 1 e MS3 = 1 do A4988 configuram o modo 1/16 step.
 */
void A4988_DRIVER::P1_16(void)
{
  Serial.println(" Micro-passo 1/16  PPR = 3200 ");
  PPR = 3200;                 // PPR = pulsos por volta
  digitalWrite(m_MS1, HIGH);    // Configura modo Micro Passo 1/16
  digitalWrite(m_MS2, HIGH);
  digitalWrite(m_MS3, HIGH);
}

  /*
     Função: Print_RPM
    Objetivo: Mostrar no serial print o numero de voltas, pulsos por segundo,rotação por minuto  
    Descrição: 
 */
void A4988_DRIVER::Print_RPM (void)
{
  FREQUENCIA();                           // calcula Pulsos, PPS e RPM
  Serial.print(" Voltas= ");
  Serial.print(Voltas);
  Serial.print(" Pulsos= ");
  Serial.print(Pulsos);
  Serial.print(" PPS= ");
  Serial.print(PPS, 2);
  Serial.print(" RPM= ");
  Serial.println(RPM, 2);
}



  /*
     Função: TesteMotor
    Objetivo: Teste de funcionamento do motor girando no sentido horario e anti horário,
              desativando e ativando o chip A4988, executando um passo.
    Descrição: Usa as funções acima para testar o funcionamento do motor.
 */
void A4988_DRIVER::TesteMotor(void)
{
  Print_RPM ();                           // Print Voltas, PPS e  RPM
 
  HOR();                                  // Gira sentido Horario
  for (int i = 0; i <= Pulsos; i++)       // Incrementa o Contador
  {
    PASSO();                              // Avança um passo no Motor
  }
  disa_A4988();                           // Desativa o chip A4988
  delay (1000) ;                          // Atraso de 1 segundo
  ena_A4988();                            // Ativa o chip A4988
 
  AHR();                                  // Gira sentido anti-Horario
  for (int i = 0; i <= Pulsos; i++)       // Incrementa o Contador
  {
    PASSO();                              // Avança um passo no Motor
  }
  disa_A4988();                           // Desativa o chip A4988
  delay (1000) ;                          // Atraso de 1 segundo
  ena_A4988();                            // Ativa o chip A4988
}


 
