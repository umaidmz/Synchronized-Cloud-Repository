#include "admin.h"
#ifndef SIZE
#define SIZE 10
#define MEM_SIZE 4096
#define MEM_NAME_USERS "USER"
#define MEM_NAME_PWDS "PWDS"
#endif

int main(int argc, char const *argv[])
{

	char userAdmin[20];
	char userPwd[20];

	char usernames[SIZE][20] = {{0}};
	char passwords[SIZE][20] = {{0}};
	int index,flag = 0;
	printf("%s\n", "Initializing Shared Memory...");
	index = preInitSharedMemory(usernames, passwords);
	sleep(1);
	

	printf("%s\n%s\n", "Welcome to Account Creator", "Please verify your credentials: You have 3 tries");


	flag = getAdminCreds(userAdmin, userPwd);
	if(!flag)
	{
		insertIntoSharedMemory(usernames, passwords);
		printf("Exiting...\n");
		exit(0);
	}

	system("clear");
	printf("%s\n", "Users: " );

	printUsers(usernames, passwords);

	char i = NULL;
	printf("Do you want to insert User? (y/n)\n");
	scanf(" %c", &i);
	while( i!='n')
	{	
		printf("%d\n", index);
		if(index > 9)
		{
			printf("You cannot add more users\n");
			break;
		}
		getCreds(usernames[index], passwords[index]);
		printf("%d\n", strlen(usernames[index]) );
		index++;
		printf("Do you want to insert User? (y/n)\n");
		scanf(" %c", &i);
	}

	printf("%s\n", "Inserting User into Memory...");
	insertIntoSharedMemory(usernames, passwords);
	sleep(1);


	printf("Exiting...\n");
	return 0;
}