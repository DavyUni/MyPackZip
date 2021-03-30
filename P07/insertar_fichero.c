/**
 * @file    insertar_fichero.c
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
#include "s_header.h"
#include "s_insertar_fichero.h"


// Funcion para calcular la siguiente posicion a insertar datos:
unsigned long calcular_position(struct s_mypack_headers mypackHeaders, int size){
    int i = 0;
    // Recorer array de cabeceras para ver si esta vacio
    while (i < NUM_HEADERS) {
        if (mypackHeaders.vHead[i].FileInfo.Type == 'z') {
            return (unsigned long) size;
        }
        i++;
    }
    return (unsigned long) ((NUM_HEADERS*S_HEADER_SIZE) + 1);
}


int insertar_fichero(char * file_mypackzip, char * file_dat, int index) {

    struct s_mypack_headers mypackHeaders;
    struct s_Header mHeader0;
    struct stat my_stat;

    int size, size_mypackzip, n, i, m;


    //Para ficheros:
    char buff[MAX_FILE_NAME];
    ssize_t c_read=0;

    // Control index:
    if (index > NUM_HEADERS) {
        return (E_POS1);
    }

    // Para enlaces symbolicos:
    ssize_t len;
    char bufff[MAX_FILE_NAME];

    // Fds:
    int fd_mypackzip, fd_dat;

    // Apertura file_dat
    fd_dat = open(file_dat, O_RDONLY, 0600);
    if (fd_dat == -1) {
        return E_OPEN2;
    }

    // Apertura file_mypackzip
    fd_mypackzip = open(file_mypackzip, O_RDWR | O_CREAT, 0600);
    if (fd_mypackzip == -1) {
        return E_OPEN1;
    }


    // Inicializar structs (con \0)
    bzero(&(mHeader0), S_HEADER_SIZE); // para guardar nuevos datos
    bzero(&(mypackHeaders), NUM_HEADERS); // para hacer el cambio de array

    // Obtenemos size de file_dat y mypackzip
    size = lseek(fd_dat, 0, SEEK_END);
    size_mypackzip = lseek(fd_mypackzip, 0, SEEK_END);

    // Guardar datos file_dat en los struct
    mHeader0.FileInfo.Type='z';
    mHeader0.FileInfo.Compress='n';
    mHeader0.FileInfo.DataSize=size;
    mHeader0.FileInfo.CompSize=size;
    strcpy(mHeader0.FileInfo.DataFileName, file_dat);


    // Averiguar si se trata de fichero o directorio:
    lstat(file_dat, &my_stat);
    if (S_ISDIR(my_stat.st_mode)){
        mHeader0.FileInfo.FileType='d';
    }
    else if (S_ISREG(my_stat.st_mode)){
        mHeader0.FileInfo.FileType='r';
    }
    else if (S_ISLNK(my_stat.st_mode)){
        mHeader0.FileInfo.FileType='l';
        //Repetimos la asginacion de tamanos para el tipo symbolic link.
        mHeader0.FileInfo.DataSize=my_stat.st_size+1;
        mHeader0.FileInfo.CompSize=my_stat.st_size+1;
    }
    else{
        mHeader0.FileInfo.FileType='o';
    }

    // Obtener datos de fichero mypackzip, controlar, modificar:
    lseek(fd_mypackzip, 0, SEEK_SET);
    read(fd_mypackzip, &mypackHeaders, sizeof(mypackHeaders));
    mHeader0.FileInfo.DatPosition = calcular_position(mypackHeaders, size_mypackzip);

    if (mypackHeaders.vHead[index].FileInfo.Type != 'z') { // if pos index libre
        mypackHeaders.vHead[index] = mHeader0;
        lseek(fd_mypackzip, 0, SEEK_SET);
        n = write(fd_mypackzip, &mypackHeaders, sizeof(mypackHeaders));
        if (mHeader0.FileInfo.FileType == 'r') {
            // Leer y escribir datos:
            lseek(fd_dat, 0, SEEK_SET);
            lseek(fd_mypackzip, mHeader0.FileInfo.DatPosition, SEEK_SET);
            while ( (c_read = read(fd_dat, buff, MAX_FILE_NAME))> 0){

                m = write(fd_mypackzip, buff, c_read );

            }

            if (n != sizeof(mypackHeaders) || m != c_read) {
                close(fd_mypackzip);
                close(fd_dat);
                return E_DESCO;
            }
        }
        else if (mHeader0.FileInfo.FileType == 'l') {
                // Leer y escribir datos:

               if( (len = readlink(file_dat, bufff, MAX_FILE_NAME))!=-1);
                bufff[len]='\0';

                if(len==-1){ //Prueba de error.
                    close(fd_mypackzip);
                    close(fd_dat);
                    return (E_DESCO);
                }

                printf("DATASIZE: %ld\n",mHeader0.FileInfo.DataSize);

                lseek(fd_mypackzip, mHeader0.FileInfo.DatPosition, SEEK_SET);
                m = write(fd_mypackzip, bufff, len+1);

                if ((n != sizeof(mypackHeaders)) || (m != len+1)) {
                    close(fd_mypackzip);
                    close(fd_dat);
                    return (E_DESCO);
                }
            }


    }
    else {
        return E_POS2;
    }

    close(fd_mypackzip);
    close(fd_dat);
    return index;

}