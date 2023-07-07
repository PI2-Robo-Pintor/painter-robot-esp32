# Notas para o sucessor

## Relés

Verifique o acionamento dos relés em `main.cpp`.

```cpp
Relay rel(...)
Relay relay_valve(...)
```

Se não me engano, o relé da válvula liga com `LOW`. De qualquer maneira, um dos 
relés tem essa peculiaridade.

## Use a extensão Git History
Use a extensão 
[Git History](https://marketplace.visualstudio.com/items?itemName=donjayamanne.githistory) pra ver os diffs dos últimos commits.

## Deslocamento do motor

O deslocamento do motor é feito em termos de **posição final**. Não é em termos
"deslocamento até lá" ou "quanto falta pra chegar lá". 

Toda vez que a posição final mudar, use `motor.set_target_position(new_position)`.

## `find_initial_position` 

Essa é a função do "find home". Ele desce até encontrar o sensor de fim de 
curso.

## Bug do não descer dps de chegar ao "upper limit"

Minha intuição diz que é algo bem besta. 

### Sugestão 1: aumentar o delay 
aumentar o delay `.set_target_position(...)` e `.start()`.

```cpp
motor.set_target_position(lower_target_position);
// vTaskDelay(1);
vTaskDelay(5); // tentar um desses
vTaskDelay(10); // ou esse. Ou até mais. Lembre que assim o delay é em TICKS, não milisegundos
motor.set_direction(D_DOWN);
motor.start();
```

### Sugestão 2: chamar `find_initial_position` de novo

Chamar `find_initial_position` quando chegar ao topo. Isso vai fazer o motor 
descer e de certa maneira, resetar. Assim, ele sempre pinta subindo e desce
tentando encontrar a posição inicial.

---

Que os deuses ajudem todos nós.
