// Kernel en modo largo (64 bits) - MoonlightOS
#include <stdint.h>

// Define un puntero a la memoria de video
volatile uint16_t* video = (volatile uint16_t*)0xB8000;

// Variables externas desde kernel.asm 
extern uint32_t total_mem_low;
extern uint32_t total_mem_high;

// Limpia la pantalla completa
void clear_screen(uint8_t color_attr) {
    uint16_t blank = ((uint16_t)color_attr << 8) | 0x20;
    for (int i = 0; i < 2000; i++) {
        video[i] = blank;
    }
}

// Escribe un carácter en la memoria de video
void putchar(int x, int y, char c, uint8_t color) {
    const int index = y * 80 + x;
    video[index] = ((uint16_t)color << 8) | c;
}

// Escribe una cadena en pantalla
void print(int x, int y, const char* str, uint8_t color) {
    for (int i = 0; str[i]; i++) {
        putchar(x + i, y, str[i], color);
    }
}

// Función para convertir número a string hexadecimal
void uint64_to_hex(uint64_t value, char* buffer) {
    const char hex_chars[] = "0123456789ABCDEF";
    buffer[0] = '0';
    buffer[1] = 'x';
    
    for (int i = 15; i >= 0; i--) {
        buffer[2 + (15-i)] = hex_chars[(value >> (i * 4)) & 0xF];
    }
    buffer[18] = '\0';
}

// Función para convertir número a string decimal
void uint64_to_dec(uint64_t value, char* buffer) {
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    
    char temp[32];
    int i = 0;
    
    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    for (int j = 0; j < i; j++) {
        buffer[j] = temp[i - 1 - j];
    }
    buffer[i] = '\0';
}

// Detectar cantidad de RAM
uint64_t detect_memory() {
    // Combinar las variables externas
    uint64_t memory_size = ((uint64_t)total_mem_high << 32) | total_mem_low;
    return memory_size;
}

// Punto de entrada del kernel 64-bit
void kernel_main() {
    // Limpiar la pantalla
    clear_screen(0x07);
    
    // Header principal
    print(0, 0, "===============================================================================", 0x0F);
    print(0, 1, "                        MoonlightOS - Modo Largo (64-bit)", 0x0F);
    print(0, 2, "===============================================================================", 0x0F);
    
    // Información del sistema
    print(0, 4, "Sistema inicializado correctamente", 0x0A);
    print(0, 5, "Desarrollado por Moonlight-Pawling", 0x0C);
    
    // Mostrar información técnica
	// Mostrar información técnica
	char mem_str[32];
	char mem_friendly[32];
	uint64_t memory = detect_memory();
	uint64_to_hex(memory, mem_str);
	format_memory_size(memory, mem_friendly);

	print(0, 7, "Informacion del Sistema:", 0x0B);
	print(2, 8, "- Arquitectura: x86_64", 0x07);
	print(2, 9, "- Memoria detectada: ", 0x07);
	print(22, 9, mem_str, 0x0E);
	print(41, 9, "(", 0x07);
	print(42, 9, mem_friendly, 0x0E);
	print(42 + 8, 9, ")", 0x07);
    
    // Status bar
    print(0, 21, "Estado: Sistema base funcionando - Listo para expansiones", 0x0A);
    
    // Footer con timestamp y heartbeat
    print(0, 23, "Tiempo de funcionamiento: ", 0x07);
    
    // Heartbeat animado con contador
    uint64_t counter = 0;
    char heartbeat[] = {'|', '/', '-', '\\'};
    char counter_str[32];
    
    while(1) {
        // Mostrar heartbeat
        putchar(79, 24, heartbeat[counter % 4], 0x0E);
        
        // Mostrar contador de segundos
        uint64_to_dec(counter / 10, counter_str);
        print(26, 23, counter_str, 0x0E);
        print(26 + 10, 23, "s   ", 0x07); // Limpiar espacios extra
        
        // Retardo usando contador de 64 bits
        for (volatile uint64_t i = 0; i < 50000000ULL; i++) { }
        
        counter++;
        
        // Cada 50 ciclos, mostrar mensaje de sistema activo
        if (counter % 50 == 0) {
            print(0, 22, "Sistema activo y estable", 0x0A);
        }
    }
}

void format_memory_size(uint64_t bytes, char* buffer) {
    if (bytes >= (1ULL << 30)) {  // >= 1GB
        uint64_t gb = bytes / (1ULL << 30);
        uint64_t decimal = (bytes % (1ULL << 30)) / ((1ULL << 30) / 10);
        
        char gb_str[16];
        uint64_to_dec(gb, gb_str);
        
        char decimal_str[4] = "0";
        if (decimal > 0) {
            uint64_to_dec(decimal, decimal_str);
        }
        
        int i = 0;
        while (gb_str[i]) {
            buffer[i] = gb_str[i];
            i++;
        }
        
        if (decimal > 0) {
            buffer[i++] = '.';
            buffer[i++] = decimal_str[0];
        }
        
        buffer[i++] = ' ';
        buffer[i++] = 'G';
        buffer[i++] = 'B';
        buffer[i] = '\0';
    } 
    else if (bytes >= (1ULL << 20)) {
        uint64_t mb = bytes / (1ULL << 20);
        char mb_str[16];
        uint64_to_dec(mb, mb_str);
        
        int i = 0;
        while (mb_str[i]) {
            buffer[i] = mb_str[i];
            i++;
        }
        
        buffer[i++] = ' ';
        buffer[i++] = 'M';
        buffer[i++] = 'B';
        buffer[i] = '\0';
    }
    else {
        uint64_to_dec(bytes / 1024, buffer);
        int i = 0;
        while (buffer[i]) i++;
        buffer[i++] = ' ';
        buffer[i++] = 'K';
        buffer[i++] = 'B';
        buffer[i] = '\0';
    }
}