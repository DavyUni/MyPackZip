/**
 * @file    createmypackzip.c
 * @author  Iyán Álvarez, Iker Fernández, Davy Wellinger
 * @date    06/03/2021
 * @brief   Utilidad para compresión de archivos
 * @version 1.0
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "s_header.h"
#define TAMANO_BUFFER 512

struct s_mypack_headers mypackHeaders;
struct s_Header mHeader0;
struct s_Info mInfo0;

int main (int argc, char *  argv[]) {

    // Descriptor del fichero a copiar
    int fdFile;
    // Descriptor del fichero a escribir
    int fdZIP;
    // Almacenar el tamaño del archivo
    long size;
    // Variable para control de errores
    int n;
    // Variable iteradora para for
    int i;
    // Array para guardar mensajes de error
    char ErrorMsg[256];


    // Asignamos la información 0 al header 0
    mHeader0.FileInfo = mInfo0;
    // Guardamos el header 0 en la posición 0 del pack
    mypackHeaders.vHead[0] = mHeader0;

    // Comprobamos que el numero de argumentos sea correcto
    if (argc != 3) {
        sprintf(ErrorMsg, "Error: Número de argumentos incorrecto en %s.\nUso: %s fich_origen fich_destino \n", argv[0], argv[0]);
        write(2, ErrorMsg, strlen(ErrorMsg));
        _exit(1);
    }

    // Abrimos el fichero que contiene los datos en modo lectura
    fdFile = open(argv[1], O_RDONLY, 0600);
    // Comprobamos que el fichero se haya podido abrir correctamente
    if (fdFile == -1) {
        sprintf(ErrorMsg, "Error: No se ha podido abrir el fichero de lectura %s.\nUso: %s fich_origen fich_destino \n",argv[1], argv[0]);
        write(2, ErrorMsg, strlen(ErrorMsg));
        _exit(2);
    }

    // Abrimos el fichero que va a contener los datos a copiar
    fdZIP = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0600);
    // Comprobamos que el fichero se haya podido abrir correctamente
    if (fdZIP == -1) {
        sprintf(ErrorMsg, "Error: No se ha podido abrir el fichero de escritura %s.\nUso: %s fich_origen fich_destino \n",argv[2], argv[0]);
        write(2, ErrorMsg, strlen(ErrorMsg));
        _exit(3);
    }

    // Inicializar structs
    bzero(&(mHeader0), S_HEADER_SIZE);
    bzero(&(mInfo0), S_INFO_SIZE);
    bzero(&(mypackHeaders), NUM_HEADERS);

    // Obtenemos el tamaño del fichero a copiar
    size = lseek(fdFile, 0, SEEK_END);

    // Se definen por defecto (al menos de momento)
    mypackHeaders.vHead[0].FileInfo.Type = 'z';
    mypackHeaders.vHead[0].FileInfo.Compress = 'n';

    // Se guarda la posicion final del fichero en DataSize y CompSize en el struct mInfo
    mypackHeaders.vHead[0].FileInfo.DataSize = size;
    mypackHeaders.vHead[0].FileInfo.CompSize = size;

    // Se copia el nombre del archivo en el struct mInfo
    strcpy(mypackHeaders.vHead[0].FileInfo.DataFileName, argv[1]);

    // Se guarda 0 ya que de momento solo almacenamos un archivo
    mypackHeaders.vHead[0].FileInfo.DatPosition = 0;

    // Posicionamos el puntero al inicio del fichero
    lseek(fdFile, 0, SEEK_SET);
    // Buffer para realizar escritura en Dummy
    char buffer[size];
    // Se copia el contenido del fichero en el buffer y se realiza la escritura en el Dummy
    read(fdFile, buffer, sizeof(buffer));
    for (i = 0; i < sizeof(buffer); i++) {
        mypackHeaders.vHead[0].Dummy[i] = buffer[i];
    }

    // Escribimos el struct mypackHeaders en el fichero fdZIP
    n = write(fdZIP, &mypackHeaders, sizeof(mypackHeaders));
    // Comprobación de errores
    if (n != sizeof(mypackHeaders)) {
        sprintf(ErrorMsg ,"Error: No se ha realizado correctamente la escritura en %s.\nUso: %s fich_origen fich_destino \n", argv[2], argv[0]);
        write(2, ErrorMsg, strlen(ErrorMsg));
        close(fdFile);
        close(fdZIP);
        _exit(3);
    }

    // Cerrar ficheros
    close(fdFile);
    close(fdZIP);

    // Programa finaliza sin errores (devuelve 0)
    return 0;

}