#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

QueueHandle_t qqueue;

#endif // QUEUE_H_INCLUDED

/**
     (mqtt.cpp.obj):/home/yudi/Documents/1-projetos/programming/unb/pi2-painter-robot-esp32/main/queue.h:7: multiple definition of `qqueue'; esp-idf/main/libmain.a(painter-robot-esp32.cpp.obj):/home/yudi/Documents/1-projetos/programming/unb/pi2-painter-robot-esp32/main/queue.h:7: first defined here
(StepMotor.cpp.obj):/home/yudi/Documents/1-projetos/programming/unb/pi2-painter-robot-esp32/main/queue.h:7: multiple definition of `qqueue'; esp-idf/main/libmain.a(painter-robot-esp32.cpp.obj):/home/yudi/Documents/1-projetos/programming/unb/pi2-painter-robot-esp32/main/queue.h:7: first defined here
*/