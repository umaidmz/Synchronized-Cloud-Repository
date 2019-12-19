#include "admin.h"
#ifndef SIZE
#define SIZE 10
#define MEM_SIZE 4096
#define MEM_NAME_USERS "USER"
#define MEM_NAME_PWDS "PWDS"
#endif

int main(int argc, char const *argv[])
{

	char userAdmin[20];												//Declarations and Initializations of important variables
	char userPwd[20];

	char usernames[SIZE][20] = {{0}};
	char passwords[SIZE][20] = {{0}};
	int index,flag = 0;

	printf("%s\n", "Initializing Shared Memory...");				//Initialzing Shared Memory
	index = preInitSharedMemory(usernames, passwords, 1);
	sleep(1);
	

	printf("%s\n%s\n", "Welcome to Account Creator", "Please verify your credentials: You have 3 tries");


	flag = getAdminCreds(userAdmin, userPwd);						//Verifying the admins authenticity
	if(!flag)
	{
		insertIntoSharedMemory(usernames, passwords);
		printf("Exiting...\n");
		exit(0);
	}

	system("clear");
	printf("S#\tUsers:\tPasswords\n");
																	// Functionality for the admin to manage the users
	printUsers(usernames, passwords);

	char i = NULL;
	int opt = 0;
	printf("(1) Insert User\n");
	printf("(2) Delete User\n");
	printf("(3) Print Users\n");
	printf("(0) Exit\n" );
	scanf("%d", &opt);
	while(opt){
		char user[20] = {0};
		switch(opt)													//Switch case for options
		{
			case 1:	
					if(index > 9)
					{
						printf("You cannot add more users\n");
						break;
					}
					getCreds(usernames[index], passwords[index]);
					printf("%d\n", strlen(usernames[index]) );
					index++;
				break;

			case 2:	
					printf("Enter Username:\n");
					scanf("%s", user);
					if(deleteUser(user, usernames, passwords))
						printf("User Deleted\n");
					else
						printf("User not found\n");
				
				break;

			case 3:	
					printf("S#\tUsers:\tPasswords\n");
					printUsers(usernames, passwords);
				
				break;

			default:

			break;
		}
		printf("(1) Insert User\n");
		printf("(2) Delete User\n");
		printf("(3) Print Users\n");
		printf("(0) Exit\n");
		scanf("%d", &opt);
	}
	
			

	printf("%s\n", "Inserting User into Memory...");					//Writing the users and passwords array to the Shared Memory
	insertIntoSharedMemory(usernames, passwords);
	printf("Restart Server to Update Changes\n");
	sleep(1);


	printf("Exiting...\n");
	return 0;
}