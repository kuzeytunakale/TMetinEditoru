#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ncurses.h>
#include <strings.h>
#include <wchar.h>
#include <unistd.h>

#define CTRL_S 19
#define CTRL_Q 17

// dosyanın zaten olduğunu varsayarak bunu yaptım, hata !!!!


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

    // Eğer dosya tamamen boşsa veya yeni oluşturulduysa kapasiteyi 1 yapıyoruz
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
    move(y, x);

    while ((ch = getch()) != 3) {
    
        switch (ch) {
            case CTRL_S:
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

                // Ekranın alt satırına kaydedildi bildirimi basıyoruz
                mvprintw(LINES - 1, COLS/2, "[ Kaydedildi ]");
                refresh();
                break;
            case KEY_UP:
                if (y > 0) y--;
                break;
            case KEY_DOWN:
                if (y < capacity - 1) y++;
                break;
            case KEY_LEFT:
                if (x > 0) x--;
                break;
            case KEY_RIGHT:
                if (x < utf8_strlen(Metin[y])) x++;
                break;
            case KEY_ENTER:
            case '\n':
            case '\r': {
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
            }
            case KEY_BACKSPACE:
            case 127:       // Çoğu Linux terminalinde Backspace tuşunun ASCII karşılığı
            case '\b': 
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
                break;

            default:
                // Yazdırılabilir tuşlar ve UTF-8 baytları
                if (ch >= 32 && ch != 127) {
                    int b_curr = char_to_byte(Metin[y], x);
                    int total_bytes = strlen(Metin[y]);

                    // Bellekte yeni bayt için yer açıp ekliyoruz
                    memmove(Metin[y] + b_curr + 1, Metin[y] + b_curr, total_bytes - b_curr + 1);
                    Metin[y][b_curr] = ch;

                    // Gelen bayt UTF-8 devam baytı değilse (yeni bir karakter başlığıysa) imleç sütununu 1 artır
                    if ((ch & 0xC0) != 0x80) {
                        x++;
                    }

                    mvaddstr(y, 0, Metin[y]);
                    clrtoeol();
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
    printf("hhhh");

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
