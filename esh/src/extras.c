/* 

this file contains functions for extra purposes
for example gethostname or user name etc.

*/

#include "extras.h"
#include <unistd.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// bilgisayardaki aktif kullanıcıyı verir
char *getusername(void) {
    struct passwd *pw;
    uid_t uid;
    uid = geteuid();
    pw = getpwuid(uid);

    if (pw) {
        return pw->pw_name;
    } return NULL;
}


// bilgisayar adını verir
char *getdevicename(void) {

    // maclerde bu komut ile bilgisayar adı alınır
    const char *command = "scutil --get ComputerName";

    // popen - initiate pipe streams to or from a process diyor man page
    // dikkat et kapatırken fclose değil pclose. ama bu kullanım hoşuma gitmedi
    // ileride değiştir çünkü galiba processin çalıştığı shellde komutun outputunu
    // buraya döndürüyor.
    FILE *f = popen("scutil --get ComputerName", "r");

    const char *unk = "unknown";
    char *device = (char *)malloc(sizeof(char) * 256);

    if (!device) {
        if (f != NULL) {
            pclose(f);
        }
        return strdup(unk);    // return unk desen stack dönmeye çalışır hata 
                               // strdup char * ı heap e kopyalar ve char * döndürür
    }

    if (!f) {
        strncpy(device, unk, 256);
        // strcpy kullanmadık çünkü overflow riski var güvenli değil
    } else {
        if (fgets(device, 256, f) == NULL) {
            strncpy(device, unk, 256);
            device[255] = '\0';
        } else {
            device[strcspn(device, "\r\n\t")] = '\0';  // bu karakterlerin olduğu ilk indeksi verir strcspn
        }
        pclose(f);
    }

    return device;
}