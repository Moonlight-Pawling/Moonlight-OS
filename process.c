#include <stddef.h>
#include <stdint.h>

// Dependencia del memory_manager
extern void* malloc(size_t size);
extern void free(void* ptr);

#define MAX_PROCESSES 16
#define PROCESS_STACK_SIZE 0x10000  // 64KB por proceso

typedef struct {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t rip, rflags;
    uint64_t cs, ds, es, fs, gs, ss;
    uint64_t cr3;  // Tabla de páginas para este proceso
} process_context_t;

typedef struct {
    uint32_t pid;
    uint8_t state;    // 0=libre, 1=activo, 2=bloqueado
    process_context_t context;
    void* stack;      // Puntero a la pila del proceso
    void* page_table; // Tabla de páginas (PML4) del proceso
} process_t;

// Tabla de procesos
process_t processes[MAX_PROCESSES];
uint32_t current_process = 0;

// Variable externa para obtener la tabla PML4 actual
extern void* current_pml4;

// Declaración externa de la función que crea una nueva tabla PML4
extern void* create_page_table();

// Inicializar subsistema de procesos
void init_processes() {
    // Inicializar todos los slots como libres
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].state = 0;  // Libre
    }
    
    // Crear proceso kernel (PID 0)
    processes[0].pid = 0;
    processes[0].state = 1;  // Activo
    processes[0].page_table = current_pml4;  // Usa la tabla PML4 actual del kernel
    
    // No necesitamos asignar pila para el proceso kernel,
    // ya que ya está usando la pila del kernel
}

// Crear un nuevo proceso
int create_process(void* entry_point) {
    // Buscar slot libre
    int pid = -1;
    for (int i = 1; i < MAX_PROCESSES; i++) {
        if (processes[i].state == 0) {
            pid = i;
            break;
        }
    }
    
    if (pid == -1) return -1;  // No hay slots libres
    
    // Asignar pila
    processes[pid].stack = malloc(PROCESS_STACK_SIZE);
    if (!processes[pid].stack) return -1;
    
    // Configurar contexto inicial
    process_context_t* context = &processes[pid].context;
    // Limpiar todo el contexto
    for (int i = 0; i < sizeof(process_context_t); i++) {
        ((uint8_t*)context)[i] = 0;
    }
    
    // Configurar registros específicos
    context->rip = (uint64_t)entry_point;
    context->rsp = (uint64_t)processes[pid].stack + PROCESS_STACK_SIZE - 16;
    context->rflags = 0x202;  // Interrupciones habilitadas
    context->cs = 0x08;       // Selector de código
    context->ss = 0x10;       // Selector de datos para la pila
    
    // Por ahora, compartimos la misma tabla de páginas con el kernel
    // Esto significa que todos los procesos comparten el mismo espacio de dirección
    // En una implementación completa, crearíamos una nueva tabla para cada proceso
    processes[pid].page_table = current_pml4;
    context->cr3 = (uint64_t)current_pml4;
    
    // Activar proceso
    processes[pid].pid = pid;
    processes[pid].state = 1;
    
    return pid;
}

// Prototipo de función en ensamblador para cambio de contexto
// Esta función necesita implementarse en el archivo assembly
extern void switch_context(process_context_t* old, process_context_t* new);

// Cambiar a otro proceso
void switch_to_process(int pid) {
    if (pid < 0 || pid >= MAX_PROCESSES || processes[pid].state != 1) {
        return;  // Proceso inválido o no está activo
    }
    
    // No hacer nada si intentamos cambiar al mismo proceso
    if (pid == current_process) return;
    
    int old_process = current_process;
    current_process = pid;
    
    // Realizar el cambio de contexto
    switch_context(&processes[old_process].context, &processes[current_process].context);
}

// Terminar el proceso actual
void exit_process(int status) {
    if (current_process == 0) {
        // No se puede terminar el proceso del kernel
        return;
    }
    
    // Liberar recursos
    free(processes[current_process].stack);
    
    // Marcar como libre
    processes[current_process].state = 0;
    
    // Volver al proceso del kernel
    switch_to_process(0);
    
    // Nunca llegará aquí
}

// Función para obtener el PID actual
int get_current_pid() {
    return (int)current_process;
}