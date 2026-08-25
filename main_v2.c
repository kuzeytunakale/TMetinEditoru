#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int capacity = 0; // Her iihtimale karşı 1 den başlattım
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
    int max = 0;
    while ((karakter = fgetc(fPtr)) != EOF) {
        max++;
        if ((karakter == '\0') || (karakter == '\n')) {
            capacity++;
            if (max > en_uzun_satirin_boyu) {
                en_uzun_satirin_boyu = max;
            }
        }
    }

    char satirsatir[en_uzun_satirin_boyu];

    // 1. Metin adreslerini (char*) tutacak dinamik pointer dizisi
    char **Metin = malloc(capacity * sizeof(char *));
    if (Metin == NULL) {
        printf("Hata, bellek ayrılamadı.");
        return(1);
    }

    int i = 0;
    while ((karakter = fgetc(fPtr)) != EOF) {
    strcat(satirsatir, karakter)
    Metin[i] = strdup("İlk satır metni");
        
    }






    //listedeli belleklerin initilazyonu
    for (int i = 0; i < capacity; i++) {
        Metin[i] = malloc(50 * sizeof(char));
        if (Metin[i] == NULL) {
            printf("Hata: Metin %d icin bellek ayrilamadi!\n", i + 1);
            // O ana kadar ayrılmış bellekleri temizleyip çıkmalıyız
            for (int j = 0; j < i; j++) free(Metin[j]);
            free(Metin);
            return 1;
        }
    }

    while ((karakter = fgetc(fPtr)) != EOF) {
        for (int index = 0; index < capacity; index++) {
            Metin[index] 
        }
    }
    
}