//Bibliotecas:
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

//Pinos:
const uint led_verde = 11, led_azul = 12, led_vermelho = 13;
const uint botao_a = 5, botao_b = 6;
const uint buzzer_a = 10, buzzer_b = 21;

//Protótipos:
void pwm_setup();


//Variáveis globais:
uint slice_verde,slice_azul,slice_vermelho,slice_buzzer_a,slice_buzzer_b;


int main()
{
    stdio_init_all();

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}

void pwm_setup(){
    //Ativando o PWM nos pinos:
    gpio_set_function(led_verde,GPIO_FUNC_PWM);
    gpio_set_function(led_azul,GPIO_FUNC_PWM);
    gpio_set_function(led_vermelho,GPIO_FUNC_PWM);
    //Obtendo o slice:
    
}