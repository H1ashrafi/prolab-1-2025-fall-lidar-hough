#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <locale.h>
/* bu bizim svg ve gp deki adetlerin a.b yi a,b ile ayni anlama gledigini anlatmak icin var
aralarda bir yerde hata aldik boyle bir sey ekledik
windowsun dili de bu . etkili olabilir...*/

double PI;

// Structlari tanimlayalim:
struct variable {
    double veri,
    x,
    y;
    //zaten space falan kod da anlami yok daha nalasir olasi icin (ve guzel olmasi icin):
    int gecerli;
    int hangiIndex;
    //bunu tanimlamiya da bilirdik te yine de...
};
struct datas {
    double angle_min,
    angle_max,
    angle_increment;
    double range_min,
    range_max;
    int ElemanSayisi;
};
struct dogru {
    double a,
    b;
    double ilkX,
    ilkY,
    sonX,
    sonY;
    int dikey_mi;
};

// Sabitlerde sira
const int EkranBoyut=800;
const double zoomlama=100.0;//10 pixel...

// Koordinat donusumu (sade) fonksiyonlarin tanimalama isi
//dis dunyadayi kucultuyoruz! daha rahat gosterilsin diye(zoomlama zoom miktari):
double DonusturX(double dunya_x)
{
 return (dunya_x*zoomlama)+(EkranBoyut/2.0);
}
double DonusturY(double dunya_y)
{
return (-dunya_y*zoomlama)+(EkranBoyut/2.0);//neden - ?svg png gibi yerlerde y - yukariya dogru demekmis...
}

// Fonksiyonlari ilk basta tanimliyalim ki sora  bir birinin icerisinde belki kullaniriz
int Esit(char BirString[],char BaskaBirString[],int Uzunluk);
double SonParametreBulucu(FILE *dosya,char String[],int Uzunluk);
int RangeCikarici(FILE *dosya,struct variable *a);
void DataYerlestirme(struct datas *data,FILE *dosya);
void RangeArrangement(struct variable *veri,struct datas data,int sayac);
int arasindami(double a,double b,double c);
double DereceKac(struct dogru bir,struct dogru iki);
int KesisimKontrol(struct dogru bir,struct dogru iki,double *kesisimX,double *kesisimY,double *aci);
int** houghuUygula(struct variable *veri,int sayac,struct datas data,
                   int *houghSatirinPtr,int *houghSutununPtr,int *rhoKaydirPtr);
                   //rho kaydir rho larin alabilecegi degerlerdir ama - olmamasi icin bu listede her etmeye celiscaz
                   /*sonraki fonklarda kulanilmasi icin ptr oluyorlar
                   pointer pointer matris gibi 2 boyutlu olmasini sagliyor*/
                   //satir uzamasin diye kaydirmaya gerek olmasin diye,yukarda da space falanin onemsiz oldugunu yazmistim ...
int DogrulariCikar(int** oylarMatrisi,int hSatir,int hSutun,int rhoKaydir,
                    struct variable *veri,int sayac,struct dogru *dogruBul);
void GrafikCizGnuplotcugumScript(struct dogru *dogruBul,int dogruKac,
                               struct variable *veri,int sayac, struct datas *data);
int compareVariables(const void *a, const void *b);

// Ana Fonksiyon (main)
int main() {
    PI=3.14159265358979323846;

    // yavas yavas her seyi koda tasiyip test etmek icin boyle bir sey denedik
    //o kadar ugrasdikdan sonra olmamasi kotu olurdu
    const char* TomlAmaKod =
        "[header]\n"
        "stamp = \"2025-10-01T12:00:00\"\n"
        "frame_id = \"laser_frame\"\n"
        "\n"
        "[scan]\n"
        "angle_min = 0.0\n"
        "angle_max = 4.71238898\n"
        "angle_increment = 0.0174533\n"
        "\n"
        "time_increment = 0.0\n"
        "scan_time = 0.1\n"
        "\n"
        "range_min = 0.2\n"
        "range_max = 3.0\n"
        "\n"
        "ranges = [\n"
        "  1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,\n"
        "  -1.0, -1.0, 999.0, -999.0, -1.0,\n"
        "  1.5, 1.51, 1.49, 1.52, 1.48, 1.53, 1.47, 1.54, 1.46, 1.55,\n"
        "  1.45, 1.56, 1.44, 1.57, 1.43, 1.58, 1.42, 1.59, 1.41, 1.6,\n"
        "  -1.0, -1.0, -1.0, 1.5, 1.51, 1.49, 1.52, 1.48, 1.53, 1.47, 1.54, 1.46, 1.55,\n"
        "  1.45, 1.56, 1.44, 1.57, 1.43, 1.58, 1.42, 1.59,\n"
        "  -1.0, -1.0, -1.0,1.4, 1.41, 1.39, 1.42, 1.38, 1.43, 1.37, 1.44, 1.36, 1.45,\n"
        "  1.35, 1.46, 1.34, 1.47, 1.33, -1.0, -1.0, -1.0,\n"
        "  1.6, 1.61, 1.59, 1.62, 1.58, 1.63, 1.57, 1.64, 1.56, 1.65,\n"
        "  1.55, 1.66, -1.0, -1.0, -1.0,\n"
        "  1.3, 1.31, 1.29, 1.32, 1.28, 1.33, 1.27, 1.34, 1.26, 1.35,\n"
        "  -1.0, -1.0, -1.0, 1.7, 1.71, 1.69, 1.72, 1.68, 1.73, 1.67, 1.74,\n"
        "  2.8, 1.1, 2.2, 1.9, 2.5, 1.2, 2.1, 1.8, 2.7, 1.3,\n"
        "  2.4, 1.6, 2.9, 1.0, 2.3, 1.7, 2.6, 1.4, 2.0, 1.5,\n"
        "  2.8, 1.2, 2.1, 1.9, 2.4, 1.6, 2.7, 1.3, 2.0, 1.0,\n"
        "  2.5, 1.8, 2.9, 1.4, 2.2, 1.7, 2.6, 1.1, 2.3, 1.5,\n"
        "  -1.0, 999.0, 0.1, 3.8, -1.0, -999.0, 0.15, 4.2,\n"
        "  -1.0, 999.0, 0.05, 3.9, -1.0, 4.3, -999.0, 0.19,\n"
        "  -1.0, 999.0, 3.5, 0.08, -1.0, 4.1, -999.0, 0.12,\n"
        "  1.8, 2.6, 1.4, 2.2, 1.1, 2.9, 1.7, 2.5, 1.3, 2.1,\n"
        "  1.9, 2.7, 1.5, 2.3, 1.2, 2.8, 1.6, 2.4, 1.0, 2.0,\n"
        "  2.5, 1.7, 2.1, 1.3, 2.8, 1.9, 2.4, 1.5, 2.0, 1.1,\n"
        "  1.8, 2.6, 1.4, 2.2, 1.0, 2.8, 1.6, 2.4, 1.2, 2.0,\n"
        "  -1.0, 999.0, -1.0, -999.0, -1.0, 999.0, -1.0, -1.0,\n"
        "  0.05, 3.9, 0.18, 4.6, 0.11, 3.1, 0.16, 4.8,\n"
        "  -1.0, 999.0, -1.0, -999.0, -1.0, 999.0, -1.0, -1.0,\n"
        "  2.2, 1.8, 2.6, 1.4, 2.0, 1.6, 2.8, 1.2, 2.4, 1.0,\n"
        "  1.9, 2.7, 1.5, 2.3, 1.1, 2.9, 1.7, 2.5, 1.3, 2.1,\n"
        "  2.5, 1.7, 2.1, 1.3, 2.8, 1.9, 2.4, 1.5, 2.0, 1.1,\n"
        "  1.8, 2.6, 1.4, 2.2, 1.0, 2.8, 1.6, 2.4, 1.2, 2.0,\n"
        "  2.3, 1.9, 2.7, 1.5, 2.1, 1.1, 2.9, 1.7, 2.5, 1.3,\n"
        "  1.6, 2.4, 1.2, 2.8, 1.8, 2.2, 1.4, 2.6, 1.0, 2.0,\n"
        "  -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0,\n"
        "  -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0,\n"
        "  -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0,\n"
        "  -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0\n"
        "]\n"
        "\n"
        "intensities = []\n";

    FILE *dosya;

    struct variable veri[1000];
    struct datas data;
    struct dogru dogruBul[50];
    int dogruKac=0;
    int secim;
    char* linkler[7]={"http://abilgisayar.kocaeli.edu.tr/lidar1.toml",
                       "http://abilgisayar.kocaeli.edu.tr/lidar2.toml",
                       "http://abilgisayar.kocaeli.edu.tr/lidar3.toml",
                       "http://abilgisayar.kocaeli.edu.tr/lidar4.toml",
                       "http://abilgisayar.kocaeli.edu.tr/lidar5.toml",
                       "",
                       "https://drive.google.com/uc?export=download&id=1qt_wuW13oDUw0EFXX8FkmLvUrnatu1Nc"};

    char* dosyalar[7]={"lidar1.toml",
                        "lidar2.toml",
                        "lidar3.toml",
                        "lidar4.toml",
                        "lidar5.toml",
                        "scan_data_user.toml",
                        "lidar_gdrive.toml"};



    printf("--- Otonom Robot Docking Projesi ---\n");
    printf("Hangi dosyayi analiz edeyim? (1-5: Online, 6: Kullanici Verisi (scan_data_NaN), 7: Google Drive Verisi): ");
    if (scanf("%d",&secim)!=1 || secim<1 || secim>7) {
        printf("HATA: 1-7 arasi bir sayi girmeliydin.\n"); return 1;
    }

    char dosyaIsim[50];
    if (secim == 6) {
        strcpy(dosyaIsim, dosyalar[secim-1]);
        printf("\nKullanicinin 'scan_data_NaN (2).toml' verisi '%s' dosyasina yaziliyor...\n", dosyaIsim);
        FILE *secilenDosya = fopen(dosyaIsim, "w");
        if (!secilenDosya) {
            printf("HATA: Gecici dosya '%s' olusturulamadi!\n", dosyaIsim);
            return 1;
        }
        fprintf(secilenDosya, "%s", TomlAmaKod);
        fclose(secilenDosya);
        printf("Kullanici verisi dosyaya yazildi.\n\n");

    } else {
        char* dosyaLinki = linkler[secim - 1];
        strcpy(dosyaIsim, dosyalar[secim - 1]);

        char komut[512];
        sprintf(komut,"curl -L -o %s \"%s\"",dosyaIsim,dosyaLinki);
        /*cmd de bizim kodun calismasi icin komutu yukardan alip system() ile katiyoruz
        curl kutuphanesinden indirecegiz diye curl...
        nerden indirecek ne diye save yapcak redirect falan var mi diye kontrol ediyor ...
        redirect olmicakta yine de ne olur ne olmaz*/
        printf("\nDosya indiriliyor: %s\n",dosyaLinki);
        int ind_sonuc=system(komut);//cmd de bizim kodun calismasi icin komutu yukardan alip system() ile katiyoruz
        if (ind_sonuc!=0) {
            printf("HATA: Dosya indirilemedi. 'curl' kurulu mu veya internet var mi?\n"); return 1;
        }
        printf("Dosya '%s' olarak indi.\n\n",dosyaIsim);
    }

    dosya=fopen(dosyaIsim,"r");
    if(!dosya)
        { printf("HATA : dosya acilamadi! (%s)\n", dosyaIsim); return 0; }
    printf("Dosya okumak icin acildi: %s\n",dosyaIsim);
    DataYerlestirme(&data,dosya);
    int Sayac=RangeCikarici(dosya,veri);
    printf("Dosyadan %d adet nokta okundu.\n",Sayac);
    RangeArrangement(veri,data,Sayac);
    fclose(dosya);

    int hSatir,hSutun,rhoKaydir;
    int** oylarMatrisi=houghuUygula(veri,Sayac,data,&hSatir,&hSutun,&rhoKaydir);
    //iste noktalari yerlestirip matrisi tanimliyo
    if (oylarMatrisi==NULL)
    {
        printf("Hough fonksiyonunda hata olustu!\n"); return 1;
    }
    //burda da dogrulari buluyor ,fonksiyonun ne yaptigini yerinde anlatacagim ama iste noktalari sayip gosteriyo
    dogruKac=DogrulariCikar(oylarMatrisi,hSatir,hSutun,rhoKaydir,veri,Sayac,dogruBul);
    printf("%d adet dogru parcasi bulundu.\n",dogruKac);

    GrafikCizGnuplotcugumScript(dogruBul,dogruKac,veri,Sayac, &data);

    printf("\n'oylarMatrisi' hafizasi temizleniyor...\n");
    if (oylarMatrisi != NULL)
        {
        for (int y=0; y<hSatir; y++)
        {
             if (oylarMatrisi[y] != NULL)
             {
                free(oylarMatrisi[y]);
             }
        }
        free(oylarMatrisi);
        printf("Hafiza temizlendi.\n");
    }
    else
    {
        printf("Hafiza zaten temizlenmis veya ayrilamamis.\n");
    }
    printf("Program sonlaniyor.\n");

    return 0;
}

void RangeArrangement(struct variable *veri,struct datas data,int sayac) {
    printf("Noktalar filtreleniyor ve (x,y) koordinatlarina donusturuluyor...\n");
    for(int i=0; i<sayac; i++)
    {
        veri[i].hangiIndex = i;
        //cok gereken bir sey degildi zaten vari[i] ordaki i ...
        double derece=data.angle_min+i*data.angle_increment;
        veri[i].x=veri[i].veri*cos(derece);
        veri[i].y=veri[i].veri*sin(derece);

        if( !isnan(veri[i].veri) && !isinf(veri[i].veri) &&
            veri[i].veri > data.range_min && veri[i].veri < data.range_max)
            veri[i].gecerli=1;
        else
            veri[i].gecerli=0;
    }
}

//DogrulariCikar Fonksiyonu (minNoktaSayi=8, rho=0.05, NMS=-15..+15, der=0.55) ***
int DogrulariCikar(int** oylarMatrisi, int hSatir, int hSutun, int rhoKaydir,
                    struct variable *veri, int sayac, struct dogru *dogruBul)
{

    const int minNoktaSayi = 8;
    const double minUzunluk = 0.05;
    const int minGap = 3;

    int dogruKac = 0;
    const double houghHassasDer = 0.55;
    const double houghHassasRho = 0.05;
    const double houghAdimRad = houghHassasDer * (PI / 180.0);

    struct variable *noktalarKumesi = (struct variable *)malloc(sayac * sizeof(struct variable));
    if (!noktalarKumesi) {
        printf("Hata: Nokta kumesi icin hafiza ayrilamadi!\n");
        return 0;
    }

    printf("En yuksek oyu alan dogru segmentleri araniyor (esik >= %d, minimum uzunluk=%.2fm, maximum gap=%d)...\n",
           minNoktaSayi, minUzunluk, minGap);

    for (int y = 0; y < hSatir; y++)
    {
        for (int i = 0; i < hSutun; i++)
        {
            if (oylarMatrisi[y][i] >= minNoktaSayi)
                {
                int kumedeNoktaKac = 0;
                double radAci = i * houghAdimRad;
                for (int numara = 0; numara < sayac; numara++)
                {
                    if (!veri[numara].gecerli)
                        continue;
                    double x = veri[numara].x;
                    double y_cordinat = veri[numara].y;
                    double noktaRhosu = x * cos(radAci) + y_cordinat * sin(radAci);
                    int noktaRhoID = (int)round(noktaRhosu / houghHassasRho) + rhoKaydir; // rho=0.05

                    if (abs(noktaRhoID - y) <= 2)
                    {
                        if (kumedeNoktaKac < sayac)
                        {
                            noktalarKumesi[kumedeNoktaKac++] = veri[numara];
                        }
                    }
                }

                if (kumedeNoktaKac >= minNoktaSayi)
                    {
                    qsort(noktalarKumesi, kumedeNoktaKac, sizeof(struct variable), compareVariables);
                    int segmentIlkIdx = 0;
                    for (int j = 1; j <= kumedeNoktaKac; j++) {
                        int t = (j == kumedeNoktaKac);
                        int gap = t ? (minGap + 1) : (noktalarKumesi[j].hangiIndex - noktalarKumesi[j - 1].hangiIndex);

                        if (gap > minGap || t)
                            {
                            int segmentSonIdx = j - 1;
                            int segmentNoktaKac = segmentSonIdx - segmentIlkIdx + 1;
                            if (segmentNoktaKac >= minNoktaSayi)
                                {
                                double endX = noktalarKumesi[segmentSonIdx].x;
                                double endY = noktalarKumesi[segmentSonIdx].y;
                                double startX = noktalarKumesi[segmentIlkIdx].x;
                                double startY = noktalarKumesi[segmentIlkIdx].y;

                                double segUzunluk = hypot(endX - startX, endY - startY);
                                if (segUzunluk >= minUzunluk)
                                {
                                    struct dogru yeniDogru;
                                    yeniDogru.ilkX = startX;
                                    yeniDogru.ilkY = startY;
                                    yeniDogru.sonX = endX;
                                    yeniDogru.sonY = endY;
                                    if (fabs(yeniDogru.sonX - yeniDogru.ilkX) < 0.01)
                                    {
                                        yeniDogru.dikey_mi = 1;
                                        yeniDogru.a = 0;
                                        yeniDogru.b = 0;
                                    }

                                    else
                                    {
                                        yeniDogru.dikey_mi = 0;
                                        yeniDogru.a = (yeniDogru.sonY - yeniDogru.ilkY) / (yeniDogru.sonX - yeniDogru.ilkX);
                                        yeniDogru.b = yeniDogru.ilkY - yeniDogru.a * yeniDogru.ilkX;
                                    }
                                    if (dogruKac < 50)
                                    {
                                        dogruBul[dogruKac++] = yeniDogru;
                                    } else {
                                        printf("Uyari: Maksimum dogru sayisi (50) asildi!\n");
                                        free(noktalarKumesi);
                                        return dogruKac;
                                    }
                                }
                            }
                            segmentIlkIdx = j;
                        }
                    }
                }
                for (int dy = -15; dy <= 15; ++dy) {
                    for (int di = -15; di <= 15; ++di) {
                        int ny = y + dy;
                        int ni = (i + di + hSutun) % hSutun;
                        if (ny >= 0 && ny < hSatir)
                        {
                             oylarMatrisi[ny][ni] = 0;
                        }
                    }
                }
            }
        }
    }

    free(noktalarKumesi);
    return dogruKac;
}

int** houghuUygula(struct variable *veri,int sayac,struct datas data,
                   int *houghSatirinPtr,int *houghSutununPtr,int *rhoKaydirPtr)
{
    printf("\n--- Hough Donusumu Baliyor ---\n");
    const double houghHassasDer = 0.55;
    const double houghAdimRad=houghHassasDer*(PI/180.0);
    const int houghSutun=(int)ceil(PI/houghAdimRad);
    const double houghHassasRho = 0.05;
    const double max_rho = data.range_max > 0 ? data.range_max : 10.0;
    const int rhoKaydir = (int)ceil(max_rho / houghHassasRho);
    const int houghSatir = (rhoKaydir * 2) + 1; // Yerel degisken adi 'houghSatir'

    *houghSatirinPtr=houghSatir; *houghSutununPtr=houghSutun; *rhoKaydirPtr=rhoKaydir;
    printf("Oy Tablosu Boyutlari: %d Satir x %d Sutun\n",houghSatir,houghSutun);

    double *sin_tablosu = (double*)malloc(houghSutun * sizeof(double));
    double *cos_tablosu = (double*)malloc(houghSutun * sizeof(double));
    if (!sin_tablosu || !cos_tablosu) { printf("Hata: sin/cos tablosu icin hafiza ayrilamadi!\n"); return NULL; }
    for (int i=0; i<houghSutun; i++) {
        double aci=i*houghAdimRad;
        sin_tablosu[i]=sin(aci); cos_tablosu[i]=cos(aci);
    }

    int** oylarMatrisi = (int**)malloc(houghSatir * sizeof(int*));
    if (!oylarMatrisi) { printf("Hata: Oy tablosu satirlari icin hafiza ayrilamadi!\n"); free(sin_tablosu); free(cos_tablosu); return NULL; }
    printf("Oy tablosu icin hafiza ayriliyor (Satirlar)...\n");
    for (int y=0; y<houghSatir; y++) { // 'houghSatir' kullanilmali
        oylarMatrisi[y]=(int*)calloc(houghSutun, sizeof(int));
        if (!oylarMatrisi[y])
        {
            printf("Hata: Oy tablosu sutunlari icin hafiza ayrilamadi (satir %d)!\n", y);
            for(int k=0; k<y; k++) free(oylarMatrisi[k]);
            free(oylarMatrisi); free(sin_tablosu); free(cos_tablosu); return NULL;
        }
    }
    printf("Oy tablosu (akumulator) hafizaya kuruldu.\n");

    printf("Oylama basliyor...\n");
    for (int numara=0; numara<sayac; numara++)
    {
        if (!veri[numara].gecerli)
            continue;
        double x=veri[numara].x;
        double y_kordinat=veri[numara].y;
        for (int i=0; i<houghSutun; i++)
        {
            double hesap_rho=x*cos_tablosu[i]+y_kordinat*sin_tablosu[i];
            int y=(int)round(hesap_rho / houghHassasRho) + rhoKaydir;
            if (y>=0 && y<houghSatir)
                oylarMatrisi[y][i]++;
        }
    }
    printf("Oylama bitti.\n");

    free(sin_tablosu);
    free(cos_tablosu);
    return oylarMatrisi;
}

// *** Gnuplot Script Olusturma Fonksiyonu (lw=1.5, Manuel Lejant, 'black' ve 'replot' duzeltmesi) ***
void GrafikCizGnuplotcugumScript(struct dogru *dogruBul, int dogruKac,
                               struct variable *veri, int sayac, struct datas *data) {

    char *original_locale = setlocale(LC_NUMERIC, NULL);
    setlocale(LC_NUMERIC, "C");

    FILE *gpScript = fopen("commands.gp", "w");
    if (!gpScript) {
        perror("Hata: Gnuplot script dosyasi (commands.gp) olusturulamadi");
        setlocale(LC_NUMERIC, original_locale);
        return;
    }
    printf("\nGnuplot komutlari 'commands.gp' dosyasina yaziliyor (C locale ile)...\n");

    // --- Gnuplot Ayarlari ---
    fprintf(gpScript, "set terminal pngcairo size %d,%d enhanced font 'Arial,10'\n", EkranBoyut, EkranBoyut);
    fprintf(gpScript, "set output 'grafik_c_gnuplot.png'\n");
    fprintf(gpScript, "set title 'LIDAR Taramasi (C Kodu / Gnuplot)'\n");
    fprintf(gpScript, "set xlabel 'X Koordinati (metre)'\n");
    fprintf(gpScript, "set ylabel 'Y Koordinati (metre)'\n");
    fprintf(gpScript, "set grid\n");
    fprintf(gpScript, "set size ratio -1\n");
    fprintf(gpScript, "unset key\n");

    // Eksen Limitleri Sabitlendi (-4m ile +4m)
    double limit = 4.0;
    fprintf(gpScript, "set xrange [-%f:%f]\n", limit, limit);
    fprintf(gpScript, "set yrange [-%f:%f]\n", limit, limit);

    // --- Kesisimleri ve Mesafe Etiketini Onceden Ayarla ---
    fprintf(gpScript, "unset label\n");
    fprintf(gpScript, "unset arrow\n");
    fprintf(gpScript, "unset object\n"); // Tum object'leri temizle

    double kesisimlerX[50*50];
    double kesisimlerY[50*50];
    int gecerliKesisim = 0;
    double ilkKesisimX = NAN, ilkKesisimY = NAN, ilkKesisimMesafe = NAN;

    for (int i = 0; i < dogruKac; i++) {
        for (int k = i + 1; k < dogruKac; k++) {
            double kesisimX, kesisimY, aci;
            if (KesisimKontrol(dogruBul[i], dogruBul[k], &kesisimX, &kesisimY, &aci)) {
                if (aci >= 60.0) {
                    if (isnan(kesisimX) || isinf(kesisimX) || isnan(kesisimY) || isinf(kesisimY)) continue;

                    if (gecerliKesisim < 50*50) {
                        kesisimlerX[gecerliKesisim] = kesisimX;
                        kesisimlerY[gecerliKesisim] = kesisimY;
                        gecerliKesisim++;

                        if(isnan(ilkKesisimX)) {
                            ilkKesisimX = kesisimX;
                            ilkKesisimY = kesisimY;
                            ilkKesisimMesafe = hypot(kesisimX, kesisimY);
                        }
                    }
                }
            }
        }
    }
    // Ilk kesisim icin mesafe etiketini ayarliyor...
    if(!isnan(ilkKesisimX)) {
        fprintf(gpScript, "set label %d '%.2fm' at %f,%f textcolor rgb '#FF0000' font ',9' center offset 0,char 1\n",
                999, ilkKesisimMesafe, ilkKesisimX / 2.0, ilkKesisimY / 2.0);
    }

    //Manuel Lejanti Ayarla
    double lejantXSimge = 0.83;
    double lejantXMetin = 0.85;
    double lejantYBaslangic = 0.95;
    double lejantYAdim = 0.025;
    double lejantY = lejantYBaslangic;

    // Cerceve
    double lejantMinY = lejantYBaslangic - ( (gecerliKesisim > 0 ? 2 : 0) + (dogruKac > 0 ? 1 : 0) + 2 + dogruKac * 0.8) * lejantYAdim;
    if (lejantMinY < 0.1) lejantMinY = 0.1;
    fprintf(gpScript, "set object 1 rectangle from graph 0.8, graph %f to graph 0.98, graph 0.97 fillstyle empty border lc rgb '#000000'\n", lejantMinY);

    // Robot
    fprintf(gpScript, "set object 2 circle at graph %f, graph %f size scr 0.008 fillcolor rgb '#FF0000' fs solid border lc rgb '#FF0000'\n", lejantXSimge, lejantY);
    fprintf(gpScript, "set label 10 'Robot' at graph %f, graph %f font ',9' textcolor rgb '#000000'\n", lejantXMetin, lejantY);
    lejantY -= lejantYAdim;
    // Tum Noktalar
    fprintf(gpScript, "set object 3 circle at graph %f, graph %f size scr 0.004 fillcolor rgb '#ADD8E6' fs solid border lc rgb '#ADD8E6'\n", lejantXSimge, lejantY);
    fprintf(gpScript, "set label 11 'Tum Noktalar' at graph %f, graph %f font ',9' textcolor rgb '#000000'\n", lejantXMetin, lejantY);
    lejantY -= lejantYAdim;
    // Kesisim (varsa)
    if(gecerliKesisim > 0) {
        fprintf(gpScript, "set label 13 '60+ Kesisim' at graph %f, graph %f font ',9' textcolor rgb '#000000' point pointtype 2 pointsize 1.5 lc rgb '#FFFF00'\n",
                lejantXMetin, lejantY);
        lejantY -= lejantYAdim;
    }
    // Mesafe Cizgisi (varsa)
    if(!isnan(ilkKesisimX)) {
        fprintf(gpScript, "set arrow 5 from graph %f, graph %f to graph %f, graph %f lw 1.5 lc rgb '#FF0000' dashtype 2 nohead\n",
                lejantXSimge - 0.01, lejantY, lejantXSimge + 0.01, lejantY);
        fprintf(gpScript, "set label 14 'Mesafe Cizgisi' at graph %f, graph %f font ',9' textcolor rgb '#000000'\n", lejantXMetin, lejantY);
        lejantY -= lejantYAdim;
    }
    // Cizgi Listesi Basligi
    if (dogruKac > 0) {
        fprintf(gpScript, "set label 12 'Tespit Edilen Cizgiler:' at graph %f, graph %f font ',9' textcolor rgb '#000000'\n", lejantXMetin, lejantY);
        lejantY -= (lejantYAdim * 0.8);
    }
    // Cizgiler (d1, d2...)
    for (int i = 0; i < dogruKac; i++) {
        if (lejantY < 0.1) break;
        fprintf(gpScript, "set arrow %d from graph %f, graph %f to graph %f, graph %f lw 1.5 lc rgb '#228B22' nohead\n",
                100+i, lejantXSimge - 0.01, lejantY, lejantXSimge + 0.01, lejantY);
        fprintf(gpScript, "set label %d 'd%d' at graph %f, graph %f font ',8' textcolor rgb '#228B22'\n",
                200+i, i+1, lejantXMetin, lejantY);
        lejantY -= (lejantYAdim * 0.8);
    }

    // Veri Bloklarini Once Yaz
    fprintf(gpScript, "$allpoints << EOD\n");
    int pointSayi = 0;
    for (int i = 0; i < sayac; i++) {
        if (!isnan(veri[i].x) && !isinf(veri[i].x) && !isnan(veri[i].y) && !isinf(veri[i].y)) {
             fprintf(gpScript, "%f %f\n", veri[i].x, veri[i].y);
             pointSayi++;
        }
    }
    fprintf(gpScript, "EOD\n\n");

    fprintf(gpScript, "$lines << EOD\n");
    int satirSayisi = 0;
    for (int i = 0; i < dogruKac; i++) {
        if (!isnan(dogruBul[i].ilkX) && !isinf(dogruBul[i].ilkX) && !isnan(dogruBul[i].ilkY) && !isinf(dogruBul[i].ilkY) &&
            !isnan(dogruBul[i].sonX) && !isinf(dogruBul[i].sonX) && !isnan(dogruBul[i].sonY) && !isinf(dogruBul[i].sonY))
        {
            fprintf(gpScript, "%f %f\n%f %f\n\n",
                    dogruBul[i].ilkX, dogruBul[i].ilkY,
                    dogruBul[i].sonX, dogruBul[i].sonY);
            satirSayisi++;
        }
    }
    fprintf(gpScript, "EOD\n\n");

    fprintf(gpScript, "$robot << EOD\n0 0\nEOD\n\n");

    if (gecerliKesisim > 0)
        {
        fprintf(gpScript, "$intersections << EOD\n");
        for(int i = 0; i < gecerliKesisim; ++i)
             fprintf(gpScript, "%f %f\n", kesisimlerX[i], kesisimlerY[i]);
        fprintf(gpScript, "EOD\n\n");

        if(!isnan(ilkKesisimX)) {
             fprintf(gpScript, "$intersection_dist_line << EOD\n");
             fprintf(gpScript, "0 0\n%f %f\n\n", ilkKesisimX, ilkKesisimY);
             fprintf(gpScript, "EOD\n\n");
        }
    }

    if (pointSayi == 0 && satirSayisi == 0 && gecerliKesisim == 0)
    {
        fprintf(gpScript, "plot $robot using 1:2 with points pt 6 ps 2 lc rgb '#FF0000' notitle\n");
        printf("Uyari: Cizilecek gecerli nokta veya cizgi bulunamadi, sadece robot ciziliyor.\n");
    }
    else
    {
        fprintf(gpScript, "plot ");
        if (pointSayi > 0)
        {
            fprintf(gpScript, "$allpoints using 1:2 with points pt 7 ps 0.3 lc rgb '#ADD8E6' notitle, \\\n");
        } else
        {
             fprintf(gpScript, "NaN notitle, \\\n");
        }
        if (satirSayisi > 0)
        {
            fprintf(gpScript, "     $lines using 1:2 with lines lw 1.5 lc rgb '#228B22' notitle, \\\n");
        } else {
             fprintf(gpScript, "     NaN notitle, \\\n");
        }
        fprintf(gpScript, "     $robot using 1:2 with points pt 6 ps 2 lc rgb '#FF0000' notitle");

        if (gecerliKesisim > 0)
        {
            fprintf(gpScript, ", \\\n     $intersections using 1:2 with points pt 2 ps 1.5 lc rgb '#FFFF00' notitle");
            if(!isnan(ilkKesisimX))
            {
                fprintf(gpScript, ", \\\n     $intersection_dist_line using 1:2 with lines lt 1 lw 1.5 lc rgb '#FF0000' dashtype 2 notitle");
            }
        }
        fprintf(gpScript, "\n"); // Plot komutunu bitir
    }

    // Dosyayi Kapat ve Locale'i Geri Yukle
    fclose(gpScript);
    setlocale(LC_NUMERIC, original_locale);

    printf("Gnuplot komutlari 'commands.gp' dosyasina yazildi.\n");
    printf("PNG olusturmak icin terminalden 'gnuplot commands.gp' komutunu calistirin.\n");
    printf("Gnuplot'un verecegi hatalari terminalde gorebilirsiniz.\n");
}

// ... (Diger tum fonksiyonlar - Esit, ..., KesisimKontrol, DereceKac, compareVariables - ayni kalir) ...
int Esit(char BirString[],char BaskaBirString[],int Uzunluk) {
    while (*BirString==' ' || *BirString=='\t')
        BirString++;
    return (strncmp(BirString,BaskaBirString,Uzunluk))?0:1;
     //bildigimiz her seyi kullanmakdir burdaki amac yegen ,tabi bu -1 de olabilir maksat
     //bu tur ifi kullanmaktir yegen 0 oldugu zaman yani esitlik oldugu zaman 1 verecek bize
}


double SonParametreBulucu(FILE *dosya,char String[],int Uzunluk) {
    char data[300];
    // dosya basa donsun...
    rewind(dosya);
    while(1) {
        if(!fgets(data,300,dosya))
            break;
        if(Esit(data,String,Uzunluk)) {
            char *ptr=strchr(data,'=');
            //strchr char arar o chari data icinde arar burda mesela...
            if(ptr)
            return atof(ptr+1);
            //yukardaki kodu yazmazsak bize 0 verdigi icin kod calismoyordu
            //(cunku atoi stringi okuyo bir harf gordugunde rakami almayi birakiyo yani ne demek
            //direk rakamdan baslamasi lazim eee napak?)bende sayaci one cektim ki = den sonrasini okusun...
        }
        if(feof(dosya)) break;
    }
    return -1;
}

int RangeCikarici(FILE *dosya,struct variable *a)
{
    /*kafamdaki kivilcim:
    ya double *list dicez o da o listeyi dondurecek falan malloc ile ugrascaz
    yada struct kullancaz yer kaplama olarak da ayni diyordum ama iler oyle bir sey yasandi ki
    ,bu secimden 100% razi dusurdu beni oynat bakalim:...*/
    int sayac=0;
    char data[300];
    while(fgets(data,300,dosya))
    {
        char *pointer=strchr(data,'[');
        if(pointer)
        {
            pointer++;
            while(1)
            {
                char *bolum=strtok(pointer,",");
                while(bolum!=NULL)
                {
                    while (*bolum==' ' || *bolum=='\t')
                        bolum++;
                    char *temp=strchr(bolum,']');
                    if (temp)
                    {
                        *temp='\0';
                        if (*bolum!='\0')
                            a[sayac++].veri=atof(bolum);
                        return sayac;
                    }
                    a[sayac++].veri=atof(bolum);
                    bolum=strtok(NULL,",");
                }
                if(!fgets(data,300,dosya)) return sayac;
                pointer=data;
            }
        }
    }
    return sayac;
    //tam burda dedim ki ben simdi noktalarin x,y sini cikartmam lazim,
    //eee nerde onlari depolicam??struct burda beni kurtardi:)

}

void DataYerlestirme(struct datas *data,FILE *dosya)
{
    data->angle_min=SonParametreBulucu(dosya,"angle_min",9);
    //bunu bilmeyen yoktur ama yinede yazayim (*d).a=d->a
    //bu stract ustunde oynicagimiz icin *d olmasi lazim ve...
    data->angle_max=SonParametreBulucu(dosya,"angle_max",9);
    data->angle_increment=SonParametreBulucu(dosya,"angle_increment",15);
    data->range_min =SonParametreBulucu(dosya,"range_min",9);
    data->range_max=SonParametreBulucu(dosya,"range_max",9);
    if (data->angle_increment > 1e-9)
        data->ElemanSayisi=(int)round((data->angle_max-data->angle_min)/data->angle_increment+1);
        //Eleman sayisi ≈ (angle_max - angle_min) /angle_increment + 1
        //bi ustunu alsin diye mathdan roundu aldim :)
    else
        data->ElemanSayisi = 0;
        //ilk basta bu yoktu biraz daha temiz olsun diye
        // fonksiyonada yerlestirme isini yapiyor a->b yi ogrendigimizi de gosteriyir
}

int arasindami(double a,double b,double c) {
    return ((b>=a && b<=c) || (b<=a && b>=c))?1:0;
}

int KesisimKontrol(struct dogru bir,struct dogru iki,double *kesisimX,double *kesisimY,double *aci) {
    if (bir.dikey_mi && iki.dikey_mi)
        return 0;
    else if (bir.dikey_mi)
    {
        *kesisimX = bir.ilkX;
        *kesisimY = iki.a * (*kesisimX) + iki.b;
    }

    else if (iki.dikey_mi)
    {
        *kesisimX = iki.ilkX;
        *kesisimY = bir.a * (*kesisimX) + iki.b;
    }

    else {
        if (fabs(bir.a-iki.a)<0.0001)
            return 0;
        *kesisimX=(iki.b-bir.b)/(bir.a-iki.a);
        *kesisimY=bir.a*(*kesisimX)+bir.b;
    }
    if(arasindami(bir.ilkX,*kesisimX,bir.sonX) && arasindami(bir.ilkY,*kesisimY,bir.sonY) &&
       arasindami(iki.ilkX,*kesisimX,iki.sonX) && arasindami(iki.ilkY,*kesisimY,iki.sonY))
    {
        *aci=DereceKac(bir,iki);
        return 1;
    }
    return 0;
}

double DereceKac(struct dogru bir,struct dogru iki)
{
    //formul:  180/PI *(V.U)/Vnorm*Unorm arccos=TETA(DERECEMIZ)
    //hypot fonksiyonu math kutuphanesinden geliyor vektorlerin olcumunu aliyor->kok(a kare+b kare)...
    double boyBir=hypot(bir.sonX-bir.ilkX,bir.sonY-bir.ilkY);
    double boyIki=hypot(iki.sonX-iki.ilkX,iki.sonY-iki.ilkY);
    if (boyBir < 1e-9 || boyIki < 1e-9) return 0;
    double icCarpim=(bir.sonX-bir.ilkX)*(iki.sonX-iki.ilkX) + (bir.sonY-bir.ilkY)*(iki.sonY-iki.ilkY);
    double kosTeta=icCarpim/(boyBir*boyIki);
    if (kosTeta>1.0)
        kosTeta=1.0;
    else if (kosTeta <-1.0)
        kosTeta =-1.0;
    double derece=acos(kosTeta)*180.0/PI;
    //burasida belli iste iki dogru kesistiginde ortaya 2derece cikar a ve b ki bunlarin da toplami 180...
    if (derece>90.0)
        derece=180.0-derece;
    return derece;
}

int compareVariables(const void *a, const void *b)
{
    struct variable *varA = (struct variable *)a;
    struct variable *varB = (struct variable *)b;
    return (varA->hangiIndex - varB->hangiIndex);
}
