//Bibliotecas:------------------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
//------------------------------------------------------------------------------------------------------------------------------------------

//Pinos://----------------------------------------------------------------------------------------------------------------------------------
const uint led_verde = 11, led_azul = 12, led_vermelho = 13;
const uint botao_a = 5, botao_b = 6;
const uint buzzer_a = 10, buzzer_b = 21;
//------------------------------------------------------------------------------------------------------------------------------------------

//Protótipos das funções:-------------------------------------------------------------------------------------------------------------------
void pwm_setup(); //Configura o PWM nos LEDs e Buzzers
static void gpio_irq_handler(uint gpio, uint32_t events); //Função de callback para interrupção
void setup_botoes(); //Configura os botões
//------------------------------------------------------------------------------------------------------------------------------------------

//Variáveis globais://----------------------------------------------------------------------------------------------------------------------
uint slice_verde,slice_azul,slice_vermelho,slice_buzzer_a,slice_buzzer_b;
uint16_t WRAP = 19999;
const float divisor = 125.0;
static volatile uint32_t last_time = 0;
const uint16_t nivel_max_reservatorio = 10000, nivel_min_reservatorio = 0;//Simulando um reservatório de 10000 L
static int16_t incremento = 1000; //Incremento de 10%
//------------------------------------------------------------------------------------------------------------------------------------------

int main()
{   pwm_setup();
    setup_botoes();
    stdio_init_all();

    //Interrupções:-----------------------------------------------------------------------------------------------------------------------
    gpio_set_irq_enabled_with_callback(botao_a,GPIO_IRQ_EDGE_FALL,true,&gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botao_b,GPIO_IRQ_EDGE_FALL,true,&gpio_irq_handler);
    //------------------------------------------------------------------------------------------------------------------------------------



    while (true) {
        
    }
}
//---------------------------------------------------------------------------------------------------------------------------------------------------
//Campo das funções:---------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------
void pwm_setup(){
    //Ativando o PWM nos pinos:
    gpio_set_function(led_verde,GPIO_FUNC_PWM);
    gpio_set_function(led_azul,GPIO_FUNC_PWM);
    gpio_set_function(led_vermelho,GPIO_FUNC_PWM);
    gpio_set_function(buzzer_a,GPIO_FUNC_PWM);
    gpio_set_function(buzzer_b,GPIO_FUNC_PWM);
    //Obtendo o slice:
    slice_verde = pwm_gpio_to_slice_num(led_verde);
    slice_azul = pwm_gpio_to_slice_num(led_azul);
    slice_vermelho = pwm_gpio_to_slice_num(led_vermelho);
    slice_buzzer_a = pwm_gpio_to_slice_num(buzzer_a);
    slice_buzzer_b = pwm_gpio_to_slice_num(buzzer_b);
    //Divisor de Clock: 
    pwm_set_clkdiv(slice_verde,divisor);
    pwm_set_clkdiv(slice_azul,divisor);
    pwm_set_clkdiv(slice_vermelho,divisor);
    pwm_set_clkdiv(slice_buzzer_a,divisor);
    pwm_set_clkdiv(slice_buzzer_b,divisor);
    //Wrap:
    pwm_set_wrap(slice_verde,WRAP);
    pwm_set_wrap(slice_azul,WRAP);
    pwm_set_wrap(slice_vermelho,WRAP);
    pwm_set_wrap(slice_buzzer_a,WRAP);
    pwm_set_wrap(slice_buzzer_b,WRAP);
    //Ativando o PWM:
    pwm_set_enabled(slice_verde,true);
    pwm_set_enabled(slice_azul,true);
    pwm_set_enabled(slice_buzzer_a,true);
    pwm_set_enabled(slice_buzzer_b,true);
    //Configuração: Divisor 125.0, Wrap: 19999, Frequência do sistema : 125MHz
    //FPWM = 50Hz
    
}

void gpio_irq_handler(uint gpio, uint32_t events){

    uint32_t tempo_atual = to_us_since_boot(get_absolute_time()); //Pega o tempo do sistema desde o seu inicio em us

    if(tempo_atual-last_time>200000){ //Debounce, verifica se o tempo atual - o ultimo tempo foi maior que 200000us (200ms)
        last_time = tempo_atual; //Atualiza o ultimo tempo

        if(gpio_get(botao_a)==0){ //Rotina do botão A

        }
        if(gpio_get(botao_b)==0){ //Rotina do botão B

        }

    }


}
void setup_botoes(){
    //Inicializando as GPIOs:
    gpio_init(botao_a);
    gpio_init(botao_b);
    //Definindo como entrada:
    gpio_set_dir(botao_a,GPIO_IN);
    gpio_set_dir(botao_b,GPIO_IN);
    //Habilitando o Pull-up:
    gpio_pull_up(botao_a);
    gpio_pull_up(botao_b);


}