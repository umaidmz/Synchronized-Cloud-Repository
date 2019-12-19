#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>

#define SIZE 10
#define MEM_SIZE 4096
#define MEM_NAME_USERS "USER"
#define MEM_NAME_PWDS "PWDS"

const char *USER = "admin";
const char *PWD = "root";

extern const char *USER;
extern const char *PWD;

bool verifyCreds(char*, char*);
char **tokenize(char*);


bool verifyCreds(char* user, char* pwd)
{
  /*
  Verifies the credentials passed as a parameter to this function
  as strings. This is used by the system to confirm that the user 
  is the admin.
  */
  if(strcmp(user, USER)){
      fprintf(stderr, "Incorrect User or Password\n");
      return false;
    }
    else if(strcmp(pwd, PWD)){
      fprintf(stderr, "Incorrect User or Password\n");
      return false;
    }
    else{
    printf("%s\n","Password Accepted" );
    return true;
    }
}

void getCreds(char* user, char* pwd)
{
  /*
  An input function which allows the user to enter their credentials
  through the standard input.
  /*/
  printf("%s\n", "Username: ");
  scanf(" %s", user);
  printf("%s\n", "Password: ");
  scanf(" %s", pwd);
  
  return;
}

int getAdminCreds(char* user, char *pwd)
{
  /*
  Similar to getCreds() except it allows 3 tries to the user who is 
  apparently the admin of the system.
  /*/
    int i =0;
    do
    {
      getCreds(user, pwd);
      if(verifyCreds(user,pwd))
        return 1;
      i++;
    }while(i < 3);
    if(i==3)
    {
     return 0;
    }
}

void appendSpace(char* user)
{
  /*
  Appends Space to the input provided as argument. 
  /*/
  strcat(user, " ");
}

int getEmptyIndex(char *array[])
{
  /*
  Returns the next available index in an array which is NULL.
  /*/
  for(int i=0; i<SIZE; i++)
  {
    if(array[i] == NULL)
    {
      return i;
    }
  }
  return -1;
}

void insertUser(char users[][20], char pwds[][20], int index)
{
  /*
  Takes input from admin regarding username and password which is inserted into
  users and pwds array which passed as argument. Since we are working with pointers,
  the additions to the users and pwds array will be inplace.
  /*/
  char user[20], pwd[20];
  getCreds(user, pwd);
  strcpy(users[index], user);
  //users[index] = user;
  strcpy(pwds[index], pwd);
  //pwds[index] = pwd;
  printf("User Inserted %s %s\n",users[index], pwds[index]);
  return;
}

int preInitSharedMemory(char usernames[][20], char passwords[][20], int mode)
{
    /*
    Star function for this library. Takes as argument usernames and passwords array
    which are empty two-dimensional array used to store the data that has been stored
    in the shared memory. The function retrieves the data as a stream of characters 
    and parses them into usernames and passwords. These are saved accordingly in their
    precise location as a contiguous memory in the arrays passed as arguments. The
    function returns an integer which is the value of the next available index. 
    [This function is specifically commented]
    /*/
    int shm_fd_users_init, shm_fd_pwds, index=0;                  //Initializing Shared Memory File Descriptors         
    void *ptr;
    char str_init[200]={0}, tmp[200];

    shm_fd_users_init = shm_open(MEM_NAME_USERS, O_RDONLY, 0666); // Initializing Shared Memory
    shm_fd_pwds = shm_open(MEM_NAME_PWDS, O_RDONLY, 0666);
    if(shm_fd_users_init > 0)
    {
    
    ptr = mmap(0, MEM_SIZE, PROT_READ, MAP_SHARED, shm_fd_users_init, 0); // Maps teh Shared Memory to a void pointer which relays the stream of data 
    
    strcpy(str_init, (char*)ptr); //storing teh stream of data in a local variable
    
    { // The following function tokenizes the stream of data according to a space and parses the users and passwords
      int i =0, j=0, k=0;
        while(1)
        {
          if(((int)str_init[i] >= 65 && (int)str_init[i] <= 90) || ((int)str_init[i] >= 97 && (int)str_init[i] <= 122))// Verifies the stream is alphabetical 
          {
            tmp[k] = str_init[i];
            i++;
            k++;
          }
          else if((int)str_init[i] == 32)   //Detect a space as a seperator
          {
            if(i == 0)
              continue;
            tmp[k]='\0';
            strcpy(usernames[j], tmp);
            index++;
            k=0;
            j++;
            i++;
          }
          else    //End of Stream detected
          {
            break;
          }
        }

    }
    if(mode)
      shm_unlink(MEM_NAME_USERS); // Removes the Shared Memory

    /*
    The following code is similar to the above mentioned except it is set 
    for passwords instead of users
    */

    ptr = mmap(0, MEM_SIZE, PROT_READ, MAP_SHARED, shm_fd_pwds, 0);
    
    strcpy(str_init, (char*)ptr);
    
    
    {
      int i =0, j=0, k=0;
        while(1)
        {
          
          if(((int)str_init[i] >= 65 && (int)str_init[i] <= 90) || ((int)str_init[i] >= 97 && (int)str_init[i] <= 122))
          {
            tmp[k] = str_init[i];
            
            i++;
            k++;
          }
          else if((int)str_init[i] == 32)
          {
            if(i == 0)
              continue;
            tmp[k]='\0';
            strcpy(passwords[j], tmp);
            k=0;
            j++;
            i++;
          }
          else
          {
            break;
          }
        }

    }
    if(mode)
      shm_unlink(MEM_NAME_PWDS);
    }
    return index;
  }

void insertIntoSharedMemory(char users[][20], char pwds[][20])
{ 
  /*
  Second Star function for this library which does the opposite of the preInitSharedMemory()
  It takes as argument the users and pwds array which containes usernames and corresponding
  passwords and maps them to the shared memory as a stream of data. These arrays are stored as
  specifies in the preprocessors.
  */
  int shm_fd_users, shm_fd_pwds, i=0;
  char str[50];
  void *ptr;

  shm_fd_users = shm_open(MEM_NAME_USERS, O_CREAT | O_RDWR, 0666);
  shm_fd_pwds = shm_open(MEM_NAME_PWDS, O_CREAT | O_RDWR, 0666);

  ftruncate(shm_fd_users, MEM_SIZE);
  ftruncate(shm_fd_pwds, MEM_SIZE);


  ptr = mmap(0, MEM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd_users, 0);

  for(i=0 ; i<SIZE; i++)
  {
    if(!strlen(users[i]))
      continue;
    strcpy(str , users[i]);
    appendSpace(users[i]);
    sprintf(ptr, "%s ", str);
    ptr+= strlen(users[i]);
  }



  ptr = mmap(0, MEM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd_pwds, 0);
  for(i=0 ; i<SIZE && users[i]!=NULL; i++)
  {
    if(!strlen(pwds[i]))
        continue;
      strcpy(str , pwds[i]);
      appendSpace(pwds[i]);
      sprintf(ptr, "%s ", str);
      ptr+= strlen(pwds[i]);
  }

  return;
}

void printUsers(char users[][20], char passwords[][20])
{
  /*
  Prints Users and Passwords for admin only.
  */
  for(int i=0; i<10; i++)
  { 
    if(!strlen(users[i]))
      break;
    printf("%d\t%s\t%s\n",i, users[i], passwords[i]);
  }
}

bool deleteUser(char user[20], char usernames[][20], char passwords[][20])
{
  /*
  Deletes the Specifies user from the users array. Both are passed as
  arguments to this function.
  */
  for(int i=0; i< 10;++i)
  {
    if(!strlen(usernames[i]))
      continue;
    if(!strcmp(user, usernames[i]) ){
      memset(usernames[i], 0, 20);
      memset(passwords[i], 0, 20);
      return true;
    }
  }
  return false;

}
