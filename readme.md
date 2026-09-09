Türkçe:
Bu programı derlemek için gcc kullanmanızı tevsiye ederim, 
bilgisayarınıda gcc'yi indirdikten sonra şu komutu çalıştırmanız 
yeterli olacaktır:    gcc main_v2.c -o teditor -lncursesw
"-lncursesw", bu dosyayı derlerken ncurses kütüphanesinin Wide 
yani geniş versiyonunu dosyaya Linklendir anlamına geliyor.
Bu komutu çalıştırdıktan sonra bulunduğunuz dizinde şu 
kuralla dosyayı çalıştırın: ./teditor [Dosya adı]

English:
I recomend gcc for compale this program. after you install gcc,
run this command: gcc main_v2.c -o teditor -lncursesw
"-lncursesw" means, durin compale this file, link to ncurses
libary's wide version.
After run that command, run the program in the folder you are 
currently in with this rule:  ./teditor [File Name]