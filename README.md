# Robô Pintor ESP32

Certifique-se que você tem o 
[ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/#manual-installation) 
instalado em sua máquina.

## Extensões úteis para o VSCode

- [Espressif](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension)
- [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)

Em `.vscode/c_cpp_properties.json` adicione a seguinte configuração para
ajudar o Intellisense:

```json
{
    "configurations": [
        {
            // Essa configuração ajuda o Intellisense
            "compileCommands": "${workspaceFolder}/build/compile_commands.json",
            "intelliSenseMode": "linux-gcc-x64",
            "cStandard": "c17",
            "cppStandard": "c++17"
        }
    ],
    "version": 4
}
```

## Build, flash e etc

Certifique-se que o microcontrolador ESP32 está conectado ao seu computador
na porta USB. Então execute no terminal:

```
idf.py build flash monitor
```

O ponto de entrada da aplicação está em `main.cpp`.