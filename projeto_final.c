//Bibliotecas:------------------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
//------------------------------------------------------------------------------------------------------------------------------------------

//Pinos://----------------------------------------------------------------------------------------------------------------------------------
const uint led_verde = 11, led_vermelho = 13; //LEDs do RGB
const uint botao_a = 5, botao_b = 6, botao_joy = 22; //Botões
const uint buzzer_a = 10, buzzer_b = 21; //Buzzers
const uint joy_x = 27, joy_y = 26; //Joystick
//------------------------------------------------------------------------------------------------------------------------------------------

//Protótipos das funções:-------------------------------------------------------------------------------------------------------------------
void pwm_setup(); //Configura o PWM nos LEDs e Buzzers
static void gpio_irq_handler(uint gpio, uint32_t events); //Função de callback para interrupção
void setup_botoes(); //Configura os botões
void bomba(); //Bomba d'água para encher o reservatório automaticamente
void adc_setup(); //Configura o joystick
void atualizar_reservatorio(uint16_t x, uint16_t y); //Atualiza o reservatório com o joystick, Eixo x: Vazão d'água. Eixo y: Direção do motor (Se vai encher ou esvaziar)
void estado_reservatorio_led_rgb();
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
//------------------------------------------------------------------------------------------------------------------------------------------

int main()
{   pwm_setup();
    setup_botoes();
    adc_setup();
    stdio_init_all();

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
        estado_reservatorio_led_rgb();
        

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