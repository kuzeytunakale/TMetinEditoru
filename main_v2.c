#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ncurses.h>

// dosyanın zaten olduğunu varsayarak bunu yaptım, hata !!!!


int main(int argc, char *argv[]) {
    setlocale(LC_ALL, ""); // Sistem Türkçe/UTF-8 karakter kümesini aktif eder

    if (argc < 2) {
        printf("Kullanım: %s <dosya_adi>\n", argv[0]);
        return 1;
    }

    initscr();          // init işlemi   
    cbreak();           // Enter'a basmadan tuşları anlık okur
    noecho();           // Basılan tuşlar ekrana otomatik yazılmaz!
    keypad(stdscr, TRUE);  // Terminalin gelişmiş tuşları (Yön okları, F1-F12, Home, End, Page Up/Down) tek bir karakter (örn: KEY_LEFT) olarak algılamasını sağlar
    nonl();             // Enter'a basınca otomatik olarak alt satıra GEÇMEZ!

    int capacity = 0; // Her iihtimale karşı (eğer metin boşsa) malloc(0) olacak hata hata hata!!!!
    int karakter;
    int en_uzun_satirin_boyu = 0;
    FILE * fPtr;

    // Dosya varsa okuma yazma moduyla açar, dosya yoksa yaratmak için yazma moduyla açar.
    fPtr = fopen(argv[1], "r+");
    if (fPtr == NULL) {
        fPtr = fopen(argv[1], "w+");
        if (fPtr == NULL) {
            printf("HATA: Dosya oluşturulamadı!\n");
            return 1;
        }
    }

    int max = 0; //burada hata var!!!!!!!!!
    while ((karakter = fgetc(fPtr)) != EOF) {
        max++;
        if (karakter == '\n') {
            capacity++;
            if (max > en_uzun_satirin_boyu) {
                en_uzun_satirin_boyu = max;
            }
            max = 0;
        }
    }
    if (max > 0) {
        capacity++;
        if (max > en_uzun_satirin_boyu) {
            en_uzun_satirin_boyu = max;
        }
    }

    rewind(fPtr);

    char satirsatir[en_uzun_satirin_boyu + 1];
    satirsatir[0] = '\0';

    // 1. Metin adreslerini (char*) tutacak dinamik pointer dizisi
    char **Metin = malloc(capacity * sizeof(char *));
    if (Metin == NULL) {
        printf("Hata, bellek ayrılamadı.");
        return(1);
    }

    int i = 0;
    unsigned int length = 0;
    while ((karakter = fgetc(fPtr)) != EOF) {

        if (karakter == '\n') {
            Metin[i] = strdup(satirsatir);
            satirsatir[0] = '\0';
            i++;
            length = 0;
        } else {

            satirsatir[length] = karakter;    
            satirsatir[length + 1] = '\0';
            length++;
        }
    }

    // DÖNGÜ BİTTİĞİNDE: Son satır \n ile bitmiyorsa tamponda veri kalmıştır
    if (length > 0) {
        Metin[i] = strdup(satirsatir);
        satirsatir[0] = '\0';
        i++;
    }

    rewind(fPtr);
    
    for (int index = 0; index < capacity; index++) {
        mvprintw(index, 0, Metin[index]);
    }
    refresh();

    move(0, 0);
    y = 0, x = 0;

    while (ch = getch())
    {
        
        if (ch == KEY_LEFT) {
            if (x > 0) x--;
        }
        else if (ch == KEY_RIGHT) {
            if (x < satir_uzunlugu_bul(metin, y - 1)) x++;
        }
        else if (ch == KEY_UP) {
            if (y > 0) y--;
        }
        else if (ch == KEY_DOWN) {
            if (y < metnin_satir_sayisi(metin) + 2) y++;
        }
    }
    


    for (int index = 0; index < i; index++) {
        free(Metin[index]);
    }
    free(Metin);
    fclose(fPtr);

    endwin();

    return 0;    
}


/*

// Enter tuşuna basıldıysa (\n = Line Feed, \r = Carriage Return)
        if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
            move(y + 1, 0); // Y'yi 1 artır (alt satır), X'i 0 yap (en baş)
        }
            */