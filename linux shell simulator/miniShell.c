#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>


char** createStr (char* str, int start, int end);//function receives char* and puts every word in array**, puts NULL in end
int numberOfWords(char* str);//function to check how many words user entered
void freeArray (char** array, int counter);//function to free memory from array**
void sigHandler(int sigNum);//function to apply signal for the father
void * RegularCommand(char * str);//function which receives a regular command and returns the output of the command
void * pipeCommand (char * str, int index);//function which receives command with character | and returns output
void * redirectionCommand (char * str, int index,char angle);//function which receives commands with character > or < and returns output


int main()
{
    signal(SIGCHLD, sigHandler);//if father gets signal from child go to function sidHandler
    signal(SIGTSTP, SIG_IGN);//if receives signal SIGSTSTP ignore it

    char hostname[1024];
    gethostname(hostname,1024);//get host name
    char *name = getlogin();//get user name
    int check = 1,verticalBar =0, RightaAngleBracket = 0,LeftaAngleBracket = 0;
    char angel;
    char *str;
    do
    {
        printf("%s@%s$ ", name, hostname);//print format
        str = (char*)malloc(sizeof(char));//locate memory for user to enter command
        if(str == NULL)//if the malloc failed return null
        {
            printf("cannot allocate memory\n");
            return -1;
        }
        fgets(str, 510, stdin);

        if(str == NULL)
        {
            printf("error, input invalid");
        }

        if(strcmp(str, "done\n")==0)//if user entered done exit
        {
            check = 0;
            exit(0);
        }
        int i;
        for(i=0; i<strlen(str); i++)//check if input contains |
        {
            if(str[i]=='|')
            {
                verticalBar =1;
                break;
            }
            if(str[i] == '<')//read from file
            {
                angel = '<';
                RightaAngleBracket = 1;
                break;
            }
            if(str[i] == '>')//write to file
            {
                angel = '>';
                LeftaAngleBracket=1;
                break;
            }
        }

        if(verticalBar == 1)
        {
            verticalBar = 0;
            pipeCommand(str,i);
        }

        else if(LeftaAngleBracket==1 || RightaAngleBracket==1)
        {
            redirectionCommand(str,i,angel);
            LeftaAngleBracket =0;
            RightaAngleBracket = 0;
        }

        else if(verticalBar == 0)
        {
            RegularCommand(str);
        }

    }while(check == 1);//continue asking for commands until error appears

}

void * redirectionCommand(char * str, int index,char angel)
{
    char *temp1 = (char*) malloc(sizeof(char));//locate memory to copy command
    if(temp1 == NULL)//if the malloc failed return null
    {
        printf("cannot allocate memory\n");
        return NULL;
    }
    temp1 = strcpy(temp1, &str[index+1]);
    int counter;
    char **sentence;
    str[index]='\0';
    char *temp = (char*) malloc(sizeof(char));//locate memory to copy command
    if(temp == NULL)//if the malloc failed return null
    {
        printf("cannot allocate memory\n");
        return NULL;
    }
    temp = strcpy(temp, str);
    counter = numberOfWords(temp);//send to function to receive length
    int fd;
    if(angel == '>')
        fd = open(temp1, O_WRONLY|O_CREAT|O_TRUNC ,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);//open file if exists, if doesnt create and over wright characters.
    else if(angel == '<')
        fd = open(temp1, O_RDONLY , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);//open file if exists. if doesnt create
    if(fd == -1)//if failed opening file
    {
        printf("error opening file\n");
    }
    else {
        pid_t pid;
        pid = fork();
        if (pid < 0)//child failed
        {
            printf("FORK FAILED\n");
            exit(1);
        }
        if (pid == 0)//child
        {
            sentence = createStr(str, 0, counter);//sends to function to put in char **
            if (angel == '>')
                dup2(fd, STDOUT_FILENO);//write to file
            else if (angel == '<')
                dup2(fd, STDIN_FILENO);//write to file
            execvp(sentence[0], sentence);
            if (execvp(sentence[0], sentence) < 0) {//if failed
                for (int i = 0; i < counter; i++) {
                    fprintf(stderr, "%s ", sentence[i]);//print command
                }
                fprintf(stderr, ":command not found\n");
                freeArray(sentence, counter);
                exit(1);
            }
            exit(0);
        }
        if (pid > 0)//father
        {
            wait(NULL);//wait for sun to finish
        }
        close(fd);//close file
        free(temp);
    }
}

void * pipeCommand (char * str, int index)
{

    int pipeFd[2];
    if(pipe(pipeFd)==-1)//if failed creating pipe
    {
        printf("PIPE FAILED\n");
        exit(1);
    }
    pid_t pid1, pid2;
    int counterWord1, counterWord2;
    char **sentence1, **sentence2;

    char *temp = (char*) malloc(sizeof(char));//locate memory to copy command
    if(temp == NULL)//if the malloc failed return null
    {
        printf("cannot allocate memory\n");
        return NULL;
    }
    temp = strcpy(temp, str);

    temp[index]='\0';

    char *temp1 = (char*) malloc(sizeof(char));//locate memory to copy command
    if(temp1 == NULL)//if the malloc failed return null
    {
        printf("cannot allocate memory\n");
        return NULL;
    }
    temp1 = strcpy(temp1, temp);
    counterWord1 = numberOfWords(temp1);


    char *temp2 = (char*) malloc(sizeof(char));//locate memory to copy command
    if(temp2 == NULL)//if the malloc failed return null
    {
        printf("cannot allocate memory\n");
        return NULL;
    }
    temp2 = strcpy(temp2, str);

    char *temp3 = (char*) malloc(sizeof(char));//locate memory to copy command
    if(temp3 == NULL)//if the malloc failed return null
    {
        printf("cannot allocate memory\n");
        return NULL;
    }
    strcpy(temp3, &temp2[index+1]);

    char *temp4 = (char*) malloc(sizeof(char));//locate memory to copy command
    if(temp4 == NULL)//if the malloc failed return null
    {
        printf("cannot allocate memory\n");
        return NULL;
    }
    strcpy(temp4, temp3);

    counterWord2 = numberOfWords(temp3);

    if(index==0 || index == strlen(str)-2)
    {
        str[strcspn(str, "\n")] = 0;
        printf("%s: command not found\n" , str);
    }
    else {
        pid1 = fork();
        if (pid1 < 0)//child failed
        {
            printf("FORK FAILED\n");
            exit(1);
        }
        if (pid1 == 0)//child
        {
            sentence1 = createStr(temp, 0, counterWord1);//send to function to receive string in char**
            close(pipeFd[0]);//close read side
            dup2(pipeFd[1], STDOUT_FILENO);//write to pipe
            execvp(sentence1[0], sentence1);
            if (execvp(sentence1[0], sentence1) < 0) {//if failed
                for (int i = 0; i < counterWord1; i++) {
                    fprintf(stderr, "%s ", sentence1[i]);//print command
                }
                fprintf(stderr, ":command not found\n");
                exit(1);
            }
            freeArray(sentence1, counterWord1);
            exit(0);
        }
        if (pid1 > 0)//father
        {
            pid2 = fork();
            if (pid2 < 0)//child failed
            {
                printf("FORK FAILED");
                exit(1);
            }
            if (pid2 == 0)//child
            {
                sentence2 = createStr(temp4, 0, counterWord2);//send to function to receive string in char**
                close(pipeFd[1]);//close write side
                dup2(pipeFd[0], STDIN_FILENO);//read from pipe
                execvp(sentence2[0], sentence2);
                if (execvp(sentence2[0], sentence2) < 0 ) {//if failed
                    for (int i = 0; i < counterWord2; i++) {
                        fprintf(stderr, "%s ", sentence2[i]);//print command
                    }
                    fprintf(stderr, ":command not found\n");
                    exit(1);
                }
                freeArray(sentence2, counterWord2);
                exit(0);
            }
            if (pid2 > 0)//father
            {
                close(pipeFd[0]);//close read side
                close(pipeFd[1]);//close write side
                wait(NULL);//wait for to sons
                wait(NULL);
            }
        }
        free(temp);
        free(temp1);
        free(temp2);
        free(temp3);
        free(temp4);
    }
}

void * RegularCommand(char * str)
{
    int count;
    char **sentence;
    char *temp = (char*) malloc(sizeof(char));//locate memory to copy command
    if(temp == NULL)//if the malloc failed return null
    {
        printf("cannot allocate memory\n");
        return NULL;
    }
    temp = strcpy(temp, str);

    count = numberOfWords(temp);//send to function that checks how many words entered

    sentence=createStr(str, 0,count);//sends to function to put in array**

    pid_t x = fork();
    if(x == -1)//if the pid failed
    {
        printf("error");
        exit(1);
    }
    if(x == 0)//child
    {
        signal(SIGTSTP, SIG_DFL);
        if(strcmp(str, "bg") == 0)
        {
            kill(-1, SIGCONT);
            exit(1);
        }
        else {
            execvp(sentence[0], sentence);//send to function to deal with command
            if (execvp(sentence[0], sentence) < 0) {//if failed
                for (int i = 0; i < count; i++) {
                    printf("%s ", sentence[i]);//print command
                }
                printf(":command not found\n");
                exit(1);
            }
            freeArray(sentence, count);
        }
    }
    if(x > 0)//parent
    {
        pause();//wait for child to finish
    }
    free(str); //free space in memory
    free(temp); //free space in memory
}

void sigHandler(int sigNum)
{
    waitpid(-1, NULL, WNOHANG);
}


void freeArray (char** array, int counter)
{
    for(int i=0; i<=counter; i++)
    {
        free(array[i]);
    }
    free(array);
}

int numberOfWords(char* str)
{
    //function to check how many words user entered
    int counter=0;
    char* token = strtok(str, " \n");
    while(token!=NULL)//read one word at a time
    {
        ++counter;//add to counter
        token = strtok(NULL, " \n");//promote word
    }
    return counter;
}

char** createStr (char* str, int start, int end)
{
    char **sentence3 = (char**) malloc((end+1) * sizeof(char));//locate memory for array**
    if(sentence3 == NULL)//if the malloc failed return null
    {
        printf("cannot allocate memory\n");
        return NULL;
    }
    int i;
    char* token2 = strtok(str, " \n");
    //read a word at a time
    while(token2!=NULL)
    {
        for(i =start; i<end; i++)
        {
            sentence3[i]=(char*)malloc((strlen(token2)+1) *sizeof(char));//locate memory for array[i]
            if(sentence3[i]==NULL)
            {
                printf("error-malloc failed\n");
                exit(1);
            }
            sentence3[i] = token2;//enter word
            token2 = strtok(NULL, " \n");
        }
    }
    sentence3[end] = NULL;//put NULL in last place in array
    return sentence3;
}


