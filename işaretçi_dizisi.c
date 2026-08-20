#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// malloc + snprintf / strcpy yapmak yerine doğrudan:
titles[0] = strdup("C Programming");


// Declaring a structure
typedef struct {
  char title[50];
  float price;
} Book;

const int MAX = 3;
int main() {
  Book *book[MAX];

  // Initialize each book (pointer)
  for (int i = 0; i < MAX; i++) {
    book[i] = malloc(sizeof(Book));
    snprintf(book[i]->title, 50, "Book %d", i + 1);
    book[i]->price = 100 + i;
  }

  // Print details of each book
  for (int i = 0; i < MAX; i++) {
    printf("Title: %s, Price: %.2f\n", book[i]->title, book[i]->price);
  }

  // Free allocated memory
  for (int i = 0; i < MAX; i++) {
    free(book[i]);
  }

  return 0;
}





#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  int capacity = 2;

  // 1. Ana pointer dizisi için yer ayırma ve NULL kontrolü
  char **titles = malloc(capacity * sizeof(char *));
  if (titles == NULL) {
    printf("Hata: Ana bellek ayrilamadi!\n");
    return 1;
  }

  // İlk 2 metin için yer ayırma ve NULL kontrolleri
  for (int i = 0; i < capacity; i++) {
    titles[i] = malloc(50 * sizeof(char));
    if (titles[i] == NULL) {
      printf("Hata: Metin %d icin bellek ayrilamadi!\n", i + 1);
      // O ana kadar ayrılmış bellekleri temizleyip çıkmalıyız
      for (int j = 0; j < i; j++) free(titles[j]);
      free(titles);
      return 1;
    }
  }

  snprintf(titles[0], 50, "C Programming");
  snprintf(titles[1], 50, "Data Structures");

  // --- BOYUTU BÜYÜTME (realloc) ---
  int new_capacity = 3;
  char **temp = realloc(titles, new_capacity * sizeof(char *));
  
  if (temp == NULL) {
    printf("Hata: Bellek genisletilemedi! Eski veriler korunuyor.\n");
    // temp NULL döndü ama 'titles' hala geçerli, bu yüzden programı güvenle kapatabiliriz
  } else {
    titles = temp; // Başarılıysa yeni adresi atıyoruz
    capacity = new_capacity;

    // 3. Yeni eklenen eleman için bellek ayırma ve NULL kontrolü
    titles[2] = malloc(50 * sizeof(char));
    if (titles[2] != NULL) {
      snprintf(titles[2], 50, "Algorithms");
    }
  }

  // Ekrana Yazdırma
  for (int i = 0; i < capacity; i++) {
    printf("Kitap %d: %s\n", i + 1, titles[i]);
  }

  // Bellek Temizleme
  for (int i = 0; i < capacity; i++) {
    free(titles[i]);
  }
  free(titles);

  return 0;
}