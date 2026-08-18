#include <stddef.h>
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
size_t satir_uzunlugu_bul(const char *metin, int hedef_satir);
size_t metnin_satir_sayisi(const char *metin);

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
    x = 0;
    y = 2;


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
        

        if (ch == KEY_LEFT) {
            if (x > 0) x--;
        }
        else if (ch == KEY_RIGHT) {
            if (x < satir_uzunlugu_bul(metin, y - 1)) x++;
        }
        else if (ch == KEY_UP) {
            if (y > 2) y--;
        }
        else if (ch == KEY_DOWN) {
            if (y < metnin_satir_sayisi(metin) + 2) y++;
        }

        if (satir_uzunlugu_bul(metin, y-1) == 0) {
            move(y, 0);
        }
        else if(satir_uzunlugu_bul(metin, y - 1) <= x) {
            move(y, satir_uzunlugu_bul(metin, y - 1));
        }
        else {
            move(y, x);
        }

    // 3. BACKSPACE (SİLME): Imlecin solundaki karakteri sil
        if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
            if (x > 0) {
                // Imleçten sonraki tüm karakterleri 1 sola kaydırarak silinen karakterin üstünü kapatıyoruz
                memmove(&metin[x - 1], &metin[x], karakter_sayisi - x + 1);
                karakter_sayisi--;
                x--;
            }
        }

        clear();
        printw("Yazmaya baslayin (Sol/Sag Ok: Gezinme | Backspace: Silme | Ctrl + S : Kaydet | Ctrl + Q: Cikis)\n\n");
        printw("%s", metin); // Güncellenmiş metni bas
        refresh();

/*
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
  */  
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

size_t satir_uzunlugu_bul(const char *metin, int hedef_satir) {
    int mevcut_satir = 1;
    size_t i = 0;

    // Hedef satıra git
    while (metin[i] != '\0' && mevcut_satir < hedef_satir) {
        if (metin[i] == '\n') mevcut_satir++;
        i++;
    }

    if (mevcut_satir < hedef_satir) return 0;

    size_t karakter_sayisi = 0;
    while (metin[i] != '\0' && metin[i] != '\n') {
        // UTF-8 devam baytı değilse (0x10xxxxxx değilse) yeni bir karakterdir
        if ((metin[i] & 0xC0) != 0x80) {
            karakter_sayisi++;
        }
        i++;
    }

    return karakter_sayisi;
}

size_t metnin_satir_sayisi(const char *metin) {
    size_t uzunluk = 0;
    for (int i = 0; i < strlen(metin); i++) {
        if (metin[i] == '\0' || metin[i] == '\n') {
            uzunluk++;
        }
    }

    return uzunluk;
}

void satiri_yazdir(const char *metin, int hedef_y, int x) {
    // 1. Terminalde ilgili satırın başına git ve satırı temizle
    int satir = 0;
    int i;
    int j;
    move(hedef_y, 0);
    clrtoeol(); // Satırın sonuna kadar olan kısmı siler (ekranın tamamını değil!)


    // 2. Metizisinden sadece hedef_y satırına denk gelen kısmı bul ve bas
    // (Örn: metin içindeki hedef_y - 1 tane '\n' atlayıp satır sonuna kadar olan kısmı printw yap)
    for (i = 0; satir <= hedef_y - 1 ; i++) {
        if (metin[i] == '\n') {
            satir++;        
        }
    }

    for (j = i; ; inc-expression) {
    
    }
    addch()
    
    // 3. Imleci kullanıcı neredeyse oraya geri koy
}