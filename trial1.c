#include <stdint.h>
// Direcciones de E/S mapeadas en memoria para controlar los pines de E/S
#define GPIO_ADDR (*(volatile uint32_t*)0x02000008)
#define LEDS_ADDR (*(volatile uint32_t*)0x03000000)  // Dirección para controlar los LEDs
// Secuencia de pasos del motor - modo de medio paso
const uint8_t step_sequence[] = {
    0b0001,  // Bobina A energizada
    0b0011,  // Bobinas A y B energizadas
    0b0010,  // Bobina B energizada
    0b0110,  // Bobinas B y C energizadas
    0b0100,  // Bobina C energizada
    0b1100,  // Bobinas C y D energizadas
    0b1000,  // Bobina D energizada
    0b1001   // Bobinas A y D energizadas
};
// Función para generar un retardo en milisegundos
void delay(uint32_t milliseconds) {
    for (volatile uint32_t i = 0; i < (milliseconds * 1000); i++) {
        asm volatile("nop");
    }
}
uint8_t step_sequence_reverse[(sizeof(step_sequence) / sizeof(step_sequence[0]))*250];
// Función para controlar el motor paso a paso
void step_motor(uint8_t direction, uint32_t delay_ms, uint8_t reverse) {
    const uint8_t* sequence = (reverse == 0) ? step_sequence : step_sequence_reverse;
    uint16_t num_steps = (sizeof(step_sequence) / sizeof(step_sequence[0]))*250;
    uint8_t step = (direction == 0) ? 0 : num_steps - 1;
    while (1) {
        GPIO_ADDR = sequence[step];      // Aplicar el valor del paso al control del motor
        LEDS_ADDR = 1 << step;           // Encender el LED correspondiente al paso del motor
        delay(1000);
        if (direction == 0) {
            step++;
            if (step >= num_steps) {
                step = 0;
                delay(delay_ms);
            }
        } else {
            step--;
            if (step < 0) {
                step = num_steps - 1;
                delay(delay_ms);
            }
        }
        delay(delay_ms);
    }
}
int main() {
    // Crear una copia del array step_sequence y revertirlo
    for (int i = 0; i < (sizeof(step_sequence) / sizeof(step_sequence[0]))*250; i++) {
        step_sequence_reverse[i] = step_sequence[(sizeof(step_sequence) / sizeof(step_sequence[0]))*250 - 1 - i];
    }
    // Mover el motor paso a paso en una dirección, pausar y luego volver
    while (1) {
        // Realizar el movimiento del motor en sentido horario y opuesto 
        step_motor(0, 1000, 1);  // Dirección: 0 (en sentido horario), Pausa: 1000 ms, Usar reversa
        // Detener el motor
        GPIO_ADDR = 0;  // Aplicar 0 al control del motor para detenerlo
        LEDS_ADDR = 0;  // Apagar todos los LEDs
        // Esperar 2 horas
        for (volatile uint32_t i = 0; i < 2 * 60 * 60 * 1000; i++) {
            delay(1);
        }
    }
    return 0;
}
