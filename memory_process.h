#ifndef MEMORY_PROCESS_H
#define MEMORY_PROCESS_H

#include <stddef.h>
#include <stdint.h>

// Funciones de gestión de memoria
void init_memory();
void* malloc(size_t size);
void free(void* ptr);
void get_memory_stats(size_t* total, size_t* used, size_t* largest_free);

// Funciones de gestión de procesos
void init_processes();
int create_process(void* entry_point);
void switch_to_process(int pid);
void exit_process(int status);
int get_current_pid();

#endif // MEMORY_PROCESS_H