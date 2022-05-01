// proj2.c
// Řešení IOS-2.projekt, 30.4.2021
// Autor: Veronika Molnárová, xmolna08
// Synchornizacia a paralelizacia procesov

#include "proj2.h"

//Shared memory
unsigned int *message_nmb, *deer_finished, *santa_status, *elf_waiting, *elf_finished;
bool *take_holiday;

//Semaphores
sem_t *printing_message, *deer_returned, *deer_hitched, *santa_working,
      *santa_mutex, *elf_need_help, *elf_got_help, *elves_busy;

//Global variables
unsigned int number_elves, number_deers, elf_delay, deer_delay;
FILE *fptr;

void free_sources(){
    munmap(message_nmb, sizeof(message_nmb));
    munmap(deer_finished, sizeof(deer_finished));
    munmap(santa_status, sizeof(santa_status));
    munmap(elf_waiting, sizeof(elf_waiting));
    munmap(elf_finished, sizeof(elf_finished));
    munmap(take_holiday, sizeof(take_holiday));

    sem_close(printing_message);
    sem_close(deer_returned);
    sem_close(deer_hitched);
    sem_close(santa_working);
    sem_close(santa_mutex);
    sem_close(elf_need_help);
    sem_close(elf_got_help);
    sem_close(elves_busy);
    sem_unlink("xmolna08_sem_print_message");
    sem_unlink("xmolna08_sem_deer_returned");
    sem_unlink("xmolna08_sem_deer_hitched");
    sem_unlink("xmolna08_sem_santa_working");
    sem_unlink("xmolna08_sem_santa_mutex");
    sem_unlink("xmolna08_sem_elf_needhelp");
    sem_unlink("xmolna08_sem_elf_gothelp");
    sem_unlink("xmolna08_sem_elves_busy");
}
void init_sources(){
    free_sources(); //to clean unliked semaphores or other mess

    message_nmb = mmap(NULL, sizeof (int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    deer_finished = mmap(NULL, sizeof (int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    santa_status = mmap(NULL, sizeof (int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    elf_waiting = mmap(NULL, sizeof (int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    elf_finished = mmap(NULL, sizeof (int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    take_holiday = mmap(NULL, sizeof (int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    santa_working = sem_open("xmolna08_sem_santa_working", O_CREAT, 0660, 0);
    santa_mutex = sem_open("xmolna08_sem_santa_mutex", O_CREAT, 0660, 1);
    printing_message = sem_open("xmolna08_sem_print_message", O_CREAT, 0660, 1);
    deer_returned = sem_open("xmolna08_sem_deer_returned", O_CREAT, 0660, 1);
    deer_hitched = sem_open("xmolna08_sem_deer_hitched", O_CREAT, 0660, 0);
    elf_need_help = sem_open("xmolna08_sem_elf_needhelp", O_CREAT, 0660, 1);
    elf_got_help = sem_open("xmolna08_sem_elf_gothelp", O_CREAT, 0660, 0);
    elves_busy = sem_open("xmolna08_sem_elves_busy", O_CREAT, 0660, 3);

    if ( santa_working == SEM_FAILED || santa_mutex == SEM_FAILED ||
         printing_message == SEM_FAILED || deer_returned == SEM_FAILED ||
         deer_hitched == SEM_FAILED || elf_need_help == SEM_FAILED ||
         elf_got_help == SEM_FAILED || elves_busy == SEM_FAILED){
        fprintf(stderr, "ERROR: Couldn't open semaphores\n");
        exit(ERROR_VALUE);
    }


    fptr = fopen(OUT_FILE, "w");
    if (fptr == NULL){
        fprintf(stderr, "Error, opening file failed\n");
        exit(ERROR_VALUE);
    }

    *message_nmb = 1; //numbering of printed messages
    *deer_finished = number_deers; //how many deers came home
    *santa_status = 0;
    *elf_waiting = 0;
    *elf_finished = 0;
    *take_holiday = false;
}

void print(char* message, unsigned int id){
    sem_wait(printing_message);
    fprintf(fptr,"%u: ", *message_nmb);
    if (id == 0){
        fprintf(fptr, "%s",message);
    }
    else{
        fprintf(fptr, message, id);
    }
    fflush(fptr);
    *message_nmb = *message_nmb + 1;
    sem_post(printing_message);
}
void sleep_rand(unsigned int max){
    if (max == 0){ //no sleep
        return;
    }
    srand(time(NULL));
    int time = (rand() % max) + 1;
    usleep(time);
}

void sem_n_post(sem_t* semaphore, int times){
    for (int i = 0; i < times; ++i) {
        sem_post(semaphore);
    }
}
void elf_holiday(unsigned int ID_elf, sem_t* semaphore){
    if (*take_holiday){ //elf takes holiday when santa closed his workshop
        print("Elf %u: taking holidays\n", ID_elf);
        sem_post(semaphore); // open the semaphore he came from
        sem_post(elves_busy); // can let all on the elves in to take holiday
        exit(0);
    }
}

void santa(){
    print("Santa: going to sleep\n", 0);
    while(1){
        sem_wait(santa_working); // santa is asleep

        if(*santa_status == 1){
            print("Santa: helping elves\n",0);
            sem_post(elf_got_help); // let one elf in

            sem_wait(santa_working); // wait for elves to wake him up again
            print("Santa: going to sleep\n",0);
        }

        else if (*santa_status == 2){
            print("Santa: closing workshop\n",0);
            *take_holiday = true;
            sem_post(deer_hitched); // let one deer in
            sem_post(elf_got_help); // let elves to take holidays

            sem_wait(santa_working); // wait for deers to wake him up again
            print("Santa: Christmas started\n", 0);
            exit(0);
        }
    }
}


void elf(unsigned int ID_elf){
    print("Elf %u: started\n", ID_elf);

    while (1){ // not an endless loop, end in elf_holiday() function
        sleep_rand(elf_delay);

        sem_wait(elf_need_help);
        print("Elf %u: need help\n", ID_elf);

        elf_holiday(ID_elf, elf_need_help); // if santa closed his workshop
        *elf_waiting = *elf_waiting + 1;

        if (*elf_waiting == 3){
            sem_wait(santa_mutex); // checks if nobody is working with santa

            if (*deer_finished == 0){ // if deers are waiting at the same time, they should go first
                sem_post(santa_mutex); // let deers take santa
                sem_wait(deer_returned); // wait for deers to finish
                sem_post(deer_returned);
                sem_wait(santa_mutex); // wait unlit santa is done with deers and is leaving
            }

            *santa_status = 1;
            *elf_waiting = *elf_waiting - 3;
            sem_post(santa_working);
        }
        sem_post(elf_need_help);

        sem_wait(elves_busy); // if there already are 3 elves inside, wait

        sem_wait(elf_got_help); // wait until santa is going to help them
        elf_holiday(ID_elf, elf_got_help);
        print("Elf %u: get help \n", ID_elf);
        *elf_finished = *elf_finished + 1;
        if (*elf_finished == 3){ // the last elf
            *elf_finished = 0;
            *santa_status = 0; // santa back to sleep
            sem_post(santa_working);
            sem_post(santa_mutex);
            sem_n_post(elves_busy, 3);
        }
        else{
            sem_post(elf_got_help); // let another elf inside
        }
    }
}


void deer(unsigned int ID_deer){
    print("RD %u: rstarted\n", ID_deer);
    sleep_rand(deer_delay);

    sem_wait(deer_returned);
    print("RD %u: return home\n", ID_deer);
    *deer_finished = *deer_finished - 1;
    if (*deer_finished == 0){ // last deer
            sem_wait(santa_mutex); // checks if santa is free
            *deer_finished = number_deers; // reset counter
            *santa_status = 2;
            sem_post(santa_working); // let santa know all deers are home
    }
    sem_post(deer_returned);

    sem_wait(deer_hitched); // wait until santa comes to hitch them
    print("RD %u: get hitched\n", ID_deer);
    *deer_finished = *deer_finished - 1;
    if (*deer_finished == 0){ // last deer
        *deer_finished = 1; // need to change value so elves won't be giving them santa when they already finished
        sem_post(santa_working);
        sem_post(santa_mutex); // ended work with santa
    }
    else{
        sem_post(deer_hitched); // let another one in
    }
    exit(0);
}

int create_elves(){
    pid_t PID_elf;
    for (unsigned int i = 1; i <= number_elves; ++i) {
        PID_elf = fork();
        if (PID_elf == 0){
            elf(i);
        }
        else if (PID_elf == -1){
            fprintf(stderr, "ERROR: Fork failed\n");
            return (ERROR_VALUE);
        }
    }
    return 0;
}
int create_deers(){
    pid_t PID_deer;
    for (unsigned int i = 1; i <= number_deers; ++i) {
        PID_deer = fork();
        if (PID_deer == 0){
            deer(i);
        }
        else if (PID_deer == -1){
            fprintf(stderr, "ERROR: Fork failed\n");
            return (ERROR_VALUE);
        }
    }
    return 0;
}

void number_only(char *string){
    for (unsigned int i = 0; i < strlen(string); ++i) {
        if (!isdigit(string[i])){ // if aregument is not a number only
            fprintf(stderr, "ERROR: Wrong type of arguments\n");
            exit(ERROR_VALUE);
        }
    }
}
bool check_argument(long bottom, long top, long arg){
    if (arg > bottom && arg < top){ // if the arguments are in the limits
        return true;
    }
    else{
        fprintf(stderr, "ERROR: Wrong type of arguments\n");
        exit(ERROR_VALUE);
    }
}
void parsing_arguments(int argc, char **argv){ // contol arguments, set values
    if (argc != 5){
        fprintf(stderr, "ERROR: Wrong number of arguments\n");
        exit(ERROR_VALUE);
    }

    for (int i = 1; i < argc; ++i) {
        number_only(argv[i]);
    }

    long tmp;
    tmp = strtol(argv[1], NULL, 10);
    if (check_argument(0, 1000, tmp)){
        number_elves = tmp;
    }

    tmp = strtol(argv[2], NULL, 10);
    if (check_argument(0, 20, tmp)){
        number_deers = tmp;
    }

    tmp = strtol(argv[3], NULL, 10);
    if (check_argument(-1, 1001, tmp)){
        elf_delay = tmp;
    }

    tmp = strtol(argv[4], NULL, 10);
    if (check_argument(-1, 1001, tmp)){
        deer_delay = tmp;
    }
}

void wait_children(){
    while (wait(NULL) != -1){
        //blank while, waits for all processes to finish
    }
}
int main(int argc, char **argv) {

    parsing_arguments(argc, argv);
    init_sources();

    pid_t PID_santa = fork();

    if (PID_santa == 0){
        santa();
    }
    else if (PID_santa == -1){
        fprintf(stderr, "ERROR: Fork failed\n");
        free_sources();
        fclose(fptr);
        exit(ERROR_VALUE);
    }
    else{
        if ( create_elves() ){
            free_sources();
            fclose(fptr);
            exit(ERROR_VALUE);
        }

        if ( create_deers() ){
            free_sources();
            fclose(fptr);
            exit(ERROR_VALUE);
        }
    }

    wait_children();
    free_sources();
    fclose(fptr);

    return 0;
}
