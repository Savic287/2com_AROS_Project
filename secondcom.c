#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>

#define SIZE 4096
#define IPC_RESULT_ERROR (-1)
#define NAME "firstcom.c"

#define SEM_FIRST "/myfirstcom"
#define SEM_SECOND "/mysecondcom"

static int get_shared_block(char *filename, int size){

        key_t key;

        key = ftok(filename, 0);

        if(key == IPC_RESULT_ERROR){
                return IPC_RESULT_ERROR;
        }
        return shmget(key, size, 0644 | IPC_CREAT);
}

char *attach_memory_block(char *filename, int size){
        int shared_block_id = get_shared_block(filename, size);
        char *result;
        result = shmat(shared_block_id, NULL, 0);
        if(result == (char *)IPC_RESULT_ERROR){
                return NULL;
        }
        return result;
}

bool detach_memory_block(char *block){
        return (shmdt(block) != IPC_RESULT_ERROR);
}

bool destroy_memory_block(char *filename){

        int shared_block_id = get_shared_block(filename, 0);
        if(shared_block_id == IPC_RESULT_ERROR){
                return NULL;
        }
	return (shmctl(shared_block_id, IPC_RMID, NULL) != IPC_RESULT_ERROR);
}

int main(void){

	char string[50];

        char *b = attach_memory_block(NAME, SIZE);

        if(b == NULL){
                printf("Error! Unable to get shm block!");
                return -1;
        }

	sem_unlink(SEM_FIRST);
        sem_unlink(SEM_SECOND);

        sem_t *sem_firs = sem_open(SEM_FIRST, O_CREAT, 0660, 0);
        sem_t *sem_seco = sem_open(SEM_SECOND, O_CREAT, 0660, 1);

        while(1){
                sem_wait(sem_firs);
                if(strlen(b)>0){
                        printf("Other: \"%s\"\n", b);
                        bool done = (strcmp(b,"bye") == 0);
                        b[0] = 0;
                        if(done){
				printf("Bye\n");
                                sem_post(sem_seco);
				sem_close(sem_seco);
      				 sem_close(sem_firs);
       				 detach_memory_block(b);

			        return 0;

				//break;
                        }

		printf("\nMy answer: ");
		scanf("%s", string);
		sprintf(b, "%s", string);

		 bool bye = (strcmp(string,"bye") == 0);
                if(bye){
                         sem_post(sem_seco);
                         sem_close(sem_firs);
                         sem_close(sem_seco);
                         detach_memory_block(b);

                        return 0;

                        }


                }
                sem_post(sem_seco);

        }
	sem_close(sem_seco);
        sem_close(sem_firs);
        detach_memory_block(b);

        return 0;
}

