#include "esh.h"
#include "internal_functions.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

cmd_hist_t *hist;
int         hist_count;

void esh_loop(void) {

    signal(SIGINT, SIG_IGN);    // ctrl + c yi shellin almasini engeller

    signal(SIGTTOU, SIG_IGN);   // background process'in input alme ve output yazma hakkini engeller
    signal(SIGTTIN, SIG_IGN);   // ornegin vim kullanirken arka planda bir find process i durmadan yazi yazarsa ekrana karisir

    char **builtin_str = get_builtin_str();

    hist = malloc(sizeof(cmd_hist_t));
    if (hist) {
        hist_count = 0;
    } else {
        fprintf(stderr, "[esh] history allocation error\n");
    }

    char **args;
    char  *line;
    int    status;

    char *home = getenv("HOME"); // if "HOME" enviroment variable defined start from there else use "/" (root dir)
    if (home != NULL) {
        chdir(home);
    } else {
        chdir("/");
    }

    device_info_t *info;

    info = (device_info_t *)malloc(sizeof(device_info_t));

    info->device_name = get_device_name();
    info->user_name   = get_user_name();

    do {
        char *path = getcwd_path();

        printf("[\033[36m%s\033[0m@\033[35m%s\033[0m %s] \033[32m$\033[0m ", info->user_name, info->device_name, path);
        
        free(path);
        path = NULL;
    
        line = esh_read_line();
    
        // bos satir kontrolu yapilir
        if (line == NULL || strlen(line) == 0 || strspn(line, " \v\f\t\r\n") == strlen(line)) {
            status = 1;
        }

        // line bize fonksiyondan geliyor ve bu !! yanina bir girdi girilirse son degere ekstra parametre olarak gecer
        // ornek echo "ege", !! merhaba -> echo "ege" merhaba (calistirilan func) -> ege merhaba (output) ve bunu history e ekler
        // ama suan sadece direk ilk iki karakter !! ise calisiyor degistirilcek
        else if (strcmp(line, "!!") == 0) {
            if (hist->rear != NULL) {

                char *rear_command = custom_strndup(hist->rear->command, strlen(hist->rear->command));   
                int   line_len     = strlen(rear_command); // \0 haric karakter sayisini doner

                hist_node_t *history_node = (hist_node_t *)malloc(sizeof(hist_node_t));

                history_node->command = malloc(sizeof(char) * (line_len + 1));
                strcpy(history_node->command, rear_command);
                history_node->next = NULL;
        
                if (hist->front == NULL && hist->rear == NULL) {
                    hist->front = history_node;
                    hist->rear = history_node;
                    hist_count += 1;
                } else {
                    hist->rear->next = history_node;
                    hist->rear = history_node;
                    hist_count += 1;
                }
                printf("%s\n",rear_command);

                char **prev_args = esh_split_line(rear_command);
                status           = esh_execute(prev_args, builtin_str);
        
                free(rear_command);
                free(prev_args);

            } else {
                // zaten ilk basta olmaz bir tek ilk komut yazdigin anda hem rear hem frontda tanimlanir
                fprintf(stderr, "[esh] no previous command found\n");
                status = 1;
            }
        }
        else {
            int linelen = strlen(line);

            hist_node_t *history_node = (hist_node_t *)malloc(sizeof(hist_node_t));
            history_node->command = malloc(sizeof(char) * (linelen + 1));

            strcpy(history_node->command, line);
            history_node->next = NULL;
    
            if (hist->front == NULL && hist->rear == NULL) {
                hist->front = history_node;
                hist->rear = history_node;
                hist_count++;
            } else {
                hist->rear->next = history_node;
                hist->rear = history_node;
                hist_count++;
            }
    
            args = esh_split_line(line);
            status = esh_execute(args, builtin_str);
            free(args);
        }

        free(line);
    } while (status);
    
}

char *esh_read_line(void) {
    int bufsiz = 1024;
    int c, position = 0;
    char *buffer = malloc(sizeof(char) * bufsiz);

    if (!buffer) {
        fprintf(stderr, "[esh] line buffer allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c; 
        } 
        
        position ++; 

        if (position >= bufsiz) {
            bufsiz += 1024;
            buffer  = realloc(buffer, bufsiz); 
            if (!buffer) {
                fprintf(stderr, "[esh] line buffer allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **esh_split_line(char *line) {
    char **val = shell_parser(line);

    #ifdef DEBUG
    printf("tokens :\n");
    for (int i = 0; val[i] != NULL; i++) {
        printf("  [%d]: '%s'\n", i, val[i]);
    }
    #endif

    return val;
}

int esh_launch(char **args) {
    int status;

    pid_t pid;
    // parent process'de cocugun pid degeri doner gercek 12345 gibi, fakat child process icinde 0 olur.
    pid = fork();

    // calistirilacak komut icin child process olusturulur 
    if (pid == 0) { 
        // normalde child process parent ile ayni grupta olur ama bu fonk sayesinde kendi grubunun patronu haline gelir
        setpgid(0, 0);   

        // biz parent process icin ctrl + c yi ignore etmistik burda onu child icin default haline getiriyoruz
        signal(SIGINT, SIG_DFL);      

        // getpid fg olmasini istedigimiz process id, stdin_fileno is terminalin file descriptoru
        // ctrl + c, ctrl + z direk child process'e gider artik
        tcsetpgrp(STDIN_FILENO, getpid());    

        /*
        execvp - eger fail olur ise return eder fakat eger basarili olursa args[0] programi calisir.
        child processin tum memory space silinir ve args[0] verileri yuklenir. pid degismez ama icerik
        degisir. args[0] bittigi an kernel artik child process i zombie durumuna alir. kalan kod calismaz
        */
        execvp(args[0], args);

        perror("[esh] ");
        exit(EXIT_FAILURE);
    } 

    else if (pid < 0) {
        perror("[esh] ");
        return 1;
    } 

    // > 0 ise parent process
    else {
        // bu cagiri process id si pid olan processin grup idsini de pid yap demek yani child kendi grubunun lideri olur
        setpgid(pid, pid);                    

        // kullanici ne yazarsa sadece child process'e gider ve child etkilenir sadece.
        tcsetpgrp(STDIN_FILENO, pid);         

        /*
        ctrl + c -> terminal foreground process grouptaki tum processlere SIGINT (Interrupt) sinyalini gonderir
                    default davranis: process hemen sonlanir (terminate).
        ---
        ctrl + z -> terminal foreground process grouptaki tum processlere SIGTSTP (Terminal Stop) sinyalini gonderir
                    default davranis: process durur (stopped state) ama olmez.
        */

        // parent child process'i izler, status bilgisi &status adresine yazilir , WUNTRACED -> child stopped olsa bile bana haber ver
        // WIFEXITED, WIFSIGNALED, WIFSTOPPED bu sayede bu fonksiyonlari kullanabilirsin ve sebebi gorursun parametre olarak status alir
        waitpid(pid, &status, WUNTRACED);     

        // terminalin foreground ownerini bu grup yapar
        tcsetpgrp(STDIN_FILENO, getpgrp());  

        // suspend olduysa devam ettir (bazı durumlar için)x
        if (WIFSTOPPED(status)) {

            // fonksitonu ismine gore yargilama amaci sinyali gondermektir ikinci parametredeki 
            kill(pid, SIGCONT);

            // normalde buraya jobs gibi durdurma islemi yapmam lazim ama bizde yapmiyorum ileride eklicem
        }
    }
    return 1;
}

// bu decleration array of function pointers which take char ** as parameter and return int demektir.
int (*builtin_func[]) (char **) = {
    &esh_cd,
    &esh_help,
    &esh_exit,
    &esh_builtins,
    &esh_history
};


int esh_execute(char **args, char **builtin_str) {
    int i;

    if (args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < builtin_num(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return esh_launch(args);
}