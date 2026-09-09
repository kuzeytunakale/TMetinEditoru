#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ncurses.h>
#include <strings.h>
#include <wchar.h>
#include <unistd.h>
#include <stdbool.h>

#define CTRL_S 19
#define CTRL_Q 17


void dosya_kaydet(FILE * fPtr, char ** Metin, int capacity, bool bir_saniye_beklensin_mi) {
    // Dosya boyutunu 0 bayt yapıp içeriği temizler
    ftruncate(fileno(fPtr), 0); 
    // Yazma imlecini dosyanın en başına çeker
    fseek(fPtr, 0, SEEK_SET);

    // Bellekteki güncel satırları dosyaya yazıyoruz
    for (int j = 0; j < capacity; j++) {
        fputs(Metin[j], fPtr);
        fputs("\n", fPtr);
    }
    fflush(fPtr); // Tam yazmayı garanti etmek için tamponu boşaltıyoruz

    if (bir_saniye_beklensin_mi) {
        move(0, 0);
        clrtoeol();
        // Ekranın üst satırına kaydedildi bildirimi basıyoruz, 18 çünkü yazı 36 harflik
        mvprintw(0, COLS/2 -18, "[ ~~~~~~~~~~|Kaydedildi|~~~~~~~~~~ ]");
        refresh();
        napms(1000);// 1 sn bekle
        move(0, 0);
        clrtoeol();
        mvprintw(0, 0, Metin[0]);
        refresh();
    }

}
// Ekrandaki karakter indeksini (x) bellekteki bayt indeksine çevirir
int char_to_byte(const char *str, int char_pos) {
    int b = 0, c = 0;
    while (str[b] != '\0' && c < char_pos) {
        unsigned char ch = str[b];
        if ((ch & 0x80) == 0) b += 1;
        else if ((ch & 0xE0) == 0xC0) b += 2;
        else if ((ch & 0xF0) == 0xE0) b += 3;
        else if ((ch & 0xF8) == 0xF0) b += 4;
        else b += 1;
        c++;
    }
    return b;
}

size_t utf8_strlen(const char *s) {
    size_t count = mbstowcs(NULL, s, 0);
    return (count == (size_t)-1) ? strlen(s) : count;
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, ""); // Sistem Türkçe/UTF-8 karakter kümesini aktif eder

    if (argc < 2) {
        printf("Kullanım: %s <dosya_adi>\n", argv[0]);
        return 1;
    }

    initscr();          // init işlemi   
    raw();   //cbreak();           // Enter'a basmadan tuşları anlık okur
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

    // Eğer dosya tamamen boşsa veya yeni oluşturulduysa kapasiteyi 1 yapıyom
    if (capacity == 0) {
        capacity = 1;
        en_uzun_satirin_boyu = 0;
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

    // Eğer dosyadan hiçbir şey okunmadıysa (dosya boşsa), yazmaya başlamak için ilk satırı başlatıyoruz
    if (i == 0) {
        Metin[0] = strdup("");
        i = 1;
    }

    rewind(fPtr);

    for (int index = 0; index < capacity; index++) {
        mvprintw(index, 0, Metin[index]);
    }
    refresh();

    int x, y;
    x = 0;
    y = 0;
    int ch;
    unsigned int degistirilen_karakter_sayisi = 0;
    move(y, x);

    while ((ch = getch()) != 3) {
        switch (ch) {
            case CTRL_S:
                dosya_kaydet(fPtr, Metin, capacity, 1);
                degistirilen_karakter_sayisi = 0;
                break;
            case CTRL_Q:
                if (degistirilen_karakter_sayisi != 0) {
                    move(0, 0);
                    clrtoeol();
                    mvprintw(0, COLS/2 -19, "[~~|Kaydetmek istiyor musun? [E/h]|~~]");
                    while (1) {
                        ch = getch();
                        if ((ch == 'e') || (ch == 'E') || (ch == '\n')) {
                            dosya_kaydet(fPtr, Metin, capacity, 0);
                            break;
                        }

                        else if ((ch == 'h') || (ch == 'H')) {
                            break;
                        }

                        else {
                            mvprintw(1, COLS/2 -18, "Lütfen geçerli bir karakter giriniz");
                        }
                    }                        
                }

                for (int index = 0; index < i; index++) {
                    free(Metin[index]);
                }
                free(Metin);
                fclose(fPtr);

                endwin();

                return 0;


            case KEY_UP:
                if (y > 0) y--;
                break;
            case KEY_DOWN:
                if (y < capacity - 1) y++;
                break;
            case KEY_LEFT:
                if (x > 0) x--;
                else if (y > 0) {
                    y--;
                    x = utf8_strlen(Metin[y]);
                }
                break;
            case KEY_RIGHT:
                if (x < utf8_strlen(Metin[y])) x++;
                else if (y < capacity - 1) {
                    x = 0;
                    y++;
                }
                break;
            case KEY_HOME:
                x = 0;
                y = 0;
                break;
            case KEY_ENTER:
            case '\n':
            case '\r': 
                degistirilen_karakter_sayisi++;
            // 1. Metin pointer dizisinde 1 yeni satır için bellek genişletiyoruz
                char **temp = realloc(Metin, (capacity + 1) * sizeof(char *));
                if (temp != NULL) {
                    Metin = temp;

                    // 2. y+1 ve altındaki satır pointer'larını 1 adım aşağı kaydırıyoruz
                    memmove(&Metin[y + 2], &Metin[y + 1], (capacity - (y + 1)) * sizeof(char *));

                    // 3. Yeni satıra boş bir string atıyoruz
                    Metin[y + 1] = strdup("");

                    capacity++;
                    i++;

                    // 4. İmleci yeni satırın başına çekiyoruz
                    y++;
                    x = 0;

                    // 5. Ekrandaki tüm satırları yeniden çizip tazeliyoruz
                    clear();
                    for (int index = 0; index < capacity; index++) {
                        mvprintw(index, 0, "%s", Metin[index]);
                    }
                    refresh();
                }
                break;
            
                case '\t':
                int spaces = 4;
                int b_curr = char_to_byte(Metin[y], x);
                int total_bytes = strlen(Metin[y]);

                // Değişken adı line_temp olarak değiştirildi:
                char *line_temp = realloc(Metin[y], total_bytes + spaces + 1);
                if (line_temp != NULL) {
                    Metin[y] = line_temp;

                    memmove(Metin[y] + b_curr + spaces, Metin[y] + b_curr, total_bytes - b_curr + 1);
                    memset(Metin[y] + b_curr, ' ', spaces);

                    x += spaces;

                    mvaddstr(y, 0, Metin[y]);
                    clrtoeol();
                }
                break;
            

            case KEY_BACKSPACE:
            case 127:       // Çoğu Linux terminalinde Backspace tuşunun ASCII karşılığı
            case '\b': 
                degistirilen_karakter_sayisi++;
                if (x > 0) {
                    int b_curr = char_to_byte(Metin[y], x);
                    int b_prev = char_to_byte(Metin[y], x - 1);
                    int total_bytes = strlen(Metin[y]);

                    // Karakterin bellekte kapladığı tüm baytları (örn. 2 bayt) tek seferde siliyoruz
                    memmove(Metin[y] + b_prev, Metin[y] + b_curr, total_bytes - b_curr + 1);
                    x--;

                    mvaddstr(y, 0, Metin[y]);
                    clrtoeol();
                }
                else {
                    if (x == 0 && y > 0) {
                        // 1. Üst satırın mevcut uzunluğunu alıyoruz (imleç buraya geçecek)
                        int prev_len_chars = utf8_strlen(Metin[y - 1]);
                        int prev_len_bytes = strlen(Metin[y - 1]);
                        int curr_len_bytes = strlen(Metin[y]);

                        // 2. Üst satırın belleğini, alt satırdaki metni de alacak şekilde genişletiyoruz
                        char *line_temp = realloc(Metin[y - 1], prev_len_bytes + curr_len_bytes + 1);
                        if (line_temp != NULL) {
                            Metin[y - 1] = line_temp;

                            // 3. Alt satırdaki metni üst satırın sonuna ekliyoruz
                            strcat(Metin[y - 1], Metin[y]);

                            // 4. Silinen alt satırın belleğini serbest bırakıyoruz
                            free(Metin[y]);

                            // 5. y+1 ve altındaki tüm satır pointer'larını 1 adım yukarı kaydırıyoruz
                            memmove(&Metin[y], &Metin[y + 1], (capacity - (y + 1)) * sizeof(char *));

                            capacity--;
                            i--;

                            // 6. Pointer dizisinin belleğini 1 eleman küçültüyoruz
                            char **temp = realloc(Metin, capacity * sizeof(char *));
                            if (temp != NULL) {
                                Metin = temp;
                            }

                            // 7. İmleci üst satırın birleşme noktasına taşıyoruz
                            y--;
                            x = prev_len_chars;

                            // 8. Ekranı tamamen temizleyip yeni durumu çiziyoruz
                            clear();
                            for (int index = 0; index < capacity; index++) {
                                mvprintw(index, 0, "%s", Metin[index]);
                            }
                            refresh();
                        }
                    }
                }
                break;

                default:
                    degistirilen_karakter_sayisi++;
                // Geçerli bir tuş (ASCII veya UTF-8 başlangıç baytı) gelmişse
                    if (ch >= 32) {
                        char utf8_char[5] = {0}; // En fazla 4 bayt UTF-8 + 1 null
                        utf8_char[0] = ch;
                        int byte_count = 1;

                        // Eğer gelen bayt bir UTF-8 karakterin ilk baytıysa, boyutunu belirle
                        if ((ch & 0xE0) == 0xC0) byte_count = 2;       // 2 baytlık (Örn: ş, ç, ğ, ö, ü, ı)
                        else if ((ch & 0xF0) == 0xE0) byte_count = 3;  // 3 baytlık 
                        else if ((ch & 0xF8) == 0xF0) byte_count = 4;  // 4 baytlık (Emojiler vb.)

                        // Eksik kalan baytları terminal tamponundan hemen okuyarak karakteri tamamla
                        for (int i = 1; i < byte_count; i++) {
                            utf8_char[i] = getch(); 
                        }

                        int b_curr = char_to_byte(Metin[y], x);
                        int total_bytes = strlen(Metin[y]);

                        // DİKKAT: Bellek alanını yeni eklenecek baytlar kadar büyütmeliyiz! 
                        // Aksi takdirde strdup ile ayrılan sınır aşılır (Segfault).
                        char *temp = realloc(Metin[y], total_bytes + byte_count + 1);
                        if (temp != NULL) {
                            Metin[y] = temp;
                            
                            // Yeni eklenecek baytlar için sağdaki metni kaydırıp yer açıyoruz
                            memmove(Metin[y] + b_curr + byte_count, Metin[y] + b_curr, total_bytes - b_curr + 1);
                            
                            // UTF-8 bayt dizisinin tamamını açılan boşluğa kopyalıyoruz
                            memcpy(Metin[y] + b_curr, utf8_char, byte_count);

                            // Karakter bütün olarak eklendiği için imleci sadece 1 artırıyoruz
                            x++;

                            // Satırı ekranda güncelliyoruz
                            mvaddstr(y, 0, Metin[y]);
                            clrtoeol();
                        }
                    }
                    break;
        }

        // alta veya üste giderken imlecimiz böşlukta görünmesin
        
        if (x > utf8_strlen(Metin[y])) {
            x = utf8_strlen(Metin[y]);
        }
        
        move(y, x);

        
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



    // index noktasından string sonuna kadar olan kısmı (len - index + 1 bayt) 
    // 1 adım sağa (str + index + 1) kaydırıyoruz ('\0' dahil kayar)
