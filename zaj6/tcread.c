#include <stdio.h>
#include <stdlib.h> // exit()
#include <string.h> // memset()
#include <arpa/inet.h> // inet_pton()
#include <sys/socket.h>

// Server program
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>



#define SERWER_IP "0.0.0.0" 

int main(int argc, char *argv[])
{
    int portno;
    struct sockaddr_in serwer;
    int active_w;
    int retval;
    char namebuffer[100];
    int sock;
    int zerostate=0;

    srand(0x8976);

    bzero((char*) &serwer,sizeof(serwer));
    bzero((char*) &namebuffer,sizeof(namebuffer));

    if (argc<2){
        printf("tcread <IP> <port> <nazwa>\n");
        perror(" nie podano adresu\n");
        exit(19);
    }

    
    if (argc<3){
        printf("tcread <IP> <port> <nazwa>\n");        
        perror(" nie podano portu\n");
        exit(19);
    }
    
    if (argc<4){
        printf("tcread <IP> <port> <nazwa>\n");        
        perror(" nie podano nazwy\n");
        exit(19);
    }
    portno = atoi(argv[2]);
    strncpy(namebuffer,argv[3],sizeof(namebuffer)-1);
    
    serwer.sin_family = AF_INET;
    serwer.sin_port = htons(portno);
    
    if( inet_pton( AF_INET, argv[1], &serwer.sin_addr ) <= 0 )
    {
        perror( "inet_pton() ERROR" );
        exit( 1 );
    }
   
    sock = socket( AF_INET, SOCK_STREAM, 0 );
    if(( sock ) < 0 )
    {
        perror( "socket() ERROR" );
        exit( 2 );
    }
  
   
    socklen_t len = sizeof( serwer );
    retval = connect(sock,(struct sockaddr *) &serwer, len );
    
    if( retval<0)
    {
        perror( "connect() ERROR" );
        exit( 3 );
    }

    retval = send(sock,namebuffer, strlen(namebuffer)+1,MSG_NOSIGNAL);
    
    while( 1 )
    {
         char c;
         int retval;
        
         retval= recv(sock,&c,1,0);
         //printf("     %c  %d   ret %d\n",c,c,retval);
         if (retval>0)
         {
             if (c=='\r') printf("\n");
             else
              /* switch (c)
                 {
                    case -71:
                        printf("a");
                        break;
                    case -22:
                        printf("e");
                        break;
                    case -13:
                        printf("o");
                        break;
                    case -65:
                    case -97:
                        printf("z");
                        break;
                    case -26:
                        printf("c");
                        break;
                    case -15:
                        printf("n");
                        break;
                    case -77:
                        printf("l");
                        break;
                    case -100:
                        printf("s");
                        break;
                    case -81:          
                        printf("Z");
                        break;
                    case -116:      
                        printf("S");
                        break;
                   
                       
                          
                    default: 
                        printf("%c",c);
                 } */
             printf("%c",c);

             fflush(NULL);
         }   
         if (retval<0)
         {
             printf("recv error, errno:%d\n",errno);
             break;
         }
         if (retval==0)
         {
             zerostate++;

             if (zerostate>20)
             {                             
                 break;
             }
         }
         usleep(80000);
    }
    printf("\nKoniec.\n");
    fflush(NULL);
    shutdown( sock, SHUT_RDWR );
}
// gcc server.c -g -Wall -o server && ./server
