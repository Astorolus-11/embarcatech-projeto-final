//Bibliotecas:------------------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include <stdlib.h>
#include <math.h>
#include "matriz_leds.pio.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "fonte.h"
//------------------------------------------------------------------------------------------------------------------------------------------

//Pinos://----------------------------------------------------------------------------------------------------------------------------------
const uint led_verde = 11, led_vermelho = 13; //LEDs do RGB
const uint botao_a = 5, botao_b = 6, botao_joy = 22; //Botões
const uint buzzer_a = 10, buzzer_b = 21; //Buzzers
const uint joy_x = 27, joy_y = 26; //Joystick
const uint pin_matriz = 7; //Matriz de LEDs 5x5
//------------------------------------------------------------------------------------------------------------------------------------------

//Protótipos das funções:-------------------------------------------------------------------------------------------------------------------
void pwm_setup(); //Configura o PWM nos LEDs e Buzzers
static void gpio_irq_handler(uint gpio, uint32_t events); //Função de callback para interrupção
void setup_botoes(); //Configura os botões
void bomba(); //Bomba d'água para encher o reservatório automaticamente
void adc_setup(); //Configura o joystick
void atualizar_reservatorio(uint16_t x, uint16_t y); //Atualiza o reservatório com o joystick, Eixo x: Vazão d'água. Eixo y: Direção do motor (Se vai encher ou esvaziar)
void estado_reservatorio_led_rgb(); //Mostra o estado do reservatório nos LEDs RGB
void estado_reservatorio_matriz(); //Mostra o estado do reservatório na matriz de LEDs 5x5
uint32_t intensidade(double g, double r, double b); //Configura a intensidade do brilho dos LEDs da matriz
//------------------------------------------------------------------------------------------------------------------------------------------

//Variáveis globais://----------------------------------------------------------------------------------------------------------------------
uint slice_verde,slice_vermelho,slice_buzzer_a,slice_buzzer_b;
uint16_t WRAP = 9999;
const float divisor = 125.0;
static volatile uint32_t last_time = 0;
const uint16_t nivel_max_reservatorio = 10000, nivel_min_reservatorio = 0, nivel_max_incremento = 600;//Simulando um reservatório de 10000 L
static int16_t reservatorio = 0;
static int16_t incremento = 1000, incremento_2 = 500; 
static bool estado_motor = false;
const uint pixels = 25;
//------------------------------------------------------------------------------------------------------------------------------------------

//PIO:--------------------------------------------------------------------------------------------------------------------------------------
static PIO pio;
static uint sm;
static uint offset;
static uint32_t valor_led;
//------------------------------------------------------------------------------------------------------------------------------------------

double estados_reservatorio [5][25] = {

       {0.1, 0.1, 0.1, 0.1, 0.1, //0% 
        0.1, 0.1, 0.1, 0.1, 0.1,
        0.1, 0.1, 0.1, 0.1, 0.1,
        0.1, 0.1, 0.1, 0.1, 0.1,
        0.1, 0.1, 0.1, 0.1, 0.1},
        
        {0.3, 0.1, 0.1, 0.1, 0.3, //25% 0.3 branco, 0.1 vermelho, 0.2 verde
         0.3, 0.1, 0.1, 0.1, 0.3,
         0.3, 0.1, 0.1, 0.1, 0.3,
         0.3, 0.2, 0.2, 0.2, 0.3,
         0.3, 0.3, 0.3, 0.3, 0.3},
        
        {0.3, 0.1, 0.1, 0.1, 0.3, //50% 0.3 branco, 0.1 vermelho, 0.2 verde
         0.3, 0.1, 0.1, 0.1, 0.3,
         0.3, 0.2, 0.2, 0.2, 0.3,
         0.3, 0.2, 0.2, 0.2, 0.3,
         0.3, 0.3, 0.3, 0.3, 0.3},
        
        {0.3, 0.1, 0.1, 0.1, 0.3, //75% 0.3 branco, 0.1 vermelho, 0.2 verde
         0.3, 0.2, 0.2, 0.2, 0.3,
         0.3, 0.2, 0.2, 0.2, 0.3,
         0.3, 0.2, 0.2, 0.2, 0.3,
         0.3, 0.3, 0.3, 0.3, 0.3},
        
        {0.3, 0.2, 0.2, 0.2, 0.3, //100% 0.3 branco, 0.2 verde
         0.3, 0.2, 0.2, 0.2, 0.3,
         0.3, 0.2, 0.2, 0.2, 0.3,
         0.3, 0.2, 0.2, 0.2, 0.3,
         0.3, 0.3, 0.3, 0.3, 0.3}


};

int main()
{   pwm_setup();
    setup_botoes();
    adc_setup();
    stdio_init_all();
    //Configuração da PIO:----------------------------------------------------------------------------------------------------------------
    pio = pio0; 
    offset = pio_add_program(pio,&matriz_leds_program); //Adiciona o programa ao PIO
    sm = pio_claim_unused_sm(pio,true); //Utiliza uma maquina de estado disponível
    matriz_leds_program_init(pio, sm, offset, pin_matriz);
    //------------------------------------------------------------------------------------------------------------------------------------
    uint64_t tempo_atual_main,last_time_main=0;

    //Interrupções:-----------------------------------------------------------------------------------------------------------------------
    gpio_set_irq_enabled_with_callback(botao_a,GPIO_IRQ_EDGE_FALL,true,&gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botao_b,GPIO_IRQ_EDGE_FALL,true,&gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botao_joy,GPIO_IRQ_EDGE_FALL,true,&gpio_irq_handler);
    //------------------------------------------------------------------------------------------------------------------------------------



    while (true) {

        if(estado_motor){//Rotina se a bomba estiver ligada

            tempo_atual_main=to_ms_since_boot(get_absolute_time()); //Tempo atual do sistema convertido para ms desde seu inicio

            if(tempo_atual_main-last_time_main>1000){//A cada 1s se o motor estiver ligado, vai enchendo o reservatório a uma taxa
            //de 500L/s
            bomba();
            pwm_set_gpio_level(buzzer_a,20);
            pwm_set_gpio_level(buzzer_b,20);

            
            last_time_main=tempo_atual_main;//Atualiza o last_time_main
            
            }

        }
        else{

            pwm_set_gpio_level(buzzer_a,0);
            pwm_set_gpio_level(buzzer_b,0);

            
            //Leitura dos valores do joystick:
            tempo_atual_main=to_ms_since_boot(get_absolute_time());

            if(tempo_atual_main-last_time_main>1000){ //Enche o reservatório com o joystick a cada 1s
                last_time_main=tempo_atual_main;
                adc_select_input(1); //Canal 0 para o pino 27, eixo x
                uint16_t valor_x = adc_read();
                adc_select_input(0); //Canal 1 para o pino 26, eixo y
                uint16_t valor_y = adc_read();

                atualizar_reservatorio(valor_x,valor_y);
                
            
            }
            

        }
        printf("Reservatório = %d\n",reservatorio);
        estado_reservatorio_led_rgb();
        estado_reservatorio_matriz();
        

        sleep_ms(100);
        
    }
}


//---------------------------------------------------------------------------------------------------------------------------------------------------
//Campo das funções:---------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------
void pwm_setup(){
    //Ativando o PWM nos pinos:
    gpio_set_function(led_verde,GPIO_FUNC_PWM);
    gpio_set_function(led_vermelho,GPIO_FUNC_PWM);
    gpio_set_function(buzzer_a,GPIO_FUNC_PWM);
    gpio_set_function(buzzer_b,GPIO_FUNC_PWM);
    //Obtendo o slice:
    slice_verde = pwm_gpio_to_slice_num(led_verde);
    slice_vermelho = pwm_gpio_to_slice_num(led_vermelho);
    slice_buzzer_a = pwm_gpio_to_slice_num(buzzer_a);
    slice_buzzer_b = pwm_gpio_to_slice_num(buzzer_b);
    //Divisor de Clock: 
    pwm_set_clkdiv(slice_verde,divisor);
    pwm_set_clkdiv(slice_vermelho,divisor);
    pwm_set_clkdiv(slice_buzzer_a,divisor);
    pwm_set_clkdiv(slice_buzzer_b,divisor);
    //Wrap:
    pwm_set_wrap(slice_verde,WRAP);
    pwm_set_wrap(slice_vermelho,WRAP);
    pwm_set_wrap(slice_buzzer_a,WRAP);
    pwm_set_wrap(slice_buzzer_b,WRAP);
    //Ativando o PWM:
    pwm_set_enabled(slice_verde,true);
    pwm_set_enabled(slice_vermelho,true);
    pwm_set_enabled(slice_buzzer_a,true);
    pwm_set_enabled(slice_buzzer_b,true);
    //Configuração: Divisor 125.0, Wrap: 9999, Frequência do sistema : 125MHz
    //FPWM = 100Hz
    
}

void gpio_irq_handler(uint gpio, uint32_t events){

    uint32_t tempo_atual = to_us_since_boot(get_absolute_time()); //Pega o tempo do sistema desde o seu inicio em us

    if(tempo_atual-last_time>200000){ //Debounce, verifica se o tempo atual - o ultimo tempo foi maior que 200000us (200ms)
        last_time = tempo_atual; //Atualiza o ultimo tempo

        if(gpio_get(botao_a)==0 && estado_motor == false){ //Rotina do botão A

            if(reservatorio<nivel_max_reservatorio){
                reservatorio+=incremento;

            }
            else{
                reservatorio=nivel_max_reservatorio;
            }


        }

        if(gpio_get(botao_b)==0 && estado_motor==false){ //Rotina do botão B
            
            if(reservatorio>nivel_min_reservatorio){
                reservatorio-=incremento;
                
            }
            else{
                reservatorio=nivel_min_reservatorio;
            }

        }
        if(gpio_get(botao_joy)==0){
            estado_motor=!estado_motor; //Inverte o estado do motor (Ligado/Desligado)
            
        }

    }


}

void setup_botoes(){
    //Inicializando as GPIOs:
    gpio_init(botao_a);
    gpio_init(botao_b);
    gpio_init(botao_joy);
    //Definindo como entrada:
    gpio_set_dir(botao_a,GPIO_IN);
    gpio_set_dir(botao_b,GPIO_IN);
    gpio_set_dir(botao_joy,GPIO_IN);
    //Habilitando o Pull-up:
    gpio_pull_up(botao_a);
    gpio_pull_up(botao_b);
    gpio_pull_up(botao_joy);


}

void bomba(){

    if(estado_motor==1){

        if(reservatorio+incremento_2<=nivel_max_reservatorio){//Atualiza o reservatório enquanto não estiver no nível máximo
            reservatorio+=incremento_2;

        }
        else{
            estado_motor=false; //Desliga o motor se alcançou o nível máximo
            
        }
    
    }
    
}
void adc_setup(){

    //Inicializando o Conversor Analógico Digital:
    adc_init(); 
    //Inicializando os pinos do Joystick como ADC:
    adc_gpio_init(joy_x);
    adc_gpio_init(joy_y);
    //Inicializando os pinos:
    gpio_init(joy_x);
    gpio_init(joy_y);
    //Definindo como entrada:
    gpio_set_dir(joy_x,GPIO_IN);
    gpio_set_dir(joy_y,GPIO_IN);
    //Habilitando o Pull-up:
    gpio_pull_up(joy_x);
    gpio_pull_up(joy_y);
}

void atualizar_reservatorio(uint16_t x, uint16_t y){

    int16_t preencher_reservatorio = (x*nivel_max_incremento)/4095;
    

    if(y>2500){//Se for pra cima enche o reservatório
        if(reservatorio==nivel_max_reservatorio){
            printf("Reservatório cheio.\n");
        }
        else{
            reservatorio+=preencher_reservatorio;
            if(reservatorio>=nivel_max_reservatorio){//Se ultrapassar o valor máximo
                reservatorio=nivel_max_reservatorio;
            }
        }

    }

    else if(y<1970){//Se for para baixo esvazia o reservatório
        if(reservatorio==nivel_min_reservatorio){
            printf("Reservatório vazio.\n");
        }
        else{
            reservatorio-=preencher_reservatorio;
            

            if(reservatorio<=nivel_min_reservatorio){//Se ultrapassar do valor mínimo
                reservatorio=nivel_min_reservatorio;
            }
           
        }

    }

}

void estado_reservatorio_led_rgb(){

    int red_pwm = 0;
    int green_pwm = 0;

    // 0% vermelho
    if (reservatorio <= nivel_max_reservatorio / 3) {
        red_pwm = 9999;  // Vermelho totalmente aceso
        green_pwm = 0;   // Verde apagado
       
    }
    // Entre 33% a 66% fica amarelo e vai esverdeando
    else if (reservatorio <= (nivel_max_reservatorio * 2) / 3) {
        red_pwm = 9999 - ((reservatorio - nivel_max_reservatorio / 3) * 9999) / (nivel_max_reservatorio / 3); 
        green_pwm = (reservatorio - nivel_max_reservatorio / 3) * 9999 / (nivel_max_reservatorio / 3);  
        
    }
    // 100% verde
    else {
        red_pwm = 0;     
        green_pwm = 9999; 
    }

    pwm_set_gpio_level(led_vermelho, red_pwm);
    pwm_set_gpio_level(led_verde, green_pwm);

    }

void estado_reservatorio_matriz(){

    uint32_t valor_led;

    if(reservatorio==nivel_min_reservatorio){ //0%

        for(uint i=0; i<pixels; i++){

            if(i>=0 && i<5 || i==5 || i==9 || i==10 || i==14 || i==15 || i==19 || i==20 || i==24){ //Cor branca do reservatório
                valor_led = intensidade(estados_reservatorio[0][i],estados_reservatorio[0][i],estados_reservatorio[0][i]);
                
            }
            else{
                valor_led = intensidade(0.0,estados_reservatorio[0][i],0.0);//Vermelho

            }
            pio_sm_put_blocking(pio,sm,valor_led);
            
        }


    }

    else if(reservatorio>=nivel_max_reservatorio/4 && reservatorio<4900){// Entre 25% e 49%

        for(uint i=0; i<pixels; i++){

            if(i>=0 && i<5 || i==5 || i==9 || i==10 || i==14 || i==15 || i==19 || i==20 || i==24){ //Cor branca do reservatório
                valor_led = intensidade(estados_reservatorio[0][i],estados_reservatorio[0][i],estados_reservatorio[0][i]);
                
            }
            else if(i>5 && i<10){
                valor_led = intensidade(estados_reservatorio[1][i],0.0,0.0); //Verde

            }
            else{
                valor_led = intensidade(0.0,estados_reservatorio[1][i],0.0); //Vermelho

                
            }
            pio_sm_put_blocking(pio,sm,valor_led);

            

        }
    }
    else if(reservatorio>=(nivel_max_reservatorio/2) && reservatorio<7400){//Ente 50% e 74%

        for(uint i=0; i<pixels; i++){

            if(i>=0 && i<5 || i==5 || i==9 || i==10 || i==14 || i==15 || i==19 || i==20 || i==24){ //Cor branca do reservatório
                valor_led = intensidade(estados_reservatorio[0][i],estados_reservatorio[0][i],estados_reservatorio[0][i]);
                
            }
        
            else if(i>5 && i<10 || i>10 && i<14){
            valor_led = intensidade(estados_reservatorio[1][i],0.0,0.0); //Verde

            }
            else{
                valor_led = intensidade(0.0,estados_reservatorio[1][i],0.0); //Vermelho

            }
            pio_sm_put_blocking(pio,sm,valor_led); 
        }

    }
    else if(reservatorio>=7500 && reservatorio<9999){ //75% a 99%

        for(uint i=0; i<pixels; i++){

            if(i>=0 && i<5 || i==5 || i==9 || i==10 || i==14 || i==15 || i==19 || i==20 || i==24){ //Cor branca do reservatório
                valor_led = intensidade(estados_reservatorio[0][i],estados_reservatorio[0][i],estados_reservatorio[0][i]);
                
            }
            else if(i>5 && i<10 || i>10 && i<14 || i>15 && i<19){
                valor_led = intensidade(estados_reservatorio[1][i],0.0,0.0); //Verde

            }
            else{
                valor_led = intensidade(0.0,estados_reservatorio[1][i],0.0); //Vermelho

            }
            pio_sm_put_blocking(pio,sm,valor_led);
            
        }

    }
    else if(reservatorio==nivel_max_reservatorio){

        for(uint i=0; i<pixels; i++){
           if(i>=0 && i<5 || i==5 || i==9 || i==10 || i==14 || i==15 || i==19 || i==20 || i==24){ //Cor branca do reservatório
                valor_led = intensidade(estados_reservatorio[0][i],estados_reservatorio[0][i],estados_reservatorio[0][i]);
                
            }
            else{
                valor_led = intensidade(estados_reservatorio[1][i],0.0,0.0); //Verde

            }
            pio_sm_put_blocking(pio, sm, valor_led);

        }
       

    }
    
    
}

uint32_t intensidade(double g, double r, double b){
    unsigned char G, R, B; //Recebe um valor inteiro entre 0 e 255 (8 bits)

    G = g*255;
    R = r*255;
    B = b*255;

    return (G<<24) | (R<<16) | (B<<8); //A cada 8 bits é uma cor de 32 bits, os primeiros 8 bits não contam
}