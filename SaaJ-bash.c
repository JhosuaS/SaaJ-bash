#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <sys/syscall.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX_LINE 128

int saaj_cd(char **args), saaj_ls(char **args),
    saaj_pwd(char **args), saaj_mkdir(char **args), saaj_rm(char **args),
    saaj_cp(char **args), saaj_mv(char **args), saaj_cat(char **args),
    my_strcmp(const char *s1, const char *s2), dispatch_cmd(char **args, bool is_background);
    
size_t my_strlen(const char *s);

void external_cmd(char **args, bool is_background);

typedef int (*BuiltInFunction)(char **args);

typedef struct {
    char *name;
    BuiltInFunction func;
} BuiltInCommand;

BuiltInCommand built_in_commands[] = {
    {"cd" , saaj_cd},
    {"ls" , saaj_ls},
    {"pwd", saaj_pwd},
    {"mkdir", saaj_mkdir},
    {"rm", saaj_rm},
    {"cp", saaj_cp},
    {"mv", saaj_mv},
    {"cat", saaj_cat},
    {NULL, NULL}
};

struct linux_dirent64 {
    long            d_ino;
    off_t           d_off;
    unsigned short  d_reclen;
    unsigned char   d_type;
    char            d_name[];
};

int dispatch_cmd(char **args, bool is_background) {
    for(BuiltInCommand *cmd = built_in_commands; cmd -> name != NULL; cmd++) {
        if(my_strcmp(args[0], cmd -> name) == 0) {
            return cmd -> func(args);
        }
    }

    external_cmd(args, is_background);
    return 0;
}

int my_strcmp(const char *s1, const char *s2) {
    while(*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1  - *(const unsigned char *)s2;
}

size_t my_strlen(const char *s) {
    size_t len = 0;
    while(s[len] != '\0') {
        len++;
    }
    return len;
}

void external_cmd(char **args, bool is_background) {
    pid_t pid = fork();
    if(pid == 0) {//es el proceso hijo        
        execvp(args[0], args);
        write(2, "Error al ejecutar el comando\n", 29);//este mensaje se muestra si execvp falla
        _exit(1);
    }else {
        if(is_background) {
            write(1, "Proceso en segundo plano iniciado\n", 35);
            return;
        }else {
            int status;
            waitpid(pid, &status, 0);
        }
    }
}
//TODO: completar ejecución en segundo plano
//TODO: implementar comandos externos
int main(){
    char line[MAX_LINE];
    char *args[20];
    int readed;

    while(1){

        int status;
        while(waitpid(-1, &status, WNOHANG) > 0) {//se limpian zombies WNOHANG verifica su existencia
            write(1, "Proceso en segundo plano finalizado\n", 36);
        }

        char *p = line;
        int counter = 0;

        write(1,"SaaJ> ", 7);
        readed = read(0, line, MAX_LINE);
        if(readed > 0){
            if(line[readed - 1] == '\n'){
                line[readed - 1] = '\0';
                readed--;
            }
            if(readed == 4 
                && line[0] == 'e' 
                && line[1] == 'x' 
                && line[2] == 'i' 
                && line[3] == 't'){
                _exit(0);
                }else{
                    while(*p != '\0') {
                        while(*p == ' ' || *p == '\t'){
                            p++;
                        }
                        if(*p  == '\0') break;

                        args[counter] = p;
                        counter++;
                        
                        while(*p != ' ' && *p != '\0' && *p != '\t'){
                            p++;
                        }
                        if(*p != '\0'){
                            *p = '\0';
                            p++;
                        }
                    }
                    args[counter] = NULL;

                    bool is_background = 0;

                    if(counter > 0 && (my_strcmp(args[counter - 1], "&") == 0)) {
                        is_background = true;
                        args[counter - 1] = NULL;
                        counter--;
                    }

                    if(counter > 0) {
                        dispatch_cmd(args, is_background);
                    }
                }
        //control de lectura 
        }else if(readed == 0){
            _exit(0);
        }else if (readed == -1){
            write(2, "Error de lectura\n", 18);
            return 1;
        }
    }
    return 0;
}

int saaj_cd(char **args) {
    if(args[1] == NULL) {
        write(2, "Error: se requiere un argumento para cd\n", 40);
        return 1;
    } 
    if(chdir(args[1]) != 0) {
        write(2, "Error al cambiar de directorio\n", 32);
        return 1;
    }
    return 0;
}

int saaj_ls(char **args) {
    struct linux_dirent64 *d;
    char buf[1024];
    char *path = (args[1] != NULL) ? args[1] : ".";
    int fd;
    int bpos;
    int nread;

    if((fd = open(path, O_RDONLY | O_DIRECTORY)) < 0) {
        write(2, "Error al abrir el directorio\n", 29);
        return 1;
    }

    while((nread = syscall(SYS_getdents64, fd, buf, sizeof(buf))) > 0) {
        for(bpos = 0; bpos < nread;) {
            d = (struct linux_dirent64 *) (buf + bpos);

            if(my_strcmp(d -> d_name, ".") != 0 && my_strcmp(d -> d_name, "..") != 0) {
                write(1, d -> d_name, my_strlen(d -> d_name));
                
                if(d -> d_type == 4) {//4 es el valor de DT_DIR
                    write(1, "/", 1);
                }
                write(1, "\n", 1);
            }
            
            bpos += d -> d_reclen;
        }
    }

    if(nread < 0) {
        write(2, "Error en getdents\n", 19);
        return 1;
    }

    close(fd);
    return 0;
}

//saaj_pwd: usa la syscall readlink para leer el enlace simbólico en /proc/self/cwd
//el directorio /proc está montado automáticamente por el kernel de Linux y contiene información sobre los procesos en ejecución
int saaj_pwd(char **args) {
    char cwd_buffer[4096];
    ssize_t read_bytes;

    read_bytes = readlink("/proc/self/cwd", cwd_buffer, sizeof(cwd_buffer) - 1);
    if(read_bytes < 0) {
        write(2, "Error al obtener el directorio actual\n", 39);
        return 1;
    }

    cwd_buffer[read_bytes] = '\0';
    write(1, cwd_buffer, read_bytes);
    write(1, "\n", 1);

    return 0;
}

int saaj_mkdir(char **args) {
    if(args[1] == NULL) {
        write(2, "Error: se requiere un nombre de directorio para mkdir\n", 54);
        return 1;
    }

    if(mkdir(args[1], 0755) != 0) {
        write(2, "Error al crear el directorio\n", 30);
        return 1;
    }

    return 0;
}

//saaj_rm: elimina un archivo o un directorio vacio
int saaj_rm(char **args) {
    if(args[1] == NULL) {
        write(2, "Error: se requiere un archivo o directorio para rm\n", 51);
        return 1;
    }

    if(unlink(args[1]) == 0) {
        return 0;
    } 

    if(rmdir(args[1]) == 0) {
        return 0;
    }

    write(2, "Error al eliminar el archivo o directorio\n", 42);
    return 1;
}

int saaj_cp(char **args) {
    if(args[1] == NULL || args[2] == NULL) {
        write(2, "Error: se requieren archivo origen y destino para cp\n", 54);
        return 1;
    }

    int fd_in, fd_out;
    char buffer[1024];
    ssize_t read_bytes;

    fd_in = open(args[1], O_RDONLY);
    if (fd_in < 0) {
        write(2, "Error al abrir el archivo origen\n", 33);
        return 1;
    }

    fd_out = open(args[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd_out < 0) {
        write(2, "Error al abrir o crear el archivo destino\n", 42);
        close(fd_in);
        return 1;
    }

    while(read_bytes = read(fd_in, buffer, my_strlen(buffer) - 1) > 0) {
        if(write(fd_out, buffer, read_bytes) != read_bytes) {
            write(2, "Error al escribir en el archivo destino\n", 40);
            close(fd_in);
            close(fd_out);
            return 1;
        }
    }

    close(fd_in);
    close(fd_out);
    return 0;
}

int saaj_mv(char **args) {
    if(args[1] == NULL || args[2] == NULL) {
        write(2, "Error: se requieren directorio origen y destino para mv\n", 55);
        return 1;
    }

    if(rename(args[1], args[2]) != 0) {
        write(2, "Error al mover o renombrar el archivo/directorio\n", 49);
        return 1;
    }

    return 0;
}

int saaj_cat(char **args) {
    if(args[1] == NULL) {
        write(2, "Error: se requiere un archivo para cat\n", 35);
        write(2, "\n", 1);
        return 1;
    }

    int fd;//file descriptor
    char buffer[4096];
    ssize_t read_bytes;

    if((fd = open(args[1], O_RDONLY)) < 0) {
        write(2, "Error al abrir el archivo\n", 27);
        return 1;
    }

    while((read_bytes = read(fd, buffer, sizeof(buffer))) > 0) {
        write(1, buffer, read_bytes);
    }

    if(read_bytes < 0) {
        write(2, "Error al leer el archivo\n", 25);
        return 1;
    }
    close(fd);
    write(1, "\n", 1);
    return 0;
}
