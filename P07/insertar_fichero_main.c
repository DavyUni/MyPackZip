/**
 * @file    insertar_fichero_main.c
 * @author  Iyán Álvarez, Iker Fernández, Davy Wellinger
 * @date    28/03/2021
 * @brief   Utilidad para insertar ficheros en archivos mypackzip
 * @version 1.1.1
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include "s_header.h"
#include "s_insertar_fichero.h"

extern int insertar_fichero(char * file_mypackzip, char * file_dat, int index);


void controlerrores(int n, char *  argv[]) {
    char ErrorMsg[256];
    if (n == E_OPEN1) {
        sprintf(ErrorMsg, "Error: No se puede abrir o crear %s.\nUso: %s file_mypackzip file_dat index \n", argv[1], argv[0]);
        write(2, ErrorMsg, strlen(ErrorMsg));
        _exit(E_OPEN1);
    }
    else if (n == E_OPEN2) {
        sprintf(ErrorMsg, "Error: No se puede abrir %s.\nUso: %s file_mypackzip file_dat index \n", argv[2], argv[0]);
        write(2, ErrorMsg, strlen(ErrorMsg));
        _exit(E_OPEN2);
    }
    else if (n == E_POS1) {
        sprintf(ErrorMsg, "Error: Número de index %s erróneo.\nUso: %s file_mypackzip file_dat index \n", argv[3], argv[0]);
        write(2, ErrorMsg, strlen(ErrorMsg));
        _exit(E_POS1);
    }
    else if (n == E_POS2) {
        sprintf(ErrorMsg, "Error: Número de index %s hace referencia a un s_header que ya contiene datos válidos.\nUso: %s file_mypackzip file_dat index \n", argv[3], argv[0]);
        write(2, ErrorMsg, strlen(ErrorMsg));
        _exit(E_POS2);
    }
}

int main (int argc, char *  argv[]) {

    int n, cerr, fd_mypackzip;
    int i = 0;
    int espacios = 0;
    struct s_mypack_headers mypackHeaders;

    struct stat my_stat, aux_stat;
    DIR *dir;
    struct dirent *rdir;
    int indexes[NUM_HEADERS];
    int nfich = 0;
    int ninser = 0;
    char insName[MAX_FILE_NAME];
    // Array para guardar mensajes de error
    char ErrorMsg[256];
    char InfoMsg[256];


    // Comprobamos que el numero de argumentos sea correcto
    if (argc != 4) {
        sprintf(ErrorMsg, "Error: Número de argumentos incorrecto en %s.\nUso: %s file_mypackzip file_dat index \n", argv[0], argv[0]);
        write(2, ErrorMsg, strlen(ErrorMsg));
        _exit(1);
    }

    // Cuantos espacios libres hay:
    fd_mypackzip = open(argv[1], O_RDWR | O_CREAT, 0600);
    lseek(fd_mypackzip, 0, SEEK_SET);
    read(fd_mypackzip, &mypackHeaders, sizeof(mypackHeaders));
    close(fd_mypackzip);

    // Averiguar espacios libres y cuales indexes son
    while (i < NUM_HEADERS){
        if(mypackHeaders.vHead[i].FileInfo.Type!='z'){
            indexes[espacios]=i;
            espacios++;
        }
        i++;
    }

    // Averiguar si se trata de fichero o directorio:
    lstat(argv[2], &my_stat);
    if (S_ISDIR(my_stat.st_mode)){

        // Directorio y prueba de error:
        n = insertar_fichero(argv[1], argv[2], atoi(argv[3]));
        if (n != atoi(argv[3])) {
            sprintf(ErrorMsg, "Error: Ha habido un error almazenando el directorio: %d\n", n);
            write(2, ErrorMsg, strlen(ErrorMsg));
            _exit(E_DESCO);
        }
        if ((dir = opendir(argv[2])) == NULL) {
            sprintf(ErrorMsg, "Error: Ha habido un error abriendo el directorio\n");
            write(2, ErrorMsg, strlen(ErrorMsg));
            _exit(E_DESCO);
        }

        // Iteramos sobre los ficheros en el directorio
        while ((rdir = readdir(dir)) != NULL) {
            sprintf(insName, "%s%s", argv[2], rdir->d_name);
            stat(insName,&aux_stat);
            if (S_ISREG(aux_stat.st_mode)) {
                // Insertamos directamente
                if (espacios > 0) {
                    cerr = insertar_fichero(argv[1], insName, indexes[--espacios]);
                    controlerrores(cerr, argv);
                    write(1, InfoMsg, strlen(InfoMsg));
                    ninser++;
                }
                nfich++;
            }
            else if(S_ISLNK(aux_stat.st_mode)) {
                // Insertamos directamente
                printf("ES UN LINK\n");
                if (espacios > 0) {
                    insertar_fichero(argv[1], insName, indexes[--espacios]);
                    //espacios--;
                    ninser++;
                }
                nfich++;
            }
        }
        // Si el numero de insertados y de ficheros es distinto, hay un error
        if (nfich != ninser) {
            sprintf(ErrorMsg, "Error: Alguno de los ficheros no ha podido añadirse");
            write(2, ErrorMsg, strlen(ErrorMsg));
            _exit(E_DESCO);
        }

    }
    else if (S_ISREG(my_stat.st_mode)) {
        cerr = insertar_fichero(argv[1], argv[2], atoi(argv[3]));
        controlerrores(cerr, argv);
        sprintf(InfoMsg, "Insertado %s en índice %d\n", argv[2], atoi(argv[3]));
        write(1, InfoMsg, strlen(InfoMsg));
    }
    else if(S_ISLNK(my_stat.st_mode)){
        cerr = insertar_fichero(argv[1], argv[2], atoi(argv[3]));
        controlerrores(cerr, argv);
        sprintf(InfoMsg, "Insertado %s en índice %d\n", argv[2], atoi(argv[3]));
        write(1, InfoMsg, strlen(InfoMsg));
    }
    else {
        // No existe la referencia o el fichero
        controlerrores(E_OPEN2, argv);
    }


    return 0;

}