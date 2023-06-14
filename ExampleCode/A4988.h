#include <Arduino.h>
#include "config.h"

#ifndef _A4988_H_
#define _A4988_H_

class A4988_DRIVER{
    public:
      A4988_DRIVER(void);
      void rst_A4988(void);
      void disa_A4988(void);
      void ena_A4988(void);
      void HOR(void); 
      void AHR(void); 
      void PASSO(void);  
      void FREQUENCIA(void); 
      void FULL(void);
      void HALF(void);
      void P1_4(void);
      void P1_8(void);
      void P1_16(void);
      void TesteMotor(void);
      void Print_RPM (void);
    
    private:
      
      // Definiçoes das Portas Digitais do Arduino
      const int m_RST ;        // Porta digital D08 - reset do A4988
      int m_SLP ;              // Porta digital D09 - dormir (sleep) A4988
      int m_ENA ;              // Porta digital D07 - ativa (enable) A4988
      int m_MS1 ;              // Porta digital D04 - MS1 do A4988
      int m_MS2 ;              // Porta digital D05 - MS2 do A4988
      int m_MS3 ;              // Porta digital D06 - MS3 do A4988
      int m_DIR ;              // Porta digital D03 - direção (direction) do A4988
      int m_STP ;              // Porta digital D02 - passo(step) do A4988
       
      int MeioPeriodo = 1000;   // MeioPeriodo do pulso STEP em microsegundos F= 1/T = 1/2000 uS = 500 Hz
      float PPS = 0;            // Pulsos por segundo
      boolean sentido = true;   // Variavel de sentido
      long PPR = 200;           // Número de passos por volta
      long Pulsos;              // Pulsos para o driver do motor
      int Voltas = 3;           // voltas do motor
      float RPM;                // Rotacoes por minuto 
  };


#endif
