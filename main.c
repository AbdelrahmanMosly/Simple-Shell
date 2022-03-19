#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

const char *builtInCMD[]={
        "cd",
        "echo",
        "export",
        "exit",
        "myVars"
};
int builtInLength = sizeof(builtInCMD) / sizeof(char *);

struct Node {
    char* key;
    char** data;
    struct Node* next;
};
void insert(struct Node** head,char*key ,char** newData) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    struct Node* last = *head;

    newNode->data = newData;
    newNode->key =key;
    newNode->next = NULL;

    if (*head == NULL) {
        *head = newNode;
        return;
    }
    if(strcmp(last->key,key)==0){
        last->data=newData;
        return;
    }
    int found=0;
    while (last->next != NULL && !found) {
        if(strcmp (last->key,key) == 0){
            last->data=newData;
            found=1;
        }
        last = last->next;
    }
    if(!found) {
        if(strcmp(last->key,key)==0){
            last->data=newData;
            return;
        }
        last->next = newNode;
    }else
        free(newNode);

    return;
}
char** find(struct Node** head,char* key){
    struct Node* current = *head;

    while (current != NULL) {
        if (strcmp(current->key,key)==0)
            return current->data;
        current = current->next;
    }
    return "-1";
}
struct Node* head=NULL;

char** checkForVars (char** command){
    int i=0;
    int k=0;
    int kIncreased=0;
    char **args = malloc(64* sizeof(char*));
    args[0]= malloc(sizeof (char )*100);
    while(command[i]!=NULL) {
        int len= strlen(command[i]);
        char* data;

        for (int j = 0; j < len; j++) {
            char temp = command[i][j];
            if(temp=='$')
            {
                j++;
                char* var= malloc(sizeof (char)*100);
                for(;j<len && command[i][j]!='$';j++) {
                    strncat(var, &command[i][j],1);
                }
                data = find(&head,var);
                if(strcmp(data,"-1")) {
                    data = strtok(data, " ");
                    while (data != NULL) {
                        args[k] = data;
                        k++;
                        data = strtok(NULL, " ");
                        kIncreased=1;
                    }
                }
                else {
                    printf("Error undefined variable\ncheck myVars to see defined vars\n");
                    return -1;
                }
                if(command[i][j]=='$') {
                    args[k] = malloc(sizeof(char) * 100);
                    j--;
                }
            }else {
                strncat(args[k], &temp, 1);
            }
        }
        if(!kIncreased) {
            k++;
            args[k]= malloc(sizeof (char )*100);
        }
        else
            kIncreased=0;
        i++;
    }
    args[k]= NULL;
    return args;
}


void executeCommnad(char** command){
    int childID=fork();
    int status;
    if(childID==0)
    {
        execvp(command[0],command );
        printf("Error\n");
        exit(1);
    }
    else{
        do {
            waitpid(childID, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}


char **parseInput(char *input)
{
    int buffer_size = 64;
    int i = 0;
    char *arg;
    char **args = malloc(buffer_size * sizeof(char*));

    arg = strtok(input, " \t\r\n\a");
    while (arg != NULL) {
        args[i] = arg;
        i++;
        arg = strtok(NULL, " \t\r\n\a");
    }

    args[i] = NULL;
    return args;
}

char *readInput(void)
{
    int bufsize = 1024;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;
    int i = 0;

    while ( c != '\n' ) {
        c = getchar();
        buffer[i] = c;
        i++;
    }

    return buffer;
}
int isBuiltIn(char **args) {
    int i;

    if (args[0] == NULL) {
        return 0;
    }
    for (i = 0; i < builtInLength; i++) {
        if (strcmp(args[0], builtInCMD[i]) == 0) {
            return 1;
        }
    }
    return 0;
}
void printList(struct Node* node) {
    while (node != NULL) {
        printf(" %s =", node->key);
        puts( node->data);
        node = node->next;
    }
}
int exitCMD(){
    return 0;
}
int cd(char** command) {
    chdir(command[1]);
    return 1;
}

int echo(char** command){
    if(command[1]==NULL) {
        printf("Error\n");
        return -4;
    }
    char* var= malloc(sizeof (char)*100);
    char* str= malloc(sizeof(char) * 1024);
    int i=1;
    while(command[i]!=NULL) {
        int len= strlen(command[i]);
        char* data;
        for (int j = 0; j < len; j++) {
                char temp = command[i][j];
                if(temp=='\"')
                    continue;
                else {
                    strncat(str, &temp, 1);
                }
            }
            strncat(str, " ", 1);
        i++;
    }
    printf(" %s \n",str);
    return 1;
}

int export(char** command){
    if(command[1]==NULL) {
        printf("Error\n");
        return -4;
    }
    char* str= malloc(sizeof(char) * 1024);
    int i=1;
    char* token;
    token = strtok(command[1],"=\"");
    char* var=malloc(sizeof (char)*100);
    strcpy(var,token);
    token = strtok(NULL,"=\"");
    if(token==NULL || var == NULL) {
        printf("Error \n");
        return -2;
    }
    strcpy(command[1],token);

    int closeQuoteFound=0;
    while(command[i]!=NULL) {
        if(closeQuoteFound) {
            printf("Error");
            return -3;
        }
        int len= strlen(command[i]);;
        int j;
        for (j = 0; j < len; j++) {
            char temp = command[i][j];
            if(closeQuoteFound) {
                printf("Error");
                return -3;
            }
            if(temp=='\"') {
                closeQuoteFound = 1;
                continue;
            }
            strncat(str, &temp, 1);

        }
        strncat(str, " ", 1);
        i++;
    }
    insert(&head,var,str);
    return 1;

}
int executeShellBuiltIn(char** command){


    if (strcmp(command[0], builtInCMD[0]) == 0) {
        return  cd(command);
    }

    if (strcmp(command[0], builtInCMD[1]) == 0) {
        return  echo(command);
    }

    if (strcmp(command[0], builtInCMD[2]) == 0) {
        return export(command);
    }

    if (strcmp(command[0], builtInCMD[3]) == 0) {
        return exitCMD();
    }
    if (strcmp(command[0], builtInCMD[4]) == 0) {
        printList(head);
        return 1;
    }
}


int main() {
    char **command;
    int runningFlag=1;
    while (runningFlag){
        command= parseInput(readInput());
        command= checkForVars(command);
        if(command!=-1 &&command[0]!=NULL) {
            if(isBuiltIn(command)) {
                runningFlag= executeShellBuiltIn(command);
            }
            else {
                executeCommnad(command);
             }
        }
    }
    return 0;
}
