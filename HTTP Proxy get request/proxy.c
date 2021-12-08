#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

/*
 * function to free allocated memory if project failed for any reason
 * recieves every string that was allocated and rees it
 */
void ErrorFree(int flagPage, char*page, char* ip, char* temp, char* request)
{
    if(flagPage ==1)
        free(page);
    free(ip);
    free(temp);
    free(request);
    exit(EXIT_FAILURE);
}
int isFile(const char* name)
{
    DIR* directory = opendir(name);

    if(directory != NULL)
    {
        closedir(directory);
        return 0;
    }
    if(errno == ENOTDIR)
    {
        return 1;
    }
    return -1;
}
/*This function receives an ip address, the file stream to keep the data in. the File it is opening and the URl
 *returns a pointer to the file to keep the data that came back from server
 *checks if the given URL contains a path. if not it will open a file "index.html" under the folder of the ip
 *otherwise it will open a folder with every part of the path and take the last one to be the file name
 */
FILE* createFolder(char* ip, char* page, FILE* file,char* text)
{
    int i,counter=0;
    char *help = NULL;
    for(i=7; i< strlen(text); i++)//count how many paths there are
    {
        if(text[i]=='/')
        {
            counter++;
        }
    }
     char *makeFolder =(char*) malloc((strlen(ip)+1)*sizeof (char));//locate place in memory for file path
    if(makeFolder == NULL)
    {
        perror("malloc failed\n");
        exit(EXIT_FAILURE);
    }
    strcpy(makeFolder,ip);
    makeFolder[strlen(ip)]='\0';
    mkdir(makeFolder, 0777);//open folder under the name of the ip
    if(strcmp(page,"/")==0)//no path
    {
        makeFolder =(char*) realloc(makeFolder,sizeof (char)*(strlen(ip)+ strlen(page)+1+11));//locate place in memory for file path
        if(makeFolder == NULL)
        {
            perror("malloc failed\n");
            exit(EXIT_FAILURE);
        }
        strcat(makeFolder,"/index.html");//open file with name "index.html" under ip folder
        file = fopen(makeFolder,"w+");
        if(file == NULL)
        {
            perror("open\n");
            exit(1);
        }
    }
    else
    {
        help = (char*)malloc((strlen(text) - 7+1)*sizeof (char ));//locate place for help array to contain all path
        if(help == NULL)
        {
            perror("malloc failed\n");
            exit(EXIT_FAILURE);
        }
        help[strlen(text) - 7]='\0';
        strncpy(help, &text[7 + strlen(ip)], strlen(text) - 7);
        char *ptr = strtok(help, "/");//use strtok to split file according to the path given
        while (ptr != NULL)
        {
            counter--;
            makeFolder = realloc(makeFolder,(strlen(makeFolder)+1+strlen(ptr)+1));
            strcat(makeFolder, "/");
            strcat(makeFolder, ptr);
            if (counter == 0)
            {
                file = fopen(makeFolder, "w+");
                break;
            }
            mkdir(makeFolder, 0777);//make folder with new path added
            ptr = strtok(NULL, "/");
        }
        free(help);
    }
    free(makeFolder);
    return file;
}

int main(int argc, char* argv[]) {
    if(argc !=2)//if user didnt enter a URL print usage
    {
        printf("Usage: http://Host[:port]/Filepath\n");
        exit(0);
    }
    char* text = argv[1];
    if(text[strlen(text)-1]=='/')//take off last character if contains /
    {
        text[strlen(text)-1] = '\0';
    }
    char *ip = NULL;
    int port = 80;
    char* page = NULL;
    int i, flagIp=0,flagPage=0;
    for(i=7; i<strlen(text); i++)
    {
        if(text[i] == ':')//have port
        {
            ip = malloc((i-7+1)*sizeof (char));
            if(ip == NULL)
            {
                perror("malloc failed\n");
                exit(EXIT_FAILURE);
            }

            int x;
            for(x=i+1; x<strlen(text);x++)
            {
                if(text[x]=='/')//path+port+host
                {
                    page= malloc((strlen(text)-x+1)*sizeof (char));
                    if(page == NULL)
                    {
                        perror("malloc failed\n");
                        ErrorFree(flagPage, NULL, ip, NULL, NULL);
                    }
                    sscanf(text, "http://%99[^:]:%i/%199[^\n]", ip, &port, page);
                    flagIp=1;
                    flagPage=1;
                    break;
                }
            }
            if(flagPage == 0)//have host and port
            {
                sscanf(text, "http://%99[^:]:%i[^\n]", ip, &port);
                page = "/";
                flagIp=1;
                break;
            }
        }
        if(text[i] == '/' && flagIp==0)//have page and host
        {
            ip = malloc((i-7+1)*sizeof (char));
            if(ip == NULL)
            {
                perror("malloc failed\n");
                exit(EXIT_FAILURE);
            }
            page= malloc((strlen(text)-i+1)*sizeof (char ));
            if(page == NULL)
            {
                perror("malloc failed\n");
                ErrorFree(flagPage, NULL, ip, NULL, NULL);
            }
            sscanf(text, "http://%99[^/]/%199[^\n]", ip, page);
            flagIp=1;
            flagPage=1;
            break;
        }
    }
    if(flagIp == 0)//only entered host
    {
        ip = (char*)malloc((strlen(text)-7+1)*sizeof(char ));
        if(ip == NULL)
        {
            perror("malloc\n");
            exit(EXIT_FAILURE);
        }
        sscanf(text, "http://%99[^\n]", ip);
        page = "/";
    }
    char *temp = (char*)malloc((strlen(ip)+1*sizeof (char )));//locate memory for temp array to contain path
    if(temp == NULL)
    {
        perror("malloc\n");
        ErrorFree(flagPage, page, ip, NULL, NULL);
    }
    FILE *file = NULL;
    temp[strlen(ip)]='\0';
    strcpy(temp,ip);
    if(strcmp(page,"/")!=0)
    {
        temp = realloc(temp,((strlen(ip)+1+1+ strlen(page))*sizeof (char )) );
        if(temp == NULL)
        {
            perror("malloc\n");
            ErrorFree(flagPage, page, ip, NULL, NULL);
        }
        temp[strlen(ip)+1+ strlen(page)]='\0';
        strcat(temp,"/");
        strcat(temp,page);
    }
    else
    {
        temp = realloc(temp,((strlen(ip)+1+11)*sizeof (char )));
        if(temp == NULL)
        {
            perror("malloc\n");
            ErrorFree(flagPage, page, ip, NULL, NULL);
        }
        strcat(temp,"/index.html");
    }
    int flag =isFile(temp);
    if(access(temp,F_OK)==0 && flag==1)//check if file exists print to screen file content
    {
        file = fopen(temp,"r");
        if(file == NULL)
        {
            perror("fopen\n");
            ErrorFree(flagPage, page, ip, temp, NULL);
        }
        int lengthRec=0;
        printf("File is given from local filesystem\n");
        printf("HTTP/1.0 200 OK\n");
        fseek(file, 0L, SEEK_END);
        lengthRec = ftell(file);//get the size of the file (bytes)
        fseek(file, 0L, SEEK_SET);
        printf("Content-Length: %d\r\n\r\n",lengthRec);
        char read[1000];
        while(fgets(read, 999, file)>0)
        {
            printf("%s", read);
        }
        printf("\n Total response bytes: %ld\n", lengthRec+37+sizeof(lengthRec));
        fclose(file);
    }
    else//if file doesnt exist
    {
        char *request = malloc((strlen(page)+ strlen(ip)+26+1)*sizeof (char ));//locate memory for request array
        if(request == NULL)
        {
            perror("malloc\n");
            ErrorFree(flagPage, page, ip, temp, NULL);
        }
        if(strcmp(page,"/")==0)
            sprintf(request,"GET %s HTTP/1.0\r\nHost: %s\r\n\r\n",page,ip);//make request in the right format
        else
            sprintf(request,"GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n",page,ip);//make request in the right format
        request[strlen(page)+ strlen(ip)+26]='\0';
        printf("HTTP request =\n%s\nLEN = %ld\n", request, strlen(request));
        struct hostent *server;
        struct sockaddr_in serv_addr;

        int sockfd = socket(AF_INET, SOCK_STREAM, 0);//create socket for client to connect
        if (sockfd == -1)
        {
            perror("socket\n");
            ErrorFree(flagPage, page, ip, temp, request);
        }
        server = gethostbyname(ip);//connect to the id server
        if (server == NULL)
        {
            herror("gethostbyname\n");
            ErrorFree(flagPage, page, ip, temp, request);
        }
        serv_addr.sin_family = AF_INET;
        bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
        serv_addr.sin_port = htons(port);

        int rc = connect(sockfd, (const struct sockaddr *) &serv_addr, sizeof(serv_addr));//connect client to server
        if (rc == -1)
        {
            perror("connect\n");
            ErrorFree(flagPage, page, ip, temp, request);
        }
        char recvline[1000];
        rc = write(sockfd, request, strlen(request));//get answer from server
        if (rc < 0)
        {
            perror("write\n");
            ErrorFree(flagPage, page, ip, temp, request);
        }
        int lengthRec =0;
        rc=read(sockfd, recvline, 15);
        recvline[rc]='\0';
        int flagFile =0;
        if(strcmp(recvline,"HTTP/1.0 200 OK") == 0)//check if the user entered a valid URL with server answer 200 ok
        {
            flagFile=1;
            file = createFolder(ip,page,file,text);
        }
        printf("%s", recvline);
        lengthRec+= strlen(recvline);
        const char* pattern = "\r\n\r\n";
        const char* patp = pattern;

       while(( rc = read(sockfd, recvline, 999))>0)
       {
           recvline[rc] = '\0';
           printf("%s", recvline);
           lengthRec+= strlen(recvline);
           if(flagFile == 1)//print to file
           {
               for (i = 0; i < rc; i++)
               {
                   if (*patp == 0)//delete from file the headers
                   {
                       fwrite(recvline + i, 1, rc - i, file);
                       break;
                   }
                   else if (recvline[i] == *patp) ++patp;
                   else patp = pattern;
               }
           }

       }
        if (rc < 0)
        {
            perror("read\n");
            ErrorFree(flagPage, page, ip, temp, request);
        }
        printf("\n Total response bytes: %d\n", lengthRec);
        if(flagFile == 1)
            fclose(file);
        close(sockfd);
        free(request);
    }
    if(flagPage ==1)
        free(page);
    free(ip);
    free(temp);
    return 0;
}

