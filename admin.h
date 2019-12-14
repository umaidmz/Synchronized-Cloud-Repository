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
  printf("%s\n", "Username: ");
  scanf(" %s", user);
  printf("%s\n", "Password: ");
  scanf(" %s", pwd);
  
  return;
}

int getAdminCreds(char* user, char *pwd)
{
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
  strcat(user, " ");
}

int getEmptyIndex(char *array[])
{
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
  char user[20], pwd[20];
  getCreds(user, pwd);
  strcpy(users[index], user);
  //users[index] = user;
  strcpy(pwds[index], pwd);
  //pwds[index] = pwd;
  printf("User Inserted %s %s\n",users[index], pwds[index]);
  return;
}

int preInitSharedMemory(char usernames[][20], char passwords[][20])
{
    int shm_fd_users_init, shm_fd_pwds, index=0;
    void *ptr;
    char str_init[200]={0}, tmp[200];

    shm_fd_users_init = shm_open(MEM_NAME_USERS, O_RDONLY, 0666);
    shm_fd_pwds = shm_open(MEM_NAME_PWDS, O_RDONLY, 0666);
    if(shm_fd_users_init > 0)
    {
    
    ptr = mmap(0, MEM_SIZE, PROT_READ, MAP_SHARED, shm_fd_users_init, 0);
    
    strcpy(str_init, (char*)ptr);
    //printf("%s\n",str_init );
    
    {
      int i =0, j=0, k=0;
        while(1)
        {
          //printf("%c\n", str[i] );
          if(((int)str_init[i] >= 65 && (int)str_init[i] <= 90) || ((int)str_init[i] >= 97 && (int)str_init[i] <= 122))
          {
            tmp[k] = str_init[i];
            //printf("%c\n", tmp[k] );
            i++;
            k++;
          }
          else if((int)str_init[i] == 32)
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
          else
          {
            //printf("%d broken\n", i);
            break;
          }
        }

    }
    shm_unlink(MEM_NAME_USERS);


    ptr = mmap(0, MEM_SIZE, PROT_READ, MAP_SHARED, shm_fd_pwds, 0);
    
    strcpy(str_init, (char*)ptr);
    //printf("%s\n",str_init );
    
    {
      int i =0, j=0, k=0;
        while(1)
        {
          //printf("%c\n", str[i] );
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
    shm_unlink(MEM_NAME_PWDS);
    }
    return index;
  }

void insertIntoSharedMemory(char users[][20], char pwds[][20])
{
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
  for(int i=0; i<10; i++)
  { 
    if(!strlen(users[i]))
      break;
    printf("%d\t%s\t\n",i, users[i], passwords[i]);
  }
}

bool deleteUser(char user[20], char usernames[][20])
{
  for(int i=0; i< 10;++i)
  {
    if(!strlen(usernames[i]))
      continue;
    if(!strcmp(user, usernames[i]) ){
      memset(usernames[i], 0, 20);
      return true;
    }
  }
  return false;

}
