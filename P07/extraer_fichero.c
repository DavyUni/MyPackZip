/**
 * @file    extraer_fichero.c
 * @author  Iyán Álvarez, Iker Fernández, Davy Wellinger
 * @date    14/03/2021
 * @brief   Utilidad para extraer ficheros y directorios de archivos mypackzip
 * @version 1.0
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "s_header.h"
#include "s_extraer_fichero.h"


int extraer_fichero(char * file_mypackzip, int index){

    // Struct return
    struct s_mypack_headers mypackHeaders;
    // Fichero de entrada
    int fdFile;
    // Fichero de salida
    int fdReturn;
    // Modo del archivo
    mode_t mode;
    //Nombre del fichero destino
    char nameFile[MAX_FILE_NAME];
    //Nombre del directorio destino
    char nameDir[MAX_FILE_NAME];

    int i=0;

    // Si el indice esta fuera de numero de cabeceras devuelve error
    if (index > NUM_HEADERS-1 || index < 0) {
        return E_POS;
    }

    // Error de apertura
    fdFile = open(file_mypackzip, O_RDONLY, 0600);
    if (fdFile == -1) {
        return E_OPEN;
    }

    // Apuntamos al principio del fichero
    lseek(fdFile, 0, SEEK_SET);
    // Guardamos la estuctura en nuestra variable
    read(fdFile, &mypackHeaders, sizeof(mypackHeaders));
    // Si el fichero del indice esta vacio devolvemos un error
    if (mypackHeaders.vHead[index].FileInfo.Type != 'z'){
        return E_POS;
    }

    //Detectamos de que tipo es el archivo a descomprimir
    if(mypackHeaders.vHead[index].FileInfo.FileType=='r'){
        // Guardamos el nombre del fichero destino
        strcpy(nameFile, mypackHeaders.vHead[index].FileInfo.DataFileName);
        // Creamos el fichero a devolver
        fdReturn = open(nameFile, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        // Apuntamos al principio del fichero a devolver
        lseek(fdReturn, 0, SEEK_SET);

        // Apuntamos al principio de la información que nos interesa
        lseek(fdFile, mypackHeaders.vHead[index].FileInfo.DatPosition, SEEK_SET);

        // Buffer
        char buf[mypackHeaders.vHead[index].FileInfo.DataSize];
        // Guardamos la información en nuestra variable
        read(fdFile, buf, sizeof(buf));

        // Guardamos la información en nuestra variable
        write(fdReturn, &buf, sizeof(buf));

    }else if(mypackHeaders.vHead[index].FileInfo.FileType=='d'){
        // Guardamos el nombre del directorio destino
        strcpy(nameDir, mypackHeaders.vHead[index].FileInfo.DataFileName);
        // Creamos la carpeta
        mkdir(nameDir,0777);
        while(i<NUM_HEADERS){
            if(mypackHeaders.vHead[i].FileInfo.Type == 'z'){
                // Guardamos el nombre del fichero destino
                strcpy(nameFile, mypackHeaders.vHead[i].FileInfo.DataFileName);
                if(strncmp(nameFile, nameDir, sizeof(nameDir))==0){
                    // Creamos el fichero a devolver
                    fdReturn = open(nameFile, O_WRONLY | O_CREAT | O_TRUNC, 0600);
                    // Apuntamos al principio del fichero a devolver
                    lseek(fdReturn, 0, SEEK_SET);

                    // Apuntamos al principio de la información que nos interesa
                    lseek(fdFile, mypackHeaders.vHead[i].FileInfo.DatPosition, SEEK_SET);

                    // Buffer
                    char buf[mypackHeaders.vHead[i].FileInfo.DataSize];
                    // Guardamos la información en nuestra variable
                    read(fdFile, buf, sizeof(buf));

                    // Guardamos la información en nuestra variable
                    write(fdReturn, &buf, sizeof(buf));
                }
            }
            i++;
        }

    }
    else if (mypackHeaders.vHead[index].FileInfo.FileType == 'l'){
        printf("Es un link \n" );
        // Guardamos el nombre del link
        strcpy(nameFile, mypackHeaders.vHead[index].FileInfo.DataFileName);
        // Apuntamos al principio de la información que nos interesa
        lseek(fdFile, mypackHeaders.vHead[index].FileInfo.DatPosition, SEEK_SET);
        printf("DATASIZE: %ld\n",mypackHeaders.vHead[index].FileInfo.DataSize);
        // Buffer (necesario declararlo aqui)
        char buf[mypackHeaders.vHead[index].FileInfo.DataSize];
        // Guardamos la información en nuestra variable
        read(fdFile, buf, sizeof(buf));
        printf("%s \n", buf);
        //Creamos el link simbolico
        symlink(buf, nameFile);
    }
    close(fdFile);
    close(fdReturn);
    return 0;

}