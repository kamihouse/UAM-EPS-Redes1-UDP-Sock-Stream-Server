#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void name1(char *host1){
    char delim[] = ".";
    int i = 0;
    char *token;
    char buf[99];
    int length;

    token = strtok(host1, delim);
    buf[0] = '\0';

    while (token != NULL) {
        length = strlen(token);
        sprintf(buf+strlen(buf), "%d", length);
        sprintf(buf+strlen(buf), "%s", token);
        token = strtok(NULL, delim);
    }
    printf("%s", buf);
}

int main () {
    char host[] = "www.thiago.com.br";
    char delim[] = ".";
    int i = 0, index = 0;
    char *token;
    char buf[100];
    char len_char;
    int length;
    token = strtok(host, delim);
    buf[0] = '\0';

    while (token != NULL) {
        length = strlen(token);
        sprintf(buf+strlen(buf), "%d", length);
        sprintf(buf+strlen(buf), "%s", token);
        token = strtok(NULL, delim);
    }
    printf("Total: %d --> %s\n", (int) strlen(buf), buf);

    printf("\nTestando a Função:\n");
    printf("URL: ");
    char url[99];
    scanf("%s", url);
    // Usando la funcion
    name1(url);

    printf("\n");
    return 0;
}