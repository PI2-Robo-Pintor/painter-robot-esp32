#include "StepMotor.h"

StepMotor::StepMotor(void) {
    // FIXME: Tem que revisar esses pinos!!!
    m_RST = GPIO_NUM_1;
    m_SLP = GPIO_NUM_2;
    m_ENA = GPIO_NUM_3;
    m_MS1 = GPIO_NUM_4;
    m_MS2 = GPIO_NUM_5;
    m_MS3 = GPIO_NUM_6;
    // FIXME: Esses pinos não existem?!?!
    m_DIR = GPIO_NUM_7;
    m_STP = GPIO_NUM_8;

    gpio_set_direction(m_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(m_SLP, GPIO_MODE_OUTPUT);
    gpio_set_direction(m_ENA, GPIO_MODE_OUTPUT);
    gpio_set_direction(m_MS1, GPIO_MODE_OUTPUT);
    gpio_set_direction(m_MS2, GPIO_MODE_OUTPUT);
    gpio_set_direction(m_MS3, GPIO_MODE_OUTPUT);
    // gpio_set_direction(m_DIR, GPIO_MODE_OUTPUT);
    // gpio_set_direction(m_STP, GPIO_MODE_OUTPUT);

    meioPeriodo = 1000;
    PPS         = 0;
    sentido     = true;
    PPR         = 200;
    voltas      = 3;
    oi          = 131;
}

/*
  Função rst_A4988
  Objetivo: Resetar o A4988 desativando todos os drivers de saída
  Descrição: O pino RST do A4988 é colocado em nível baixo, o arduino fica em delay por 10 milisegundos
             e depois este pino é colocado em nível lógico alto novamente
*/
void StepMotor::reset(void) {
    gpio_set_level(m_RST, LOW);          // Realiza o reset do A4988
    vTaskDelay(10 / portTICK_PERIOD_MS); // Atraso de 10 milisegundos
    gpio_set_level(m_RST, HIGH);         // Libera o reset do A4988
    vTaskDelay(10 / portTICK_PERIOD_MS); // Atraso de 10 milisegundos
}

/*
  Função disa_A4988
  Objetivo: Desativar o A1988
  Descrição: Força o pino ENABLE do A1988 para nível lógico alto
             desativando os drivers do motor
*/
void StepMotor::disable(void) {
    gpio_set_level(m_ENA, HIGH);         // Desativa o chip A4988
    vTaskDelay(10 / portTICK_PERIOD_MS); // Atraso de 10 milisegundos
}

/*
     Função ena_A4988
    Objetivo: Ativar o A4988
    Descrição: Força o pino ENABLE do A1988 para nível lógico baixo
 */
void StepMotor::enable(void) {
    gpio_set_level(m_ENA, LOW);          // Ativa o chip A4988
    vTaskDelay(10 / portTICK_PERIOD_MS); // Atraso de 10 milisegundos
}

/*
    Função: HOR
   Objetivo: Configurar a rotação do Motor no sentido horário
   Descrição: Configura um sinal alto no pino DIR do A4988
*/
void StepMotor::clockwise(void) {
    ESP_LOGD(TAG, " Sentido Horario ");
    gpio_set_level(m_DIR, HIGH); // Configura o sentido HORÁRIO
}

/*
     Função: AHR
    Objetivo: Configurar a rotação do Motor no sentido anti horário
    Descrição: Configura um sinal baixo no pino DIR do A4988
 */
void StepMotor::counterClockwise(void) {
    ESP_LOGD(TAG, " Sentido anti-Horario ");
    gpio_set_level(m_DIR, LOW);
}

/*
   Função: PASSO
   Objetivo: Um sinal de transição de baixo para alto avança um passo
   Descrição: Mudo o pino STP do A4988 de baixo para alto fazendo com que essa transição
              execute um passo.
*/
void StepMotor::step(void) {
    // Pulso do passo do Motor
    gpio_set_level(m_STP, LOW); // Pulso nível baixo
    // FIXME: Não tenho certeza se essa divisão tá correta
    vTaskDelay(meioPeriodo / 1000 / portTICK_PERIOD_MS); // MeioPeriodo de X microsegundos
    gpio_set_level(m_STP, HIGH);                         // Pulso nível alto
    vTaskDelay(meioPeriodo / 1000 / portTICK_PERIOD_MS); // MeioPeriodo de X microsegundos
}

/*
  Função: FREQUENCIA
  Objetivo: Calcular Pulsos, PPS(passos por segundo) e RPM (rotação por minuto)
  Descrição:Calcula o numero de pulsos multiplicando o numero de passos por volta (PPR) por volta do motor
            Calcula a frequencia de pulsos por segundo f=1/T *  10^6
            Calcula a quantidade de rotação por minuto  multiplicando passos por volta por 60
*/
void StepMotor::frequency(void) {
    pulsos = PPR * voltas;                // Quantidade total de Pulsos (PPR = pulsos por volta)
    PPS    = 1000000 / (2 * meioPeriodo); // Frequencia Pulsos por segundo
    RPM    = (PPS * 60) / PPR;            // Calculo do RPM
}

/*
   Função: FULL
  Objetivo: Executa uma volta completa
  Descrição: Cada passo corresponde a 1,8°requerendo 200 passos para uma volta completa(360°).
             Os pinos MS1,MS2 e MS3 do A4988 em nível baixo configuram o modo full step.
*/
void StepMotor::fullStep(void) {
    ESP_LOGD(TAG, " Passo Completo  PPR = 200 ");
    PPR = 200;                  // PPR = pulsos por volta
    gpio_set_level(m_MS1, LOW); // Configura modo Passo completo (Full step)
    gpio_set_level(m_MS2, LOW);
    gpio_set_level(m_MS3, LOW);
}

/*
    Função: HALF
   Objetivo: Executa meia volta
   Descrição: Cada passo corresponde a 1,8°requerendo 400 passos para uma meia volta (180°).
              Os pinos MS1 = 1,MS2 = 0 e MS3 = 0 do A4988 configuram o modo half step.
*/
void StepMotor::halfStep(void) {
    ESP_LOGD(TAG, " Meio Passo  PPR = 400 ");
    PPR = 400;                   // PPR = pulsos por volta
    gpio_set_level(m_MS1, HIGH); // Configura modo Meio Passo (Half step)
    gpio_set_level(m_MS2, LOW);
    gpio_set_level(m_MS3, LOW);
}

/*
    Função: P1_4
   Objetivo: Executar um quarto de volta.
   Descrição: Cada passo corresponde a 1,8°requerendo 800 passos para um quarto de volta (90°).
              Os pinos MS1 = 0,MS2 = 1 e MS3 = 0 do A4988 configuram o modo 1/4 step.
*/
void StepMotor::microStep4(void) {
    ESP_LOGD(TAG, " Micro-passo 1/4  PPR = 800 ");
    PPR = 800;                  // PPR = pulsos por volta
    gpio_set_level(m_MS1, LOW); // Configura modo Micro Passo 1/4
    gpio_set_level(m_MS2, HIGH);
    gpio_set_level(m_MS3, LOW);
}

/*
  Função: P1_8
  Objetivo: Executar um oitavo de volta.
  Descrição: Cada passo corresponde a 1,8°requerendo 1600 passos para um oitavo de volta (45°).
             Os pinos MS1 = 1,MS2 = 1 e MS3 = 0 do A4988 configuram o modo 1/8 step.
*/
void StepMotor::microStep8(void) {
    ESP_LOGD(TAG, " Micro-passo 1/8  PPR = 1600 ");
    PPR = 1600;                  // PPR = pulsos por volta
    gpio_set_level(m_MS1, HIGH); // Configura modo Micro Passo 1/8
    gpio_set_level(m_MS2, HIGH);
    gpio_set_level(m_MS3, LOW);
}

/*
  Função: P1_16
  Objetivo: Executar um dezesseis avos de volta.
  Descrição: Cada passo corresponde a 1,8°requerendo 3200 passos para um dezesseis avos (22,5°).
             Os pinos MS1 = 1,MS2 = 1 e MS3 = 1 do A4988 configuram o modo 1/16 step.
*/
void StepMotor::microStep16(void) {
    ESP_LOGD(TAG, " Micro-passo 1/16  PPR = 3200 ");
    PPR = 3200;                  // PPR = pulsos por volta
    gpio_set_level(m_MS1, HIGH); // Configura modo Micro Passo 1/16
    gpio_set_level(m_MS2, HIGH);
    gpio_set_level(m_MS3, HIGH);
}

/*
   Função: Print_RPM
  Objetivo: Mostrar no serial print o numero de voltas, pulsos por segundo,rotação por minuto
  Descrição:
*/
void StepMotor::printRpm(void) {
    frequency(); // calcula Pulsos, PPS e RPM
    ESP_LOGD(TAG,
             "Voltas=%d, pulsos=%ld, PPS=%.2f, RPM=%.2f",
             voltas, pulsos, PPS, RPM);
}

/*
   Função: TesteMotor
  Objetivo: Teste de funcionamento do motor girando no sentido horario e anti horário,
            desativando e ativando o chip A4988, executando um passo.
  Descrição: Usa as funções acima para testar o funcionamento do motor.
*/
void StepMotor::testMotor(void) {
    printRpm(); // Print Voltas, PPS e  RPM

    clockwise();                      // Gira sentido Horario
    for (int i = 0; i <= pulsos; i++) // Incrementa o Contador
    {
        step(); // Avança um passo no Motor
    }
    disable();                             // Desativa o chip A4988
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Atraso de 1 segundo
    enable();                              // Ativa o chip A4988

    counterClockwise();               // Gira sentido anti-Horario
    for (int i = 0; i <= pulsos; i++) // Incrementa o Contador
    {
        step(); // Avança um passo no Motor
    }
    disable();                             // Desativa o chip A4988
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Atraso de 1 segundo
    enable();                              // Ativa o chip A4988
}

void step_motor_control_loop(void* args) {
    StepMotor* motor = (StepMotor*)args;
    // FIXME: motor.oi é alterado dentro do loop por algum motivo!!!
    ESP_LOGE(TAG, "motor.oi é alterado dentro do loop por algum motivo!!!");
    ESP_LOGI(TAG, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>> motor %p %d %p",
             motor, motor->oi, motor->queue);

    char command = 0;
    while (true) {
        // ESP_LOGI(TAG, "verifica se tem mensagem em Queue...");
        // if (xQueueReceive(motor->queue, &command, (TickType_t)10) == pdPASS) {
        if (xQueueReceive(qqueue, &command, (TickType_t)10) == pdPASS) {
            ESP_LOGI(TAG, "Mensagem recebdia do Mqtt! comando [%c]", command);

            if (command == 'a') {
                gpio_set_level(GPIO_NUM_2, HIGH);
            } else {
                gpio_set_level(GPIO_NUM_2, LOW);
            }
        }
        ESP_LOGI(TAG, "step motor control iteration | motor %p %d", motor, motor->oi);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}