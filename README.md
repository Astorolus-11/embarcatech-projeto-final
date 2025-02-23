# Projeto simulando um reservatório de água de 10.000L


## 1. Propósito


O propósito desse projeto é fazer uma pequena simulação de um reservátorio de água com um sistema de bomba que pode tanto encher o reservatório quanto esvaziar. Além disso demonstrar domínio na linguagem C,
em manipulação de ADC, PWM, comunicação serial, tratamento de interrupções e bounce dos botões e temporização.


## 2. Hardware


Para o desenvolvimento desse projeto foi utilizado a placa BitDogLab fornecida pela Embarcatech, com o Raspberry Pi Pico W com o microcontrolador RP2040.
Os componentes utilizados presentes na placa foram:


1. Botões : A, B e JOY (GPIOs 5, 6 e 22 respectivamente).
2. LEDs do RGB : Verde e vermelho (GPIOs 11 e 13).
3. Buzzers : A, B (GPIOs 10 e 21).
4. Matriz de LEDs 5x5 WS2812 : (GPIO 7).
5. Joystick : Eixo x e y (GPIOs 27 e 26).
6. Comuinicação serial I2C : SDA e SCL (GPIOs 14 e 15).
7. Display SSD1306 : Endereço 0x3C.





## 3. Funcionalidades


1. Botão A - Aumenta a água do reservatório com um valor fixo de 1000L.
2. Botão B - Diminui a água do reservatório com um valor fixo de 1000L.
3. Botão Joy - Liga a bomba que enche o reservatório a uma taxa de 500L/s.
4. Joystick - No eixo X ao aproximar para esquerda diminui e para direita aumenta a taxa de enchimento. No eixo Y para cima liga e para baixo desliga a bomba.

   Ao interagir com o botão Joy e com o joystick a um efeito sonoro com os buzzer a e buzzer b
   para simular o acionamento da bomba. Cada interação com o reservatório refletirá seu estado
   na matriz de LEDs WS2812, no RGB com PWM e com o display SSD1306, mostrando seu status. 
   


   
   
## 4. Pré-requisitos


1. Ter o [Pico SDK](https://github.com/raspberrypi/pico-sdk) instalado na máquina;
2. Ter o [ARM GNU Toolchain](https://developer.arm.com/Tools%20and%20Software/GNU%20Toolchain) instalado;
3. Ter o [Visual Studio Code](https://code.visualstudio.com/download) instalado;
4. Ter este repositório clonado na sua máquina;
5. Ter as seguintes extensões instaladas no seu VS Code para o correto funcionamento:
- [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools);
- [CMake](https://marketplace.visualstudio.com/items?itemName=twxs.cmake);
- [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools);
- [Raspberry Pi Pico](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico);
  
  

##  5. Como executar o projeto:


1. Clonar o repositório: https://github.com/Astorolus-11/embarcatech-projeto-final
2. Abrir a pasta clonada através do import project:

   ![image](https://github.com/user-attachments/assets/9ea528e1-0253-4cf8-b6c6-8532be0fc1b4)
   

3. Para executar na placa clique em Run que está localizada no rodapé do vscode (A placa precisa já está conectada e com o BootSel ativado):

   ![image](https://github.com/user-attachments/assets/36b14dce-1309-4f0c-a7f3-3cd7edb2b336)


## 6. Vídeo de demonstração


   
  
  # Pronto! já está tudo pronto para testar o projeto!
