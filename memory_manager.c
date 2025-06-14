#include <stddef.h>
#include <stdint.h>

// Definiciones para el gestor de memoria
#define PAGE_SIZE       0x1000      // 4KB
#define HEAP_START      0x00200000  // 2MB
#define HEAP_END        0x40000000  // 1GB
#define BLOCK_MAGIC     0x424C4B43  // "BLKC"

// Estructura de bloque de memoria
typedef struct memory_block {
    uint32_t magic;              // Identificador mágico
    uint32_t flags;              // Flags (bit 0 = usado/libre)
    size_t size;                 // Tamaño en bytes
    struct memory_block* next;   // Siguiente bloque
    struct memory_block* prev;   // Bloque anterior
} memory_block_t;

// Bloque inicial
static memory_block_t* heap_start = NULL;

// Inicializar el gestor de memoria
void init_memory() {
    // Inicializar el bloque inicial que abarca todo el heap disponible
    heap_start = (memory_block_t*)HEAP_START;
    heap_start->magic = BLOCK_MAGIC;
    heap_start->flags = 0;       // Libre
    heap_start->size = HEAP_END - HEAP_START - sizeof(memory_block_t);
    heap_start->next = NULL;
    heap_start->prev = NULL;
}

// Encontrar un bloque libre adecuado (first-fit)
static memory_block_t* find_free_block(size_t size) {
    memory_block_t* current = heap_start;
    
    while (current) {
        // Verificar si el bloque está libre y es suficientemente grande
        if (!(current->flags & 1) && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;  // No se encontró un bloque adecuado
}

// Dividir un bloque si es demasiado grande
static void split_block(memory_block_t* block, size_t size) {
    // Solo dividir si el espacio restante puede contener un bloque y algo de datos
    if (block->size >= size + sizeof(memory_block_t) + 64) {
        memory_block_t* new_block = (memory_block_t*)((uint8_t*)block + sizeof(memory_block_t) + size);
        
        // Configurar el nuevo bloque
        new_block->magic = BLOCK_MAGIC;
        new_block->flags = 0;  // Libre
        new_block->size = block->size - size - sizeof(memory_block_t);
        new_block->next = block->next;
        new_block->prev = block;
        
        // Actualizar enlaces
        if (block->next) {
            block->next->prev = new_block;
        }
        block->next = new_block;
        
        // Actualizar tamaño del bloque original
        block->size = size;
    }
}

// Implementación de malloc
void* malloc(size_t size) {
    if (size == 0) return NULL;
    
    // Alinear el tamaño a 16 bytes (alineación típica)
    size = (size + 15) & ~15;
    
    // Buscar un bloque libre adecuado
    memory_block_t* block = find_free_block(size);
    if (!block) return NULL;  // No hay memoria suficiente
    
    // Dividir el bloque si es mucho más grande que lo necesario
    split_block(block, size);
    
    // Marcar el bloque como utilizado
    block->flags |= 1;
    
    // Devolver un puntero a los datos (después de la cabecera)
    return (void*)((uint8_t*)block + sizeof(memory_block_t));
}

// Fusionar con el siguiente bloque si está libre
static void merge_next(memory_block_t* block) {
    if (block->next && !(block->next->flags & 1)) {
        block->size += sizeof(memory_block_t) + block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }
}

// Implementación de free
void free(void* ptr) {
    if (!ptr) return;
    
    // Obtener el encabezado del bloque
    memory_block_t* block = (memory_block_t*)((uint8_t*)ptr - sizeof(memory_block_t));
    
    // Verificar que sea un bloque válido
    if (block->magic != BLOCK_MAGIC) return;
    
    // Marcar como libre
    block->flags &= ~1;
    
    // Fusionar con bloques adyacentes si están libres
    if (block->prev && !(block->prev->flags & 1)) {
        merge_next(block->prev);
    } else {
        merge_next(block);
    }
}

// Función para obtener estadísticas de memoria
void get_memory_stats(size_t* total, size_t* used, size_t* largest_free) {
    *total = 0;
    *used = 0;
    *largest_free = 0;
    
    memory_block_t* current = heap_start;
    
    while (current) {
        *total += current->size;
        
        if (current->flags & 1) {
            // Bloque en uso
            *used += current->size;
        } else {
            // Bloque libre
            if (current->size > *largest_free) {
                *largest_free = current->size;
            }
        }
        
        current = current->next;
    }
}