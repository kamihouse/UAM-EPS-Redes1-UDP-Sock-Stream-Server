/*
 * UAM - EPS - Arquitectura de Redes I - Practica.
 * Profesores.:	Pedro María Santiago del Río (pedro.santiago@uam.es)
 *      	Javier Ramos (javier.ramos@uam.es)
 *
 * File.......: client_udp.c
 * Link.......: https://github.com/kamihouse/UDP-Sock-Stream-Server
 *
 * Author.....: Thiago Pereira Rosa	(thiago.pereirarosa@estudiante.uam.es)
 *              Isaac Gonzalez Gonzalez	(isaac.gonzalez@estudiante.uam.es)
 *
 * Ejemplo de sockets en Linux que mandan y reciben datagramas UDP
 * Created on 6 de Novembro de 2012, 17:56
 */

#include <stdio.h>
#include <stdlib.h>
// Estas cabeceras son necesarias para los 'sockets'
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>  // Usado por 'timeval'
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>  // Usado por 'close'

#include <string.h>  // Usado por 'strlen'

uint16_t id = 0x8521;
uint8_t buffer_query[1000]={0};
uint16_t aux=0;
uint32_t offset=0;
uint16_t nq=1;
uint16_t flags=0x0100;

char *leer(){
    char aux[1024];
    char *ip='\0';
    char *tmp,*tmp1;
    int lon;
    FILE *resolv;
    resolv=fopen("/etc/resolv.conf","r");
    if(resolv==NULL){
        printf("Error abriendo resolv.config\n");
    }

    while(fgets(aux,256,resolv)!=NULL){
        if ((strncmp(aux,"nameserver ",strlen("nameserver ")))==0){
            tmp=strtok(aux,"nameserver ");
            lon=strlen(tmp);
            tmp1=tmp + (lon -1);
            *tmp1='\0';
            ip=tmp;
        }
    }
    fclose(resolv);
    return ip;
}

char * nameHost(char *host1){
    char delimit[] = ".";
    char *token;
    char *buf=NULL;
    int length;
    buf = (char*) malloc (150*sizeof(char));

    token = strtok(host1, delimit);

    while (token != NULL) {
        length = strlen(token);
        buf[strlen(buf)]=length;
        sprintf(buf+strlen(buf), "%s", token);
        token = strtok(NULL, delimit);

    }
    return buf;
}

void mygethostbyname(char *dominio){
    char *name;
    aux=htons(id);
    memcpy(buffer_query,&aux,sizeof(uint16_t));
    offset+=sizeof(uint16_t);

    aux=htons(flags);
    memcpy(buffer_query+offset,&aux,sizeof(uint16_t));
    offset+=sizeof(uint16_t);

    aux=htons(nq);
    memcpy(buffer_query+offset,&aux,sizeof(uint16_t));
    offset+=sizeof(uint16_t);

    aux=0;
    memcpy(buffer_query+offset,&aux,sizeof(uint16_t));
    offset+=sizeof(uint16_t);

    aux=0;
    memcpy(buffer_query+offset,&aux,sizeof(uint16_t));
    offset+=sizeof(uint16_t);
    // respuesta autoritativa
    aux=0;
    memcpy(buffer_query+offset,&aux,sizeof(uint16_t));
    offset+=sizeof(uint16_t);

    // Cambiar a formato DNS el argv[1] -> www.uam.es-> 3www3uam2es0 -> name
    name = nameHost(dominio);

    memcpy(buffer_query+offset,name,strlen(name)+1);

    offset+=strlen(name)+1;

    //type
    aux=htons(nq);
    memcpy(buffer_query+offset,&aux,sizeof(uint16_t));
    offset+=sizeof(uint16_t);

    //class
    aux=htons(nq);
    memcpy(buffer_query+offset,&aux,sizeof(uint16_t));
    offset+=sizeof(uint16_t);

    free(name);
}

int main(int argc, char *argv[]){
    struct sockaddr_in  sock_addr, to_addr;
    socklen_t           sockaddrlen = sizeof(struct sockaddr);
    int                 sock, c, i,k=0,q=0,cq=0,answers=0,cont=0;
    struct timeval      tv;     // Usado para el 'timeout'
    fd_set              rfds;   // Usado por el 'select'
    unsigned char       buffer[1024];
    char ip[15], a[256], *nombre;

    fprintf( stdout, "--> Programa ejemplo que envia y recibe un datagrama...\n");

    strcpy(ip,leer());
    printf("\nServer: \t%s\n",ip);
    printf("\nName: \t\t%s",argv[1]);
    printf("\nAddress: \t\n");

    // Crear un socket para manejar datagramas UDP
    sock = socket( AF_INET, SOCK_DGRAM, 0 );
    if(sock == -1){
        perror("El socket no pudo ser creado !\n");
        exit(-1);
    } //endif

    // Activaremos una propiedad del socket que permitira que otros
    // sockets puedan reutilizar cualquier puerto al que nos enlacemos.
    // Esto permitira en protocolos como el TCP, poder ejecutar un
    // mismo programa varias veces seguidas y enlazarlo siempre al
    // mismo puerto. De lo contrario habria que esperar a que el puerto
    // quedase disponible (TIME_WAIT en el caso de TCP)
    c = 1;  // Opcion requerida por 'setsockopt'
    setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, (char *)&c, sizeof(c) );

    // Otro problema que hay que resolver es el bloqueo que se produce
    // en las funciones de envio y recepcion de informacion.
    // Si no se le indica nada al socket, todas las operaciones son
    // bloqueantes. Esto afecta especialmente a la recepcion de datos,
    // puesto que hasta que no lleguen, la funcion no devolvera el
    // control. Para evitar bloqueos se puede usar la funcion 'select'
    // antes de recibir datos. Aqui solamente se van a preparar las
    // estructuras y el timeout que usara el 'select'.
    FD_ZERO( &rfds );
    // Activar el 'indicador de lectura' asociado al socket creado.
    // Cuando se reciban datos, se activara el 'indicador de lectura'.
    FD_SET( sock, &rfds);
    tv.tv_sec  = 2;  // Segundos del 'timeout'.
    tv.tv_usec = 0;  // Microsegundos del 'timeout'.

    // Preparar una estructura con informacion sobre como vamos
    // a recibir los datagramas UDP
    // En este ejemplo, enlazaremos el socket a la IP local a cualquier
    // puerto UDP que esta libre. Esto lo conseguiremos inicializando
    // la estructura a cero mediante, por ejemplo, 'bzero'.
    // Si especificasemos un puerto mediante el comando:
    //    sock_addr.sin_port = htons(port);
    // entonces forzariamos el enlace del socket al puerto numero 'port'
    bzero( (char *)&sock_addr, sizeof(sock_addr) );
    sock_addr.sin_family = AF_INET;

    // Enlazar el socket creado al puerto que hemos descrito usando
    // la estructura anterior. Este paso es obligatorio si se desea
    // recibir cualquier datagrama.
    if( bind(sock, (struct sockaddr *)&sock_addr, sizeof(sock_addr) ) == -1 ){
        perror("El socket no pudo ser enlazado a un puerto libre !\n");
        exit(-1);  // No se pudo enlazar el socket. Habria que reintentarlo...
    } //endif

    // Preparar una estructura donde indicaremos la direccion IP a donde
    // vamos a enviar los datagramas y el puerto al que queremos que vayan
    // dirigidos.
    to_addr.sin_addr.s_addr = inet_addr(ip);// IP del DNS de la UAM
    to_addr.sin_port = htons(53);			// Puerto del DNS
    to_addr.sin_family = AF_INET;

    // Enviaremos un datagrama a dicha direccion IP y puerto, con unos
    // datos de ejemplo.
    // En el caso de la practica de DNS, estos datos corresponderan
    // con una trama DNS que se explica en el guion de la practica.


    mygethostbyname(argv[1]);
    c = sendto( sock,                    // Socket a utilizar para el envio
               buffer_query, offset,  // Ptr a datos y tamaÃ±o de los datos
               0, (struct sockaddr *)&to_addr, sockaddrlen );


    for( i=0; i<c; i++){
         if( (i%16)==0 ) fprintf( stdout, "\n" );  // Cada 16 bytes un EOL
         fprintf( stdout, "%02X ", buffer_query[i] );
    }



    if(c == -1){
        perror("No se pudo enviar el datagrama por el socket !\n");
        close( sock );
        exit(-1);
    } //endif

    // NOTA IMPORTANTE 1:
    // Cuando se envia un datagrama UDP a traves de un socket enlazado,
    // el puerto UDP de destino es el indicado por la estructura usada en
    // funcion 'sendto'. Pero el puerto UDP de origen es el que se uso
    // en la funcion 'bind' (que en este ejemplo es el primero libre que haya)
    // Cuando un servidor UDP recibe un 'request' en un puerto dedicado y
    // bien conocido (como es el 53 para el DNS), la respuesta la envia
    // al puerto de origen que le viene en el datagrama recibido.

    // NOTA IMPORTANTE 2:
    // Como ya se ha indicado arriba, la recepcion de datos no es bloqueante.
    // Tras un determinado 'timeout' la funcion 'select' devolvera un error
    // si no se han recibido datos por el socket. En caso contrario, la
    // funcion 'select' indicara que ya hay datos disponibles y entonces
    // podremos llamar a 'recvfrom' sin que se produzca un bloqueo.

    // Por lo tanto, esperaremos la respuesta del servidor por nuestro
    // puerto de origen (fuere cual fuere) que hemos enlazado al principio.

    c = select( FD_SETSIZE, &rfds, (fd_set *)NULL, (fd_set *)NULL, &tv );
    if(c == -1){
        perror("El 'select' tuvo un error !\n");
        close(sock);
        exit(1);
    } //endif

    if(c == 0){
        // El 'select' ha indicado que el tiempo indicado en 'tv' ha acabado
        fprintf( stdout, "Timeout expirado... Respuesta no recibida !\n");
    }
    else{
        // El 'select' ha indicado que hay datos pendientes de ser recibidos
        c = recv( sock, buffer, sizeof(buffer), 0 );


        // Nota: La funcion 'recvfrom' hace lo mismo, solo que ademas tambien
        //       rellena una estructura indicando desde que direccion IP nos
        //       han enviado el datagrama, y cual es el puerto de origen.

        if(c == -1){
           perror("Error al tratar de recibir un datagrama UDP !\n");
           close( sock );
           exit(-1);
        } //endif

//memcopy(&DESTINO,ORIGEN,CUANTO COPIA) ORIGEN:buffer+pos
aux=0;
offset=0;
memcpy(&aux,buffer+offset,sizeof(uint16_t));
offset=sizeof(uint16_t);
aux=ntohs(aux);
printf("\n\nIdentificador: 0x%02X\n\n",aux);

if (aux!=id){
	printf("\nEl identificador no coincide.\n");
}

memcpy(&aux,buffer+offset,sizeof(uint16_t));
offset+=sizeof(uint16_t);
aux=ntohs(aux);
printf("\nFlags: 0x%02X\n",aux);

if (aux<0x8000){
	printf("No es una respuesta.\n");
}

memcpy(&aux,buffer+offset,sizeof(uint16_t));
offset+=sizeof(uint16_t);
aux=ntohs(aux);
printf("\nPreguntas: %02X\n",aux);

memcpy(&aux,buffer+offset,sizeof(uint16_t));
offset+=sizeof(uint16_t);
aux=ntohs(aux);
printf("\nRespuestas: %02X\n",aux);
answers=aux;

memcpy(&aux,buffer+offset,sizeof(uint16_t));
offset+=sizeof(uint16_t);
aux=ntohs(aux);
printf("\nRespuestas autoritativas: %02X\n",aux);

memcpy(&aux,buffer+offset,sizeof(uint16_t));
offset+=sizeof(uint16_t);
aux=ntohs(aux);
printf("\nRespuestas adicionales: %02X\n",aux);
			//QUERIES
printf("\nQUERIES:\n");
i=12;
while(buffer[i]!=00) {
i++;
k++;
}

nombre=malloc((k+1)*sizeof(char));
memcpy(nombre,buffer+offset,k+1);
offset+=k+1;
printf("\nServidor: ");

for(i=0;i<k+1;i++){
if (nombre[i]<32){
q=(htons(nombre[i]))/256;
	if (cq==0){
	cq++;
	}
	else if(q!=0){
	printf(".");
	}


}
else{
printf("%c",nombre[i]);
}
}
free(nombre);
k=0;
cq=0;

memcpy(&aux,buffer+offset,sizeof(uint16_t));
offset+=sizeof(uint16_t);//Type

memcpy(&aux,buffer+offset,sizeof(uint16_t));
offset+=sizeof(uint16_t);//Class
offset1=offset;
			//ANSWERS

printf("\n\nANSWERS:\n");
while(cont<answers){  		//MIENTRAS NO LLEGUE AL FINAL DEL ANSWERS
memcpy(&aux1,buffer+offset1,sizeof(uint8_t));
offset1+=sizeof(uint8_t);

	if(aux1==192){//Si es una direccion, la leo
		memcpy(&aux1,buffer+offset1,sizeof(uint8_t));
		offset1=offset1+sizeof(uint8_t);
		i=aux1;
		while(buffer[i]!=00) {
			i++;
			k++;
		}

		nombre=malloc((k+1)*sizeof(char));
		memcpy(nombre,buffer+(aux1),k+1);
		offset+=k+1;
		printf("Servidor: ");

		for(i=0;i<k+1;i++){
			if (nombre[i]<32){
				q=(htons(nombre[i]))/256;
			if (cq==0){
				cq++;
			}
			else if(q!=0){
				printf(".");
			}
		}
		else
			printf("%c",nombre[i]);
		}

		free(nombre);
		k=0;
		cq=0;

	}
printf("\n");
memcpy(&aux,buffer+offset1,sizeof(uint16_t));
offset1+=sizeof(uint16_t);//Type

memcpy(&aux,buffer+offset1,sizeof(uint16_t));
offset1+=sizeof(uint16_t);//Class

memcpy(&aux,buffer+offset1,sizeof(uint32_t));
offset1+=sizeof(uint32_t);//Time to leave

memcpy(&aux,buffer+offset1,sizeof(uint16_t));
offset1+=sizeof(uint16_t);//Data length

memcpy(&adrr,buffer+offset1,sizeof(uint32_t));
offset1+=sizeof(uint32_t);//Address
adrr=htonl(adrr);
printf("Address: %02X",adrr);
printf("\n\n");

cont++;
}









        // Mostrar los datos recibidos. Si hemos mandado basura al DNS
        // nos contestara con una trama indicando que hay un error en la
        // estructura del 'query' que le enviamos.
        fprintf(stdout, "\nDatos recibidos:");                     //
        for( i=0; i<c; i++){
           if( (i%16)==0 ) fprintf( stdout, "\n" );  // Cada 16 bytes un EOL
           fprintf( stdout, "%02X ", buffer[i] );
        } //endfor
    } //endif
    printf("\n");
	//free(name); //Liberar el nombre 3www3uam etc..
    // Cerrar el socket y volver
    close( sock );

    return 0;
}
