#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>

void ChildProcess(int *ShmPTR) {
    int account, balance;
    int i;
    for (i = 0; i < 25; i++) {
        // Sleep for a random time between 0-5 seconds
        sleep(rand() % 6);

        // Copy BankAccount to local variable
        account = ShmPTR[0];

        // Wait for Turn to be 1
        while (ShmPTR[1] != 1);

        // Generate a random balance needed between 0-50
        balance = rand() % 51;
        printf("Poor Student needs $%d\n", balance);

        // Perform withdrawal if possible
        if (balance <= account) {
            account -= balance;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        // Copy back to BankAccount
        ShmPTR[0] = account;

        // Set Turn to 0
        ShmPTR[1] = 0;
    }
}

void ParentProcess(int *ShmPTR) {
    int account, balance;
    int i;
    for (i = 0; i < 25; i++) {
        // Sleep for a random time between 0-5 seconds
        sleep(rand() % 6);

        // Copy BankAccount to local variable
        account = ShmPTR[0];

        // Wait for Turn to be 0
        while (ShmPTR[1] != 0);

        // Perform operations based on account balance
        if (account <= 100) {
            // Generate a random balance between 0-100
            balance = rand() % 101;
            if (balance % 2 == 0) {
                account += balance;
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
            } else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        } else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }

        // Copy back to BankAccount
        ShmPTR[0] = account;

        // Set Turn to 1
        ShmPTR[1] = 1;
    }
}




int main(){
  int shm_id = shmget(IPC_PRIVATE, sizeof(int) * 2, IPC_CREAT | 0666);
  if (shm_id < 0){
    perror("shmget failed");
    exit(1);
  }

  int *ShmPTR = (int *) shmat(shm_id, NULL, 0);
  if (ShmPTR == (void *) - 1){
    perror("shmat failed");
    exit(1);
  }

  ShmPTR[0] = 0;
  ShmPTR[1] = 0;

  srand(time(NULL));

  pid_t pid = fork();
  if (pid < 0){
    perror("fork failed");
    exit(1);
  } else if (pid == 0){
     ChildProcess(ShmPTR);
      exit(0);
  }else{
  ParentProcess(ShmPTR);
  wait(NULL);
  if (shmdt((void *) ShmPTR) == -1) {
              perror("shmdt failed");
              exit(1);
          }

          if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
              perror("shmctl failed");
              exit(1);
          }

          printf("Process completed.\n");
}
return 0;
}