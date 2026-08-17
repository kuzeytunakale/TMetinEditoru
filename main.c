#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>

// Renk kodları tanımlamaları
#define KIRMIZI "\033[31m"
#define RENK_RESET   "\033[0m"
#define CTRL_S 19
#define CTRL_Q 17

int karakter_sayisi = 0;
size_t mevcut_boyut = 50;


char* metin_genislet(char *metin);

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, ""); // Sistem Türkçe/UTF-8 karakter kümesini aktif eder

    if (argc == 1) {
        printf("İkinci eleman olarak bir dosya girmelisiniz\n");
        return 1;
    }

    initscr();          
    cbreak();           // Enter'a basmadan tuşları anlık okur
    echo();           // Basılan tuşlar ekrana otomatik yazılır
    keypad(stdscr, TRUE);
    nl();   // Enter tuşuna basıldığında yeni satıra (alt satırın başına) geçmesini sağlar
    raw();
    
    FILE * fPtr;
    int karakter;

    char *metin = malloc(mevcut_boyut * sizeof(char));

    if (metin == NULL) {
        fprintf(stderr, "Bellek ayrilamadi!\n");
        endwin();
        return 1;
    }

    char ch[2] = "";

    fPtr = fopen(argv[1], "r+");

    if (fPtr == NULL) {
        fPtr = fopen(argv[1], "w+");

        if (fPtr == NULL) {
            fprintf(stderr, KIRMIZI "HATA: Dosya oluşturulamadı!\n" RENK_RESET);
            endwin();
            return 1;
        }
    }

    while ((karakter = fgetc(fPtr)) != EOF) {
        addch(karakter);
        metin[strlen(metin)] = karakter;

        metin = metin_genislet(metin);
        refresh();
        }

    while (1) {
        ch[0] = getch();
            
        if ( ch[0] == CTRL_S) {
            fputs(metin, fPtr);
            free(metin);
            fclose(fPtr);
            endwin();
            return 0;
        }
        else if (ch[0] == CTRL_Q) {
            free(metin);
            fclose(fPtr);
            endwin();
            return 0;
        }

        else {
            if (ch[0] == '\n' || ch[0] == '\r') {
                int y, x;
                getyx(stdscr, y, x);
                move(y + 1, 0);
            }
            
            metin = metin_genislet(metin);

            strcat(metin, ch);
            refresh();

        }
    }
    

    free(metin);
    fclose(fPtr);
    endwin();
    return 0;
}

char* metin_genislet(char *metin) {

    karakter_sayisi++;
    if(karakter_sayisi == mevcut_boyut - 1) {
        mevcut_boyut *= 2;

        char *gecici = realloc(metin,mevcut_boyut * sizeof(char));
        if (gecici == NULL) {
            fprintf(stderr, "Bellek genisletilemedi!\n");
            free(metin);// Eski belleği temizle
        }
        return gecici;
    }
}
