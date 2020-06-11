#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct PetersonVars {
  bool turn;
  bool interested_0;
  bool interested_1; 
} PetersonVars;

int nloop = 50;

/**********************************************************\
 * Function: increment a counter by some amount one by one *
 * argument: ptr (address of the counter), increment       *
 * output  : nothing                                       *
 **********************************************************/
void add_n(int *ptr, int increment){
  int i,j;
  for (i=0; i < increment; i++){
    *ptr = *ptr + 1;
    for (j=0; j < 1000000;j++);
  }
}

void initPeterson(PetersonVars *vars) {
  vars->interested_0 = false;
  vars->interested_1 = false;
}

int main(){
  int pid;        /* Process ID                     */

  int *countptr;  /* pointer to the counter         */

  int fd;     /* file descriptor to the file "containing" my counter */
  int zero = 0; /* a dummy variable containing 0 */

  system("rm -f counter");

  /* create a file which will "contain" my shared variable */
  fd = open("counter",O_RDWR | O_CREAT);
  write(fd,&zero,sizeof(int));

  /* map my file to memory */
  countptr = (int *) mmap(NULL, sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fd,0);


 
  if (!countptr) {
    printf("Mapping failed\n");
    exit(1);
  }
  *countptr = 0;
  close(fd);

  PetersonVars *petersonptr;
  int petersonFile;
  system("rm -f peterson");
  petersonFile = open("peterson",O_RDWR | O_CREAT);
  write(petersonFile,&zero,sizeof(PetersonVars));
  petersonptr = (PetersonVars *) mmap(NULL, sizeof(PetersonVars),PROT_READ | PROT_WRITE, MAP_SHARED, petersonFile,0);

  if (!petersonptr) {
    printf("Peterson Mapping failed\n");
    exit(1);
  }
  initPeterson(petersonptr);
  close(petersonFile);

  setbuf(stdout,NULL);

  pid = fork();
  if (pid < 0){
    printf("Unable to fork a process\n");
    exit(1);
  }

  if (pid == 0) {
    /* The child increments the counter by two's */
    while(1) {
      petersonptr->turn = true;
      petersonptr->interested_0 = true;
      while(petersonptr->turn && petersonptr->interested_1) {
        if (*countptr < nloop) {
          add_n(countptr,2);
          printf("Child process -->> counter= %d\n",*countptr);
        } else {
          petersonptr->interested_0 = false;
          close(petersonFile);
          close(fd);
          break;
        }
        petersonptr->interested_0 = false;
        close(petersonFile);
        close(fd);
      }
      if (*countptr >= nloop) {
        break;
      }
    }
  } else {
    /* The parent increments the counter by twenty's */
    while(1) {
      petersonptr->turn = false;
      petersonptr->interested_1 = true;
      while(!petersonptr->turn && petersonptr->interested_0) {
        if (*countptr < nloop) {
          add_n(countptr,20);
          printf("Parent process -->> counter = %d\n",*countptr);
        } else {
          petersonptr->interested_1 = false;
          close(petersonFile);
          close(fd);
          break;
        }
        petersonptr->interested_1 = false;
        close(petersonFile);
        close(fd);
      }
      if (*countptr >= nloop) {
        break;
      }
    }
  }


}








