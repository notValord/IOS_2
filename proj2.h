// proj2.h
// Řešení IOS-2.projekt, 30.4.2021
// Autor: Veronika Molnárová, xmolna08
// Synchornizacia a paralelizacia , hlaviskovy subor

#ifndef IOS_2_PROJ2_H
#define IOS_2_PROJ2_H

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define ERROR_VALUE 1
#define OUT_FILE "proj2.out"

void free_sources();
void init_sources();

void print(char* message, unsigned int id);
void sleep_rand(unsigned int max);

void sem_n_post(sem_t* semaphore, int times);
void elf_holiday(unsigned int ID_elf, sem_t* semaphore);

void santa();

void elf(unsigned int ID_elf);

void deer(unsigned int ID_deer);

int create_elves();
int create_deers();

void number_only(char *string);
bool check_argument(long bottom, long top, long arg);
void parsing_arguments(int argc, char **argv);

void wait_children();

#endif //IOS_2_PROJ2_H
