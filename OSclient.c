#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define PORT 8080

int sockfd = 0;

struct user{

    char username[10];
    char pwd[10];
};

int makeFolder(char fname[20]){
    char sysmsg[50];
    printf("here\n");
    sprintf(sysmsg, "mkdir %s", fname);
    system(sysmsg);
    printf("here2\n");
    return 0;
}

void receiveFile(char *file_path)
{
FILE *fp;
int count = 0;
char ch;
int received_correct; // received correct flag
fp = fopen(file_path,"wb");            
int file_length;
int b=0;
if((received_correct=recv(sockfd, &file_length, sizeof(int),0))>0) //receive client
{
printf("The size of file is %d\n",file_length); 
}  
while(count != (file_length))
{
	b += recv(sockfd, &ch , sizeof(char),0); //receive client
	printf("Bytes received %d\n", b);
	fwrite(&ch, sizeof(char),1,fp);
	count++;
}
fclose(fp); // close the file
 return; 
}


void sendFile(char *path)
{
printf("file path %s\n", path); 
FILE *f;  
int file_length = 0;  
f=fopen(path,"rb"); //opening file to send
printf("here\n");
char ch;
int sent_correct; // sent correct flag
int count = 0;
fseek(f, 0, SEEK_END);
file_length = ftell(f);
printf("here\n");
if((sent_correct=send(sockfd, &file_length, sizeof(int),0))>0) //send server
{
	printf("file length sent correctly\n");
}
fseek(f, 0, SEEK_SET);
// sending the file byte by byte
while(count != file_length)			
{
	fread(&ch, sizeof(char),1,f);	//reading from file  
	printf("byte sent %c\n",ch);
	if((sent_correct=send(sockfd, &ch , sizeof(char),0))>0) //send server
	{
	printf("file data sent correctly\n");
	}
	count++;
}   
fclose(f);  // close the file 
printf("The file was sent successfully to the server\n");
 return; 
}
//sync methods here


//check if the file exist
int fileExists(char *path)
{
int file_exists = -1;
if (access(path,F_OK) == -1)
{
printf("File doesnot exist\n");
file_exists = 0;
}
else
{
printf("File exists\n");
file_exists = 1;
}
return file_exists;
}

//get the new files from the server that client does not have
void receiveNewFiles(char *folder_name)
{
int m;
int k;
int sent_correct;
int received_correct;
int size=0;
char var[20]; // file name
while(1)
{
	//printf("hello\n");
	if ((received_correct=recv(sockfd, &size, sizeof(int), 0))>0) // receive file name size
	{
		//printf("file name size %d k %d\n",size,received_correct);
	}
	else
		break;
	if (size == -1)
	{
	printf("Process Done\n");
	break;
	}
	if ((received_correct=recv(sockfd, var, size, 0))>0) // receive file name
	{
	var[size] = '\0';
	printf("file name %s\n",var);
	}

	char path[40]; // Path of the file
	int choice; // whether to update the file or not
	int file_exists =-1;
	int update_file = -1;
	// get the absolute path of the file
	strcpy(path, folder_name);
	strcat(path, "/");
	strcat(path, var);
	printf("Path of file name: %s\n", path);
	//check if the file exists
	file_exists = fileExists(path); // returns 0 if the file doesnot exists else 1
	send(sockfd, &file_exists, sizeof(int), 0); // sending to the client
	if (file_exists == 0)
	{
	printf("\nServer has the new file %s. Get the file?Enter1-Yes,0-No\n", var);
	scanf("%d",&choice);
	if (choice == 1)
	{
	update_file = 1;	
	}
	else if (choice == 0)
	{
	update_file = 0;
	}
	else
	{
	printf("INVALID INPUT\n");
	return;
	}
	send(sockfd, &update_file, sizeof(int), 0); // sending the info to the server
	if (update_file == 1)  // user wants to update the file
	{
	printf("receiving the file\n");
	receiveFile(path);
	}
	else if (update_file == 0) //user does not want the file
	{
	printf("New file not retrieved\n");
	}
	}
}
}

// main method for sync files
void syncFiles(char folder_name[20]){
printf("\nSYNCHRONIZATION\n");
int n = 0;  
int sent_correct; // to check the bytes sent
int received_correct; // to check the bytes received
int dir_exists;
char var[40];
// receive info about the existence of directory
if ((received_correct= recv(sockfd, &dir_exists, sizeof(int), 0))>0)
{
printf("received %d\n", dir_exists);
}
if (dir_exists == 0)
{
printf("Directory doesn't exist\n");
return;
}
else if (dir_exists == 1) // directory exists
{
printf("Directory exists\n");
printf("\nNEW FILES IN THE SERVER \n");
receiveNewFiles(folder_name); // get those files from the server that client doesnot have
printf("\nGET NEW FILE PROCESS DONE\n\n");
//printf("\nhi bro\n");

// get the files in the directory
printf("\nUPDATE FILES EITHER WAYS\n\n");
DIR *d;
struct dirent *dir;
// absolute path to the folder name
char path[100]="/home/muptii/Desktop/semester5 files/operating systems/os_proj/project/client/";
strcat(path, folder_name);
printf("path of folder name: %s\n", path);
d= opendir(path);
if (d)
{
while((dir = readdir(d)) != NULL) // reading the files from the directory
{
	strcpy(var, dir->d_name);
	int trim = strlen(var)-1;
	int size = strlen(var);
	char file_path[100];
	strcpy(file_path, path);
	strcat(file_path,"/");
	strcat(file_path,var);
	if((var[0] != '.') && (var[trim] != '~') )
	{
		printf("file name:%s\n", var);
		if((sent_correct = send(sockfd, &size, sizeof(int), 0))>0)
		{
			//printf("file name size sent correctly\n");
		}
		if((sent_correct = send(sockfd, var, strlen(var), 0))>0)
		{
			//printf("file name sent correctly\n");
		}
		int file_exists=-1;
		int update_file = -1;
		if((received_correct=recv(sockfd, &file_exists, sizeof(int), 0))>0)
		{
			//printf("file_exists value received\n");
		}
		if (file_exists == 0)
		{
			printf("\nServer does not have the file\n");
			sendFile(file_path);
		}
		else if (file_exists == 1)
		{
			printf("Server has the file\n");
			printf("UPDATING FILES\n");
			// get the mod time of file
			struct stat attr;
			stat(file_path, &attr);
			time_t client_mod_time = attr.st_mtime;
			time_t server_mod_time;
			if((received_correct=recv(sockfd, &server_mod_time, sizeof(time_t),0))>0) // receive modification time of the server
			{
			//printf("server mod time value received\n");
			}
			printf("client last modification time %s: %ld\n", var, client_mod_time);
			printf("server last modification time %s: %ld\n", var, server_mod_time);
			if (client_mod_time < server_mod_time) // server file recent
			{
				int choice =-1; // to check if the user wants to update the file or no
				printf("Server file is more recent. Do you want to update?Enter 1-Yes,0-No");
				scanf("%d", &choice);
				if (choice == 0) // donot update
				{
				update_file = 1;
				}
				else if (choice == 1) // update the file
				{
				update_file = 2;
				}
				if ((sent_correct=send(sockfd, &update_file, sizeof(int),0))>0) //send update_file value
				{
				//printf("update_file value sent\n");
				}
				if (update_file == 1)
				{
				printf("\nYou have chosen not to update the older version\n\n");
				}
				else if (update_file == 2)
				{
				printf("\nUpdating your file\n\n");
				receiveFile(file_path);
				}
				 
			}
			else if (client_mod_time>server_mod_time) //client file recent
			{
				update_file = 0;
				if ((sent_correct=send(sockfd, &update_file, sizeof(int),0))>0) //send update_file value
				{
				//printf("update_file value sent\n");
				}
				printf("\nClient file is more recent\n");
                printf("%s\n",file_path);
				sendFile(file_path);
			}
			else
			{
				update_file = 3; // equal time
				printf("\nBoth are equal\n\n");
				if ((sent_correct=send(sockfd, &update_file, sizeof(int),0))>0) //send update_file value
				{
				//printf("update_file value sent\n");
				}
			}
		}
	}
	bzero(var, sizeof(var));
	size = 0;
}
}
}
int val = -1;
if((sent_correct = send(sockfd, &val, sizeof(int), 0))>0)
{
	printf("\nSYNC PROCESS DONE\n\n");
}
}

void uploadFiles(char folderName[20]){
    int sent_correct;
    int ack=0;
    int size =0;
    int received_correct;
    DIR *d;
    struct dirent *dir;
    char var[256];
    char file_path[100];
    char path[100]="/home/muptii/Desktop/semester5 files/operating systems/os_proj/project/client/";
    strcat(path, folderName);
    printf("folder name path%s\n", path);
    d= opendir(path);
    if (d)
    {
        while((dir = readdir(d)) != NULL)
        {

            memset(var,0,sizeof(var));
            memset(file_path,0,sizeof(file_path));
            //printf("%ld", sizeof(dir->d_name))
            strcpy(var, dir->d_name);
            printf("var: %s\n", var);
            int trim = strlen(var)-1;
            size = strlen(var);
            printf("path: %s\n", path);
            
            if((var[0] != '.') && (var[trim] != '~') )
            {
                strcpy(file_path, path);
                printf("path: %s\n", path);
                strcat(file_path,"/");
                strcat(file_path,var);
                printf("var: %s\n", var);
                printf("file path: %s\n", file_path);
                printf("file name:%s\n", var);
                printf("path: %s\n", path);
                if((sent_correct = send(sockfd, &size, sizeof(int), 0))>0)
                {
                    printf("file name size sent correctly\n");
                }
                if((sent_correct = send(sockfd, var, strlen(var), 0))>0)
                {
                    printf("file name sent correctly\n");
                }
                ack=0;
                printf("send the new file to server\n");
                printf("%s\n", file_path);
                sendFile(file_path);
                
            }
        }
        
    }
int val = -1;
if((sent_correct = send(sockfd, &val, sizeof(int), 0))>0)
{
	printf("Done\n");
}
return;
}

void downloadFiles(char folderName[20]){
    int m;
    int k;
    int sent_correct;
    int received_correct;
    int size=0;
    char var[20];
    int ack=0;
    while(1)
    {
        printf("hello\n");
        if ((k=recv(sockfd, &size, sizeof(int), 0))>0) // receive file name size
            printf("file name size %d k %d\n",size,k);
        else{
            break;
            }
        if (size == -1)
        {
        printf("Done\n");
        break;
        }
        if ((m=recv(sockfd, var, size, 0))>0) // receive file name
        {
        var[size] = '\0';
        printf("file name %s\n",var);
        ack =1;
        }
        // path 
        char path[40];
        
        
        
        strcpy(path, folderName);
        strcat(path, "/");
        strcat(path, var);
        printf("file name path %s\n", path);
       /* printf("value of ack %d\n",ack);
        send(sockfd, &ack, sizeof(int), 0); // sending ack to the server
        printf("value of ack %d\n",ack);*/
        printf("receiving the file\n");
	    receiveFile(path);
    } 
    return;   
}


int main(int argc, char *argv[]){
    int msgread;
    struct sockaddr_in server_addr;
    char msgbuffer[1024] = {0};
    bool loggedIn=false;
    char username[10] ={0};
    char pwd[10] ={0};

    sockfd = socket(AF_INET,SOCK_STREAM,0);

    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(PORT); 
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    while(!loggedIn){
        printf("Please login to continue.\n");
        printf("Enter Username: ");
        fgets(username,30,stdin);
        username[strlen(username)-1]='\0';
        printf("\nEnter password: ");
        fgets(pwd,30,stdin);
        pwd[strlen(pwd)-1]='\0';
        struct user login;
        memset(&login,0,sizeof(struct user));
        memcpy(login.username,username,sizeof(username));
        memcpy(login.pwd,pwd,sizeof(pwd));

        printf("\nsending user to server %s:%s\n", login.username, login.pwd);
        send(sockfd, &login, sizeof(struct user), 0);
        //add auth code here and then below code in if statement

        if(loggedIn){
            while(1){

                char cmd[10] = {0};
                char folderName[20]={0};
                char msg_send[50] ={0};
                
                printf("Enter one of the following commands.\n1) ls\n2) upload [folder_name]\n3) download [folder_name]\n4)sync [folder_name]\n5)exit\n");
                fgets(msg_send, 30, stdin);
                //scanf(" %[^\n]%*c", msg_send);
                printf("here msg received %s\n",msg_send);
                int len2 = strlen(msg_send);
                printf("length of command %d\n", len2);
                msg_send[len2-1] = '\0';
                printf("here msg received %s\n",msg_send);
                sscanf(msg_send, "%s %s", cmd, folderName);
                printf("here cmd received %s\n",cmd);
                printf("here fname received %s\n",folderName);
                /*
                if(send(sockfd, msg_send, strlen(msg_send), 0)<0){

                    printf("error sending msg from client");
                    exit(EXIT_FAILURE);
                }
                */
                send(sockfd, msg_send, strlen(msg_send), 0);
                printf("here in client after send\n");
                //commmand sent to server
                if(!strcmp(cmd,"ls")){
                    printf("here in ls condition\n");
                    int n = 0;
                    int len = 0, maxlen = 1000;
                    char buffer[maxlen];
                    char* pbuffer = buffer;
                    // will remain open until the server terminates the connection
                    /*
                    while ((n = recv(sockfd, pbuffer, maxlen, 0)) > 0) {
                        pbuffer += n;
                        maxlen -= n;
                        len += n;
                        buffer[len] = '\0';
                    }
                    */
                    read(sockfd, msgbuffer, 1024);
                    printf("Response from the server of ls command\n");
                    printf("%s\n", msgbuffer);;
                
                }
                else if(!strcmp(cmd,"upload")&& (folderName[0]!='\0'))
                {
                    //upload command code here
                    printf("here in upload\n");
                    //receive msg from server to send files
                    char *res;
                    read(sockfd, res, strlen(res));
                    /*
                    if(res)
                    {
                        //send mul files
                        printf("here in res\n");
                        uploadFiles(folderName);
                        fflush(stdout);
                        fflush(stdin);

                    }
                    else
                    {
                        printf("client folder response error\n");
                    }
                    */
                        uploadFiles(folderName);
                        fflush(stdout);
                        fflush(stdin);
                    
                    
                }
                else if(!strcmp(cmd,"download")&& (folderName[0]!='\0' )){
                    //download command code here
                    printf("here in download\n");
                    makeFolder(folderName);
                    downloadFiles(folderName);
                    //recieve mul files
                    //receiveNewFiles(folderName);


                }
            else if(!strcmp(cmd,"sync") && (folderName[0]!='\0')){
            // sync command code here
            printf("sync files\n");
            syncFiles(folderName);// calling the main sync method
            }
                


                else if(!strcmp(cmd,"exit")){
                    //exit code here
                    printf("exiting...\n");
                
                    close(sockfd);
                    exit(EXIT_SUCCESS);
                    break;
                    
                }



                


            }
        }
    }
    return 0;

       
}
