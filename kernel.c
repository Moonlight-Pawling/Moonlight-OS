// Añadir al principio del archivo kernel.c
#include "memory_process.h"

// Variables externas para la detección de memoria
extern uint32_t total_mem_low;
extern uint32_t total_mem_high;

// Añadir después de las declaraciones existentes
void format_memory_size(uint64_t bytes, char* buffer);
uint64_t detect_memory();

// Esta función convierte el tamaño de memoria en un formato legible
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
    else if (bytes >= (1ULL << 20)) {  // >= 1MB
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

// Esta función devuelve la memoria total detectada
uint64_t detect_memory() {
    // Combinar las variables externas
    uint64_t memory_size = ((uint64_t)total_mem_high << 32) | total_mem_low;
    return memory_size;
}

// Modificar la función kernel_main() para incluir la inicialización
void kernel_main() {
    // Código existente para limpiar pantalla...
    clear_screen();
    
    // Mostrar mensaje de bienvenida
    print(0, 0, "MoonlightOS v1.0", 0x0A);
    print(0, 1, "==================", 0x0A);
    
    // Inicializar subsistemas
    init_memory();    // Inicializar el gestor de memoria
    init_processes(); // Inicializar el gestor de procesos
    
    // Mostrar información técnica
    char mem_str[32];
    char mem_friendly[32];
    uint64_t memory = detect_memory();
    uint64_to_hex(memory, mem_str);
    format_memory_size(memory, mem_friendly);
    
    print(0, 3, "Informacion del Sistema:", 0x0B);
    print(2, 4, "- Arquitectura: x86_64", 0x07);
    print(2, 5, "- Memoria detectada: ", 0x07);
    print(22, 5, mem_str, 0x0E);
    print(41, 5, "(", 0x07);
    print(42, 5, mem_friendly, 0x0E);
    print(42 + strlen(mem_friendly), 5, ")", 0x07);
    
    // Prueba del gestor de memoria
    print(0, 7, "Prueba de asignacion de memoria:", 0x0B);
    
    // Obtener estadísticas iniciales
    size_t total, used, largest_free;
    get_memory_stats(&total, &used, &largest_free);
    
    char stats_buffer[32];
    
    format_memory_size(total, stats_buffer);
    print(2, 8, "- Memoria total en heap: ", 0x07);
    print(26, 8, stats_buffer, 0x0E);
    
    format_memory_size(used, stats_buffer);
    print(2, 9, "- Memoria usada: ", 0x07);
    print(26, 9, stats_buffer, 0x0E);
    
    // Asignar algunos bloques de prueba
    void* block1 = malloc(1024*1024);    // 1MB
    void* block2 = malloc(2*1024*1024);  // 2MB
    
    print(2, 11, "- Asignado bloque 1 (1MB): ", 0x07);
    print(29, 11, block1 ? "OK" : "FALLO", block1 ? 0x0A : 0x0C);
    
    print(2, 12, "- Asignado bloque 2 (2MB): ", 0x07);
    print(29, 12, block2 ? "OK" : "FALLO", block2 ? 0x0A : 0x0C);
    
    // Obtener estadísticas después de asignación
    get_memory_stats(&total, &used, &largest_free);
    
    format_memory_size(used, stats_buffer);
    print(2, 14, "- Memoria usada ahora: ", 0x07);
    print(26, 14, stats_buffer, 0x0E);
    
    // Liberar un bloque
    free(block1);
    print(2, 16, "- Liberado bloque 1", 0x07);
    
    // Volver a verificar
    get_memory_stats(&total, &used, &largest_free);
    format_memory_size(used, stats_buffer);
    print(2, 17, "- Memoria usada final: ", 0x07);
    print(26, 17, stats_buffer, 0x0E);
    
    // Indicar que el kernel está listo
    print(0, 20, "Sistema inicializado correctamente.", 0x0A);
    print(0, 21, "Presiona cualquier tecla para entrar al shell...", 0x07);
    
    // En una versión real, aquí esperarías input y lanzarías el shell
    
    // Por ahora, simplemente hacemos un bucle infinito
    while (1) {
        // Bucle infinito al final
    }
}