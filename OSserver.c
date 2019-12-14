#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

#define PORT 8080
int new_socket;

struct user{

    char username[10];
    char pwd[10];
};

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
if((sent_correct=send(new_socket, &file_length, sizeof(int),0))>0) //send server
{
	printf("file length sent correctly\n");
}
fseek(f, 0, SEEK_SET);
// sending the file byte by byte
while(count != file_length)			
{
	fread(&ch, sizeof(char),1,f);	//reading from file  
	printf("byte sent %c\n",ch);
	if((sent_correct=send(new_socket, &ch , sizeof(char),0))>0) //send server
	{
	printf("file data sent correctly\n");
	}
	count++;
}   
fclose(f);  // close the file 
printf("The file was sent successfully to the client\n");
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
if((received_correct=recv(new_socket, &file_length, sizeof(int),0))>0) //receive client
{
printf("The size of file is %d\n",file_length); 
}  
while(count != (file_length))
{
	b += recv(new_socket, &ch , sizeof(char),0); //receive client
	printf("Bytes received %d\n", b);
	fwrite(&ch, sizeof(char),1,fp);
	count++;
}
fclose(fp); // close the file
}


int ls(FILE *f) 
{ 
	struct dirent **dirent; int n = 0;

	if ((n = scandir(".", &dirent, NULL, alphasort)) < 0) { 
		perror("Scanerror"); 
		return -1; 
	}
        
	while (n--) {
		fprintf(f, "%s\n", dirent[n]->d_name);	
		free(dirent[n]); 
	}
	
	free(dirent); 
	return 0; 
} 
//sync methods here
//check if the directory exists
int directoryExists(char *folder_name)
{
int dir_exists = -1;
DIR* dir = opendir(folder_name);
if (dir)
{
printf("directory exists\n");
dir_exists = 1;
}
else if (ENOENT == errno)
{
printf("directory does not exist\n");
dir_exists = 0;
}
else
{
printf("failed for some other reason\n");
}
return dir_exists;
}
// check if the file exists
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
// initially server offers and send the new files to the client
void sendNewFiles(char *folder_name)
{
int sent_correct;
int received_correct;
DIR *d;
struct dirent *dir;
char var[20];// file name
//absolute path of the folder
char path[100]="/home/muptii/Desktop/semester5 files/operating systems/os_proj/project/server/";
strcat(path, folder_name);
printf("folder name path%s\n", path);
d= opendir(path);
if (d)
{
while((dir = readdir(d)) != NULL) // get the files from the directory
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
		if((sent_correct = send(new_socket, &size, sizeof(int), 0))>0)
		{
			//printf("file name size sent correctly\n");
		}
		if((sent_correct = send(new_socket, var, strlen(var), 0))>0)
		{
			//printf("file name sent correctly\n");
		}
		int update_file = -1;
		int file_exists = -1;
		if((received_correct=recv(new_socket, &file_exists, sizeof(int), 0))>0)
		{
			//printf("file_exists value received\n");
		}
		if (file_exists == 0) // file doesnot exist
		{
			if((received_correct=recv(new_socket, &update_file, sizeof(int), 0))>0)
		{
			//printf("file_update value received\n");
		}
		if(update_file == 1)
		{
			printf("\nSending the new file to the client\n");
			sendFile(file_path);
		}
		}
		
	}
}
}
int val = -1;
if((sent_correct = send(new_socket, &val, sizeof(int), 0))>0)
{
	printf("\n\nSENDING NEW FILES PROCESS DONE\n\n");
}
}

//main synchronization method
void syncFiles(char folder_name[20]){
printf("\nSYNCHRONIZATION\n");

int dir_exists = -1;
int breakcheck=0;
//checking if the directory exists
dir_exists = directoryExists(folder_name);
send(new_socket, &dir_exists, sizeof(int), 0); // sending to the server
if (dir_exists==1)
{
// sending those files that client does not have
sendNewFiles(folder_name);
printf("\nUPDATE FILES EITHER WAY\n\n");
int m;
int k;
int sent_correct; // to check the bytes sent
int received_correct; // to check the bytes received
int size=0;
char var[20];
while(1)
{
	//printf("hi\n");
	if ((received_correct=recv(new_socket, &size, sizeof(int), 0))>0) // receive file name size
	{
		//printf("file name size %d k %d\n",size,k);
	}
	else
		break;
	if (size == -1)
	{
	printf("SYNC PROCESS DONE\n\n");
	break;
	}
	if ((received_correct=recv(new_socket, var, size, 0))>0) // receive file name
	{
	var[size] = '\0';
	printf("\n\nReceiving new file from the client\n");
	printf("file name %s\n",var);
	}
	// path 
	char path[40];
	int file_exists =-1;
	int update_file = -1;
	// get the file path
	strcpy(path, folder_name);
	strcat(path, "/");
	strcat(path, var);
	printf("Path of file %s\n", path);
	//check if the file exists
	file_exists = fileExists(path);	
	send(new_socket, &file_exists, sizeof(int), 0); // sending the info to the client
	if (file_exists == 0) // file doesnot exist
	{
		printf("\nRECEIVING FILE FROM THE CLIENT\n");
		receiveFile(path);
	}
	else if (file_exists == 1) //file exists
	{
		//printf("Hello\n");
		// get the modification time of the file
		struct stat attr;
		stat(path, &attr);
		time_t mod_time = attr.st_mtime;
		if ((sent_correct=send(new_socket, &mod_time, sizeof(time_t), 0))>0) //send modtime to the client
		{
			//printf("mod time sent\n");
		}
		//receiving updtation detail
		if((received_correct=recv(new_socket, &update_file, sizeof(int),0))>0) // receive
		{
			//printf("mod time received\n");
		}
		if (update_file ==0)//receive the recent file
		{
			printf("\nRECEIVING RECENT FILE FROM THE CLIENT\n");
			receiveFile(path);
		}
		if (update_file == 1)// client has the older version
		{
			printf("\nClient has decided to keep the older version\n");
		}
		else if (update_file == 2)// send the recent file
		{
			printf("\nSending the recent file to the client\n");
			sendFile(path);
		}
	}
	//bzero(var, sizeof(var));
	//size = 0;
}
}

}

int makeFolder(char fname[20]){
    char sysmsg[50];
    printf("here\n");
    sprintf(sysmsg, "mkdir %s", fname);
    system(sysmsg);
    printf("here2\n");
    return 0;
}


void downloadFiles(char folderName[20])
{
    int sent_correct;
    int ack=0;
    int size =0;
    int received_correct;
    DIR *d;
    struct dirent *dir;
    char var[256];
    char file_path[100];
    char path[100]="/home/muptii/Desktop/semester5 files/operating systems/os_proj/project/server/";
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
                if((sent_correct = send(new_socket, &size, sizeof(int), 0))>0)
                {
                    printf("file name size sent correctly\n");
                }
                if((sent_correct = send(new_socket, var, strlen(var), 0))>0)
                {
                    printf("file name sent correctly\n");
                }
                ack=0;
                /*if((received_correct=recv(new_socket, &ack, sizeof(int), 0))>0) //recieve ack from server
                {
                    printf("file_ack value received: %d\n", ack);
                    printf("send the new file to server\n");
                    printf("%s\n", file_path);
                    sendFile(file_path);
                }*/
                printf("send the new file to server\n");
                printf("%s\n", file_path);
                sendFile(file_path);
                
            }
        }
        
    }
int val = -1;
if((sent_correct = send(new_socket, &val, sizeof(int), 0))>0)
{
	printf("Done\n");
}
return;
}

void uploadFiles(char folderName[20])
{
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
        if ((k=recv(new_socket, &size, sizeof(int), 0))>0) // receive file name size
            printf("file name size %d k %d\n",size,k);
        else{
            break;
            }
        if (size == -1)
        {
        printf("Done\n");
        break;
        }
        if ((m=recv(new_socket, var, size, 0))>0) // receive file name
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

int main(int argc, char const *argv[]){
    int server_fd,  msgread;
    int opt = 1;
    struct sockaddr_in server_addr, client_addr;
    socklen_t cliAddr_len;
    int addrlen = sizeof(server_addr);
    char msgbuffer[1024] = {0};
    
    
    
    server_fd = socket(AF_INET,SOCK_STREAM,0);

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons( PORT );

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(server_fd, 3);
    cliAddr_len = sizeof(client_addr);
    while(1){

        bool breaking=false;
        FILE* fp;
        bool loggedIn=false;
        printf("waiting for client...\n");
        new_socket = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&cliAddr_len);
        printf("client connected\n");
        struct user login;
        memset(&login, 0, sizeof(struct user));
        printf("receiving user credentials...\n");
        msgread = read (new_socket, &login, sizeof(struct user));
        printf("received user credentials %s:%s\n", login.username, login.pwd);
        //add auth code here and then below code in if statement

        loggedIn=true;
        send(new_socket,&loggedIn,sizeof(bool),0);
        if(loggedIn){
            while(!breaking){
                char msg_rcv[50] = {0};
                char cmd_rcv[10] = {0};
                char fname_rcv[20] = {0};
                memset(msg_rcv, 0, sizeof(msg_rcv));
                memset(cmd_rcv, 0, sizeof(cmd_rcv));
                memset(fname_rcv,0,sizeof(fname_rcv));
                int len = strlen(msg_rcv);
                msgread = read (new_socket, msg_rcv, 50);
                //command received
                printf("command received from client: %s\n", msg_rcv);
                int len2 = strlen(msg_rcv);
                printf("length of command %d\n", len2);
                msg_rcv[len2] = '\0';
                sscanf(msg_rcv, "%s %s", cmd_rcv, fname_rcv);
                printf("here cmd received %s\n",cmd_rcv);
                printf("here fname received %s\n",fname_rcv);


                if((strcmp(cmd_rcv,"ls"))==0)
                {
                    //ls code here
                    printf("received command: %s\n", cmd_rcv);	
                    printf("running %s in the c program\n", cmd_rcv);
                    char file_entry[200];
                    fp = fopen("a.log", "wb");
                    
                    if(ls(fp) == -1){
                        printf("error in ls command\n ");
                        exit(EXIT_FAILURE);
                    }
                    fclose(fp);
                    fp = fopen("a.log", "rb");
                    int fSize = fread(file_entry, 1, 200, fp);
                    printf("File size is %d %ld", fSize, strlen(file_entry));
             
                    send(new_socket,file_entry,fSize,0);
                    printf("file entry sent %s\n", file_entry);
                    remove("a.log");
                    fclose(fp);
                    memset(msg_rcv, 0, sizeof(msg_rcv));
                    memset(cmd_rcv, 0, sizeof(cmd_rcv));
                    memset(fname_rcv,0,sizeof(fname_rcv));
                    continue;
                        
                }
                printf("here fname received %s\n",fname_rcv);
                printf("here cmd received %s\n",cmd_rcv);

                if((!strcmp(cmd_rcv,"upload"))&&(fname_rcv[0]!='\0')){
                    printf("1here\n");
                    printf("here in upload\n");
                    int len2 = strlen(fname_rcv);
                    fname_rcv[len2]='\0';
                    printf("making directory %s in server with length %d.\n",fname_rcv, len2);

                    makeFolder(fname_rcv);
                    fflush(stdout);
                    printf("here\n");
                    //add code for recieving  mul files
                    uploadFiles(fname_rcv);


                    memset(msg_rcv, 0, sizeof(msg_rcv));
                    memset(cmd_rcv, 0, sizeof(cmd_rcv));
                    memset(fname_rcv,0,sizeof(fname_rcv));
                    continue;
                }

                
                if((!strcmp(cmd_rcv,"download"))&&(fname_rcv[0]!='\0'))
                {
                    //download command code here
                    printf("here in download\n");

                    //send mul files
                    downloadFiles(fname_rcv);
                    
                    memset(msg_rcv, 0, sizeof(msg_rcv));
                    memset(cmd_rcv, 0, sizeof(cmd_rcv));
                    memset(fname_rcv,0,sizeof(fname_rcv));
                    continue;
                }
                if((!strcmp(cmd_rcv,"sync"))&&(fname_rcv[0]!='\0'))
                {
                // sync command code here
                printf("sync files in folder %s\n", fname_rcv);
                syncFiles(fname_rcv); // calling the main sync method
                memset(msg_rcv, 0, sizeof(msg_rcv));
                memset(cmd_rcv, 0, sizeof(cmd_rcv));
                memset(fname_rcv,0,sizeof(fname_rcv));
                continue;

                }
                if(!strcmp(cmd_rcv,"exit"))
                {
                    //exit code here
                    memset(msg_rcv, 0, sizeof(msg_rcv));
                    memset(cmd_rcv, 0, sizeof(cmd_rcv));
                    memset(fname_rcv,0,sizeof(fname_rcv));
                    
                    printf("exiting...\nclient disconnected.\n");
                    close(new_socket);
                    breaking=true;
                    
                }
                else{
                    printf("do nothing");
                    exit(EXIT_FAILURE);
                }
                
            }
        }
    }
    
    return 0;


}
