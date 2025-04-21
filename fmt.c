#include <stdio.h>
#include <stdlib.h>

static void mainMsg(){
    printf("mainMsg");
    exit(0);
}

static void hackMe(){
    printf("That's impossible!!!\n");
}

static void readPayload(unsigned char* buf){
    unsigned int num = 0;
    FILE *f;
    const char* filename = "payload.bin";

    printf("How many bytes should I read from your %s?\n", filename);
    scanf("%u", &num);

    f = fopen(filename, "rb");
    if (f){
        fread(buf, sizeof(unsigned char), num, f);
        fclose(f);
    } else{
        printf("File [%s] not found\n", filename);
    }
}

static void echo(void){
    unsigned char buf[32];
    int num = 0;

    printf("Enter a string: ");
    gets(buf);
    printf(buf);
    puts("\n");

    readPayload(buf);
}

int main(){
    echo();
    return 0;
}