/**
 * @file    extraer_fichero_main.c
 * @author  Iyán Álvarez, Iker Fernández, Davy Wellinger
 * @date    14/03/2021
 * @brief   Utilidad para extraer ficheros de archivos mypackzip
 * @version 1.0
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "s_header.h"
#include "s_extraer_fichero.h"

extern int extraer_fichero(char * file_mypackzip, int index);

int main (int argc, char *  argv[]) {

    // Array para guardar mensajes de error:
    char ErrorMsg[256];

    // Comprobamos que el numero de argumentos sea correcto:
    if (argc != 3) {
        sprintf(ErrorMsg, "Error: Número de argumentos incorrecto en %s.\nUso: %s file_mypackzip index\n", argv[0], argv[0]);
        write(2, ErrorMsg, strlen(ErrorMsg));
        _exit(1);
    }

    // Control de errores
    int n;

    //ejecutamos la funcion que hemos creado
    n = extraer_fichero(argv[1], atoi(argv[2]));

    //Fallo al abrir el archivo
    if (n == E_OPEN) {
        sprintf(ErrorMsg, "Error: No se puede abrir el el archivo %s.\nUso: %s file_mypackzip index\n", argv[1], argv[0]);
        write(2, ErrorMsg, strlen(ErrorMsg));
        _exit(E_OPEN);
    }

    //Indice no valido
    if (n == E_POS) {
        sprintf(ErrorMsg, "Error: Numero de indice no valido.\nUso: %s file_mypackzip index\n", argv[0]);
        write(2, ErrorMsg, strlen(ErrorMsg));
        _exit(E_POS);
    }

    //Error desconocido
    if (n != 0){
        sprintf(ErrorMsg, "Error: Ha ocurrido un error inespeardo.\nUso: %s file_mypackzip index\n", argv[0]);
        write(2, ErrorMsg, strlen(ErrorMsg));
        _exit(E_DESCO);

    }

    return 0;
}