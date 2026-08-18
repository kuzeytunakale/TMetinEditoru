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
    noecho();           // Basılan tuşlar ekrana otomatik yazılır
    keypad(stdscr, TRUE);
    nl();   // Enter tuşuna basıldığında yeni satıra (alt satırın başına) geçmesini sağlar
    raw(); 
    
    printw("Yazmaya baslayin (Sol/Sag Ok: Gezinme | Backspace: Silme | Ctrl + S : Kaydet | Ctrl + Q: Cikis)\n\n");
    refresh();

    FILE * fPtr;
    int karakter;
    int x,y; // Imlecin string içindeki konumu

    char *metin = malloc(mevcut_boyut * sizeof(char));

    if (metin == NULL) {
        fprintf(stderr, "Bellek ayrilamadi!\n");
        endwin();
        return 1;
    }

    metin[0] = '\0';

    int ch;

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
        size_t len = strlen(metin);
        metin[len] = (char)karakter;
        metin[len + 1] = '\0'; // String'i kapatmak şarttır

        metin = metin_genislet(metin);
        refresh();
        }


    while (1) {
        ch = getch();
            
        // Kaydetme kısayolu basıldığında (CTRL_S)
        if (ch == CTRL_S) {
            // Mevcut dosya bağlantısını kapatıyoruz
            fclose(fPtr); 

            // Dosyayı "w" modunda açmak içeriği tamamen temizler (truncate eder)
            fPtr = fopen(argv[1], "w");

            if (fPtr != NULL) {
                fputs(metin, fPtr); // Metni en baştan temiz dosyaya yazar
                fclose(fPtr);
            }
        }
        else if (ch == CTRL_Q) {
            free(metin);
            endwin();
            return 0;
        }

        else if (ch == KEY_LEFT) {
            if (imlec > 0) imlec--;
        }
        // 2. SAĞ OK TUŞU: Imleci sağa kaydır
        else if (ch == KEY_RIGHT) {
            if (imlec < karakter_sayisi) imlec++;
        }
        // 3. BACKSPACE (SİLME): Imlecin solundaki karakteri sil
        else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
            if (imlec > 0) {
                // Imleçten sonraki tüm karakterleri 1 sola kaydırarak silinen karakterin üstünü kapatıyoruz
                memmove(&metin[imlec - 1], &metin[imlec], karakter_sayisi - imlec + 1);
                karakter_sayisi--;
                imlec--;
            }
        }

        else if (ch >= 32 && ch <= 126){
            // Imlecin olduğu yerden itibaren karakterleri 1 sağa kaydırarak yer açıyoruz
            memmove(&metin[imlec + 1], &metin[imlec], mevcut_boyut - imlec + 1);
            
            metin[imlec] = (char)ch; // Karakteri ekle
            imlec++; // Imleci ileri taşı
        }

        clear();
        printw("Yazmaya baslayin (Sol/Sag Ok: Gezinme | Backspace: Silme | Ctrl + S : Kaydet | Ctrl + Q: Cikis)\n\n");
        printw("%s", metin); // Güncellenmiş metni bas
        
        // ncurses imlecini metin içindeki mantıksal konumumuza taşıyoruz
        // (1. satır bilgi mesajı olduğu için Y koordinatı 2'dir)
        if (ch == '\n' || ch == '\r') {
            int y, x;
            getyx(stdscr, y, x);
            move(y + 1, 0);

        }   
        move(2, imlec); 
        refresh();
    
    }
    

    free(metin);
    fclose(fPtr); 
    endwin();
    return 0;
}

char* metin_genislet(char *metin) {

    karakter_sayisi++;
    if(karakter_sayisi + 2 >= mevcut_boyut) {
        mevcut_boyut *= 2;

        char *gecici = realloc(metin,mevcut_boyut * sizeof(char));
        if (gecici == NULL) {
            fprintf(stderr, "Bellek genisletilemedi!\n");
            free(metin);// Eski belleği temizle
        }
        return gecici;
    }
    else {
        return metin;
    }
}
