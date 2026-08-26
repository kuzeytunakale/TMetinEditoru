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
void satiri_yazdir(const char *metin, int hedef_y, int x);
void tum_ekranı_yenile(const char *metin, int y, int x);
int koordinati_indekse_cevir(const char *metin, int hedef_y, int hedef_x);

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, ""); // Sistem Türkçe/UTF-8 karakter kümesini aktif eder

    if (argc == 1) {
        printf("İkinci eleman olarak bir dosya girmelisiniz\n");
        return 1;
    }

    initscr();          // init işlemi   
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
            x--;
            int iy, ix;
            getyx(stdscr, iy, ix); // Ncurses'tan mevcut ekran imlecini al

            // 1. Ekran koordinatını gerçek dizi indeksine çevir
            int idx = koordinati_indekse_cevir(metin, iy, ix);
            
            if (idx > 0) {
                // 1. SİLİNECEK KARAKTERİN BAŞLANGICINI BUL
                int silinecek_bayt = 0;
                int k = idx - 1;

                while (k >= 0) {
                    silinecek_bayt++;
                    // Eğer bayt 10xxxxxx (devam) formatında DEĞİLSE, harfin başıdır
                    if ((metin[k] & 0xC0) != 0x80) {
                        break; 
                    }
                    k--;
                }

                // 2. SİLME İŞLEMİNİ YAP
                bool satir_silindi = (metin[idx - silinecek_bayt] == '\n');
                size_t len = strlen(metin);

                memmove(&metin[idx - silinecek_bayt], &metin[idx], len - idx + 1);

                // 3. EKRANI GÜNCELLE
                if (satir_silindi) {
                    // Satır birleştiği için tüm ekran yenilenmeli
                    clear();
                    printw("Yazmaya baslayin (Sol/Sag Ok: Gezinme | Backspace: Silme | Ctrl + S : Kaydet | Ctrl + Q: Cikis)\n\n");
                    printw("%s", metin);
                    
                    // İmleci üst satırın sonuna alman gerekecek (bunu yazmıştın varsayıyorum)
                } else {
                    // Sadece tek karakter silindiği için ekranı temizle ve x'i 1 geri al
                    clear();
                    printw("Yazmaya baslayin (Sol/Sag Ok: Gezinme | Backspace: Silme | Ctrl + S : Kaydet | Ctrl + Q: Cikis)\n\n");
                    printw("%s", metin);
                    move(y, x - 1);
                }
            }        
        }

        

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
    move(hedef_y, 0);
    clrtoeol(); 

    int satir = 0;
    int i = 0;

    // 2. Hedef satırın başlangıç indeksini bul
    while (metin[i] != '\0' && satir < hedef_y) {
        if (metin[i] == '\n') {
            satir++;
        }
        i++;
    }

    // 3. Sadece o satırdaki karakterleri \n veya \0 görene kadar bas
    while (metin[i] != '\0' && metin[i] != '\n') {
        addch(metin[i]);
        i++; // i değerini artırmayı UNUTMA
    }

    // 4. İmleci kullanıcının kaldığı koordinata geri getir
    move(hedef_y, x);
    refresh();
}

void tum_ekranı_yenile(const char *metin, int y, int x) {
    clear();
    printw("Yazmaya baslayin (Sol/Sag Ok: Gezinme | Backspace: Silme | Ctrl + S : Kaydet | Ctrl + Q: Cikis)\n\n");
    printw("%s", metin);
    refresh();
    move(y, x);
}

// (y, x) koordinatını metin dizisindeki tek boyutlu 'index'e çeviren fonksiyon
int koordinati_indekse_cevir(const char *metin, int hedef_y, int hedef_x) {
    int mevcut_y = 0;
    int mevcut_x = 0;
    int i = 0;

    while (metin[i] != '\0') {
        // Geçerli bayt bir UTF-8 "devam baytı" mı? (10xxxxxx formatında mı?)
        bool devam_bayti_mi = ((metin[i] & 0xC0) == 0x80);

        // Sadece karakterin tam başındayken koordinat eşleşmesi ara
        if (!devam_bayti_mi) {
            if (mevcut_y == hedef_y && mevcut_x == hedef_x) {
                return i;
            }
        }

        if (metin[i] == '\n') {
            mevcut_y++;
            mevcut_x = 0;
        } else if (!devam_bayti_mi) {
            // SADECE GÖRSEL BİR KARAKTER BAŞLADIĞINDA X'İ ARTIR
            mevcut_x++;
        }
        i++;
    }

    return i;
}