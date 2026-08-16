#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Renk kodları tanımlamaları
#define KIRMIZI "\033[31m"
#define RENK_RESET   "\033[0m"
#define VERI_BOYUTU 1000

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf(" ikinci eleman olarak bir dosya girmelisiniz;");
        return 1;
    }
    
    FILE * fPtr;
    int karakter;

    fPtr = fopen(argv[1], "r+");

    if (fPtr == NULL) {
        fPtr = fopen(argv[1], "w+");

        if (fPtr == NULL) {
            fprintf(stderr, KIRMIZI "HATA: Dosya oluşturulamadı!\n" RENK_RESET);
            return 1;
        }
    }

    while ((karakter = fgetc(fPtr)) != EOF) {
        putchar(karakter);
    }

    char veriler[VERI_BOYUTU] = "";
    char son_eleman;
    
        while (1 == 1) {
        fgets(veriler, VERI_BOYUTU, stdin);
        
        son_eleman = veriler[strlen(veriler) - 2]
        if ( son_eleman == '0') {
            son_eleman = '\0';

            fputs(veriler, fPtr);
            break;
            
        }

        fputs(veriler, fPtr);

        }

    fclose(fPtr);
    return 0;
}