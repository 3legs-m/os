#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <dirent.h>             

#define IN 1
#define OUT 0
#define MAX_CMD 10
#define BUFFSIZE 100
#define MAX_CMD_LEN 100


int argc;                                       // Effective number of parameters
char* argv[MAX_CMD];                            // Parameter array
char command[MAX_CMD][MAX_CMD_LEN];             // Parameter array
char buf[BUFFSIZE];                             // Accept an array of input parameters
char backupBuf[BUFFSIZE];                       // Backup of parameter arrays
char curPath[BUFFSIZE];                         // shell path
int i, j;                                       
int commandNum;                                 
char history[MAX_CMD][BUFFSIZE];                // Save history command


int get_input(char buf[]);                      // Enter the command and store it in the buf array
void parse(char* buf);                          // Parsing strings
void do_cmd(int argc, char* argv[]);
    int callCd(int argc);                       
    int printHistory(char command[MAX_CMD][MAX_CMD_LEN]);   // Print history command
    // Redirect command
    int commandWithOutputRedi(char buf[BUFFSIZE]);          // Perform output redirection
    int commandWithInputRedi(char buf[BUFFSIZE]);           // Execute the input redirect command
    int commandWithReOutputRedi(char buf[BUFFSIZE]);        // Execute output redirect append write
    int commandWithPipe(char buf[BUFFSIZE]);                // Execute pipeline commands
    int commandInBackground(char buf[BUFFSIZE]);
    void myTop();                                           // Execute the mytop command


/* Function Definition */
/* get_input takes the input characters and stores them in the buf array */
int get_input(char buf[]) {
    // buf array initialization
    memset(buf, 0x00, BUFFSIZE);
    memset(backupBuf, 0x00, BUFFSIZE);        

    fgets(buf, BUFFSIZE, stdin);
    // Remove the \n character at the end of fgets
    buf[strlen(buf) - 1] = '\0';
    return strlen(buf);
}

void parse(char* buf) {
    // Remove the \n character at the end of fgets
    for (i = 0; i < MAX_CMD; i++) {
        argv[i] = NULL;
        for (j = 0; j < MAX_CMD_LEN; j++)
            command[i][j] = '\0';
    }
    argc = 0;
    // The following operation changes the buf array, make a backup for the buf array
    strcpy(backupBuf, buf);

    int len = strlen(buf);
    for (i = 0, j = 0; i < len; ++i) {
        if (buf[i] != ' ') {
            command[argc][j++] = buf[i];
        } else {
            if (j != 0) {
                command[argc][j] = '\0';
                ++argc;
                j = 0;
            }
        }
    }
    if (j != 0) {
        command[argc][j] = '\0';
    }


    argc = 0;
    int flg = OUT;
    for (i = 0; buf[i] != '\0'; i++) {
        if (flg == OUT && !isspace(buf[i])) {
            flg = IN;
            argv[argc++] = buf + i;
        } else if (flg == IN && isspace(buf[i])) {
            flg = OUT;
            buf[i] = '\0';
        }
    }
    argv[argc] = NULL;
}

void do_cmd(int argc, char* argv[]) {
    pid_t pid;
    /* Identify program commands */
    // Identify redirected output commands
    for (j = 0; j < MAX_CMD; j++) {
        if (strcmp(command[j], ">") == 0) {
            strcpy(buf, backupBuf);
            int sample = commandWithOutputRedi(buf);
            return;
        }
    }
    // Identify input redirects
    for (j = 0; j < MAX_CMD; j++) {
        if (strcmp(command[i], "<") == 0) {
            strcpy(buf, backupBuf);
            int sample = commandWithInputRedi(buf);
            return;
        }
    }
    // Identify appending write redirects
    for (j = 0; j < MAX_CMD; j++) {
        if (strcmp(command[j], ">>") == 0) {
            strcpy(buf, backupBuf);
            int sample = commandWithReOutputRedi(buf);
            return;
        }
    }

    // Identify pipeline commands
    for (j = 0; j < MAX_CMD; j++) {
        if (strcmp(command[j], "|") == 0) {
            strcpy(buf, backupBuf);
            int sample = commandWithPipe(buf);
            return;
        }
    }

    // Identify background running commands
    for (j = 0; j < MAX_CMD; j++) {
        if (strcmp(command[j], "&") == 0) {
            strcpy(buf, backupBuf);
            int sample = commandInBackground(buf);
            return;
        }
    }





    /* Identify shell built-in commands */
    if (strcmp(command[0], "cd") == 0) {
        int res = callCd(argc);
        if (!res) printf("cd指令输入错误!");
    } else if (strcmp(command[0], "history") == 0) {
        printHistory(command);
    } else if (strcmp(command[0], "mytop") == 0) {
        myTop();
        return;
    } else if (strcmp(command[0], "exit") == 0) {
        exit(0);
    } else {
        switch(pid = fork()) {
            // fork子进程失败            case -1:
                printf("创建子进程未成功");
                return;
            case 0:
                {   
                    execvp(argv[0], argv);
                    printf("%s: 命令输入错误\n", argv[0]);
                    exit(1);
                }
            default: {
                    int status;
                    waitpid(pid, &status, 0);      
                    int err = WEXITSTATUS(status); 

                    if (err) { 
                        printf("Error: %s\n", strerror(err));
                    }                    
            }
        }
    }
}

int callCd(int argc) {
    // result为1代表执行成功, 为0代表执行失败
    int result = 1;
    if (argc != 2) {
        printf("指令数目错误!");
    } else {
        int ret = chdir(command[1]);
        if (ret) return 0;
    }

    if (result) {
        char* res = getcwd(curPath, BUFFSIZE);
        if (res == NULL) {
            printf("文件路径不存在!");
        }
        return result;
    }
    return 0;
}


int printHistory(char command[MAX_CMD][MAX_CMD_LEN]) {
    int n = atoi(command[1]);

    for (i = n; i > 0 && commandNum - i >= 0; i--) {
        printf("%d\t%s\n", n - i + 1, history[commandNum - i]);
    }
    return 0;
}

int commandWithOutputRedi(char buf[BUFFSIZE]) {
    strcpy(buf, backupBuf);
    char outFile[BUFFSIZE];
    memset(outFile, 0x00, BUFFSIZE);
    int RediNum = 0;
    for ( i = 0; i + 1 < strlen(buf); i++) {
        if (buf[i] == '>' && buf[i + 1] == ' ') {
            RediNum++;
            break;
        }
    }
    if (RediNum != 1) {
        printf("输出重定向指令输入有误!");
        return 0;
    }

    for (i = 0; i < argc; i++) {
        if (strcmp(command[i], ">") == 0) {
            if (i + 1 < argc) {
                strcpy(outFile, command[i + 1]);
            } else {
                printf("缺少输出文件!");
                return 0;
            }
        }
    }

    // command split, outFile is the output file, buf is the command before the redirect symbol
    for (j = 0; j < strlen(buf); j++) {
        if (buf[j] == '>') {
            break;
        }
    }
    buf[j - 1] = '\0';
    buf[j] = '\0';
    // Parsing Instructions
    parse(buf);
    pid_t pid;
    switch(pid = fork()) {
        case -1: {
            printf("创建子进程未成功");
            return 0;
        }
        // Processing sub-processes:
        case 0: {
            // Complete output redirection
            int fd;
            fd = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, 7777);
            // File open failure
            if (fd < 0) {
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);  
            execvp(argv[0], argv);
            if (fd != STDOUT_FILENO) {
                close(fd);
            }
            printf("%s: 命令输入错误\n", argv[0]);
            exit(1);
        }
        default: {
            int status;
            waitpid(pid, &status, 0);       // Wait for the sub-process to return
            int err = WEXITSTATUS(status);  // Read the return code of the sub process
            if (err) { 
                printf("Error: %s\n", strerror(err));
            } 
        }                        
    }

}

int commandWithInputRedi(char buf[BUFFSIZE]) {
    strcpy(buf, backupBuf);
    char inFile[BUFFSIZE];
    memset(inFile, 0x00, BUFFSIZE);
    int RediNum = 0;
    for ( i = 0; i + 1< strlen(buf); i++) {
        if (buf[i] == '<' && buf[i + 1] == ' ') {
            RediNum++;
            break;
        }
    }
    if (RediNum != 1) {
        printf("Input redirection command was entered incorrectly!");
        return 0;
    }

    for (i = 0; i < argc; i++) {
        if (strcmp(command[i], "<") == 0) {
            if (i + 1 < argc) {
                strcpy(inFile, command[i + 1]);
            } else {
                printf("Missing input command!");
                return 0;
            }
        }
    }

    for (j = 0; j < strlen(buf); j++) {
        if (buf[j] == '<') {
            break;
        }
    }
    buf[j - 1] = '\0';
    buf[j] = '\0';
    parse(buf);
    pid_t pid;
    switch(pid = fork()) {
        case -1: {
            printf("Create sub-process unsuccessful");
            return 0;
        }
        // Processing sub-processes:
        case 0: {
            // Complete input redirection
            int fd;
            fd = open(inFile, O_RDONLY, 7777);
            // File open failure
            if (fd < 0) {
                exit(1);
            }
            dup2(fd, STDIN_FILENO);  
            execvp(argv[0], argv);
            if (fd != STDIN_FILENO) {
                close(fd);
            }
            printf("%s: 命令输入错误\n", argv[0]);
            exit(1);
        }
        default: {
            int status;
            waitpid(pid, &status, 0);       //Wait for the sub-process to return
            int err = WEXITSTATUS(status);  // Read the return code of the child process
            if (err) { 
                printf("Error: %s\n", strerror(err));
            } 
        }                        
    }

}


int commandWithReOutputRedi(char buf[BUFFSIZE]) {
    strcpy(buf, backupBuf);
    char reOutFile[BUFFSIZE];
    memset(reOutFile, 0x00, BUFFSIZE);
    int RediNum = 0;
    for ( i = 0; i + 2 < strlen(buf); i++) {
        if (buf[i] == '>' && buf[i + 1] == '>' && buf[i + 2] == ' ') {
            RediNum++;
            break;
        }
    }
    if (RediNum != 1) {
        printf("Wrong input for the Append Output Redirect command!");
        return 0;
    }

    for (i = 0; i < argc; i++) {
        if (strcmp(command[i], ">>") == 0) {
            if (i + 1 < argc) {
                strcpy(reOutFile, command[i + 1]);
            } else {
                printf("Missing output file!");
                return 0;
            }
        }
    }


    for (j = 0; j + 2 < strlen(buf); j++) {
        if (buf[j] == '>' && buf[j + 1] == '>' 
            && buf[j + 2] == ' ') {
            break;
        }
    }
    buf[j - 1] = '\0';
    buf[j] = '\0';

    parse(buf);
    pid_t pid;
    switch(pid = fork()) {
        case -1: {
            printf("Create sub-process unsuccessful");
            return 0;
        }

        case 0: {

            int fd;
            fd = open(reOutFile, O_WRONLY|O_APPEND|O_CREAT|O_APPEND, 7777);
            if (fd < 0) {
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);  
            execvp(argv[0], argv);
            if (fd != STDOUT_FILENO) {
                close(fd);
            }
            printf("%s: 命令输入错误\n", argv[0]);
            exit(1);
        }
        default: {
            int status;
            waitpid(pid, &status, 0);      
            int err = WEXITSTATUS(status); 
            if (err) { 
                printf("Error: %s\n", strerror(err));
            } 
        }                        
    }   
}


int commandWithPipe(char buf[BUFFSIZE]) {
    for(j = 0; buf[j] != '\0'; j++) {
        if (buf[j] == '|')
            break;
    }


    char outputBuf[j];
    memset(outputBuf, 0x00, j);
    char inputBuf[strlen(buf) - j];
    memset(inputBuf, 0x00, strlen(buf) - j);
    for (i = 0; i < j - 1; i++) {
        outputBuf[i] = buf[i];
    }
    for (i = 0; i < strlen(buf) - j - 1; i++) {
        inputBuf[i] = buf[j + 2 + i];
    }


    int pd[2];
    pid_t pid;
    if (pipe(pd) < 0) {
        perror("pipe()");
        exit(1);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork()");
        exit(1);
    }


    if (pid == 0) {                    
        close(pd[0]);                   
        dup2(pd[1], STDOUT_FILENO);     
        parse(outputBuf);
        execvp(argv[0], argv);
        if (pd[1] != STDOUT_FILENO) {
            close(pd[1]);
        }
    }else {                              

        int status;
        waitpid(pid, &status, 0);       
        int err = WEXITSTATUS(status);  
        if (err) { 
            printf("Error: %s\n", strerror(err));
        }

        close(pd[1]);                    
        dup2(pd[0], STDIN_FILENO);       
        parse(inputBuf);
        execvp(argv[0], argv);
        if (pd[0] != STDIN_FILENO) {
            close(pd[0]);
        }       
    }

    return 1;
}


int commandInBackground(char buf[BUFFSIZE]) {
    char backgroundBuf[strlen(buf)];
    memset(backgroundBuf, 0x00, strlen(buf));
    for (i = 0; i < strlen(buf); i++) {
        backgroundBuf[i] = buf[i];
        if (buf[i] == '&') {
            backgroundBuf[i] = '\0';
            backgroundBuf[i - 1] = '\0';
            break;
        }
    }

    pid_t pid;
    pid = fork();
    if (pid < 0) {
        perror("fork()");
        exit(1);
    }

    if (pid == 0) {
        freopen( "/dev/null", "w", stdout );
        freopen( "/dev/null", "r", stdin ); 
        signal(SIGCHLD,SIG_IGN);
        parse(backgroundBuf);
        execvp(argv[0], argv);
        printf("%s: 命令输入错误\n", argv[0]);
        exit(1);
    }else {
        exit(0);
    }
}


void myTop() {
    FILE *fp = NULL;                    
    char buff[255];

    /* Get content i: 
       Overall memory size, the
       free memory size.
       Cache size */
    fp = fopen("/proc/meminfo", "r");   
    fgets(buff, 255, (FILE*)fp);        
    fclose(fp);

    // Get pagesize
    int i = 0, pagesize = 0;
    while (buff[i] != ' ') {
        pagesize = 10 * pagesize + buff[i] - 48;
        i++;
    }

    // Get the total number of pages total
    i++;
    int total = 0;
    while (buff[i] != ' ') {
        total = 10 * total + buff[i] - 48;
        i++;
    }

    // Get the number of free pages free
    i++;
    int free = 0;
    while (buff[i] != ' ') {
        free = 10 * free + buff[i] - 48;
        i++;
    }

    // Get the maximum number of pages largest
    i++;
    int largest = 0;
    while (buff[i] != ' ') {
        largest = 10 * largest + buff[i] - 48;
        i++;
    }

    // Get the number of cached pages cached
    i++;
    int cached = 0;
    while (buff[i] >= '0' && buff[i] <= '9') {
        cached = 10 * cached + buff[i] - 48;
        i++;
    }

    int totalMemory  = pagesize / 1024 * total;
    int freeMemory   = pagesize / 1024 * free;
    int cachedMemory = pagesize / 1024 * cached;

    printf("totalMemory  is %d KB\n", totalMemory);
    printf("freeMemory   is %d KB\n", freeMemory);
    printf("cachedMemory is %d KB\n", cachedMemory);

    /* 2. Get Content 2
        Number of processes and tasks
     */
    fp = fopen("/proc/kinfo", "r");     
    memset(buff, 0x00, 255);           
    fgets(buff, 255, (FILE*)fp);        
    fclose(fp);

    // Get the number of processes
    int processNumber = 0;
    i = 0;
    while (buff[i] != ' ') {
        processNumber = 10 * processNumber + buff[i] - 48;
        i++;
    }
    printf("processNumber = %d\n", processNumber);

    // Get the number of tasks
    i++;
    int tasksNumber = 0;
    while (buff[i] >= '0' && buff[i] <= '9') {
        tasksNumber = 10 * tasksNumber + buff[i] - 48;
        i++;
    }
    printf("tasksNumber = %d\n", tasksNumber);


    // /* 3. get the contents of psinfo */
    DIR *d;
    struct dirent *dir;
    d = opendir("/proc");
    int totalTicks = 0, freeTicks = 0;
    if (d) {
        while ((dir = readdir(d)) != NULL) {                   // Traversing the proc folder
            if (strcmp(dir->d_name, ".") != 0 && 
                strcmp(dir->d_name, "..") != 0) {
                    char path[255];
                    memset(path, 0x00, 255);
                    strcpy(path, "/proc/");
                    strcat(path, dir->d_name);          // The connection becomes the completion pathname
                    struct stat s;
                    if (stat (path, &s) == 0) {
                        if (S_ISDIR(s.st_mode)) {       // Judgment as a directory
                            strcat(path, "/psinfo");

                            FILE* fp = fopen(path, "r");
                            char buf[255];
                            memset(buf, 0x00, 255);
                            fgets(buf, 255, (FILE*)fp);
                            fclose(fp);


                            int j = 0;
                            for (i = 0; i < 4;) {
                                for (j = 0; j < 255; j++) {
                                    if (i >= 4) break;
                                    if (buf[j] == ' ') i++;
                                }
                            }

                            int k = j + 1;
                            for (i = 0; i < 3;) {              
                                for (k = j + 1; k < 255; k++) {
                                    if (i >= 3) break;
                                    if (buf[k] == ' ') i++;
                                }
                            }
                            int processTick = 0;
                            while (buf[k] != ' ') {
                                processTick = 10 * processTick + buff[k] - 48;
                                k++;
                            }
                            totalTicks += processTick;
                            if (buf[j] != 'R') {
                                freeTicks += processTick;
                            }
                        }else continue;
                    }else continue;
                }
        }
    }
    printf("CPU states: %.2lf%% used,\t%.2lf%% idle",
           (double)((totalTicks - freeTicks) * 100) / (double)totalTicks,
           (double)(freeTicks * 100) / (double)totalTicks);
    return;
}


/* main function */
int main() {
    // while loop
    while(1) {
        printf("[zlfshell]$ ");
        // Input characters are stored in buf array, if the number of input characters is 0, then the loop is skipped
        if (get_input(buf) == 0)
            continue;
        strcpy(history[commandNum++], buf);
        strcpy(backupBuf, buf);
        parse(buf);
        do_cmd(argc, argv);
        argc = 0;
    }
}
