/**
 ******************************************************************************
 * @file           : room_control.c
 * @author         : Sam C
 * @brief          : Room control driver for STM32L476RGTx
 ******************************************************************************
 */
#include "room_control.h"

#include "gpio.h"    // Para controlar LEDs y leer el botón (aunque el botón es por EXTI)
#include "systick.h" // Para obtener ticks y manejar retardos/tiempos
#include "uart.h"    // Para enviar mensajes
#include "tim.h"     // Para controlar el PWM

static volatile uint8_t led_onoff_activo=0;
static volatile uint32_t led_onoff_tick=0;
static volatile uint32_t ultimo_tick=0;
static volatile uint8_t led_toggle=0;
void room_control_app_init(void)
{
    led_onoff_activo=0;
    led_onoff_tick=0;
    ultimo_tick=0;// Inicializar variables de estado si es necesario.
    // Por ejemplo, asegurar que los LEDs estén apagados al inicio

    tim3_ch1_pwm_set_duty_cycle(50); // Establecer un duty cycle inicial para el PWM LED
}

#define Tiempo_Debounce  100 

void room_control_on_button_press(void)
{
    uint32_t t_actual = systick_get_tick();
    if (t_actual - ultimo_tick  > Tiempo_Debounce)
    {
        gpio_write_pin(EXTERNAL_LED_ONOFF_PORT,EXTERNAL_LED_ONOFF_PIN,GPIO_PIN_SET );
        led_onoff_tick = t_actual;
        led_onoff_activo=1;
        uart2_send_string("Boton B1: Presionado.\r\n");
        ultimo_tick = t_actual;
    }
    // TODO: Implementar anti-rebote
    // TODO: Procesar la presion para realizar acciones
}

void room_control_on_uart_receive(char received_char)
{
    uart2_send_char(received_char); 
    if (received_char == 'h' || received_char == 'H')
    {
        tim3_ch1_pwm_set_duty_cycle(100);
     
    }
    else if (received_char == 'l' || received_char == 'L')
    {
        tim3_ch1_pwm_set_duty_cycle(0);
    }
    else if (received_char == 't'|| received_char == 'T')
    {
        led_toggle = !led_toggle;
        if(led_toggle)
        gpio_write_pin(EXTERNAL_LED_ONOFF_PORT,EXTERNAL_LED_ONOFF_PIN,led_onoff_activo ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
        else if (!led_toggle)
        {
            gpio_write_pin(EXTERNAL_LED_ONOFF_PORT,EXTERNAL_LED_ONOFF_PIN,led_onoff_activo ? GPIO_PIN_SET : GPIO_PIN_SET);
        }
    }
}

void room_control_on_systick(void) {
    if (led_onoff_activo && (systick_get_tick() - led_onoff_tick >= 3000)) {
        gpio_write_pin(EXTERNAL_LED_ONOFF_PORT, EXTERNAL_LED_ONOFF_PIN, GPIO_PIN_RESET);
        led_onoff_activo = 0;
    }
}
