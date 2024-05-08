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



char wiersz0[]="Dobrze jest nad jeziorem\rnawet por¹ deszczow¹.\rLeœniczy wieczorem\rlampê zapala naftow¹,\rpo chwili we wszystkich pokojach\rnaftowe lampy p³on¹\ra cienie od rogów jelenich\rrozrastaj¹ siê w nieskoñczonoœæ.\rPsy szczekaj¹.\rTr¹bki pó³nocy bliskie.\rA chmury pêdz¹ po niebie\rjak wielkie psy myœliwskie.\rZasypiamy\rprzytuleni do siebie jak dzieci.\rNoc siê wypogodzi³a.\rKsiê¿yc mruczy i œwieci.\r";
char wiersz1[]="Wp³yn¹³em na suchego przestwór oceanu,\rWóz nurza siê w zielonoœæ i jak ³ódka brodzi,\rŒród fali ³¹k szumi¹cych, œród kwiatów powodzi,\rOmijam koralowe ostrowy burzanu.\rJu¿ mrok zapada, nigdzie drogi ni kurhanu;\rPatrzê w niebo, gwiazd szukam, przewodniczek ³odzi;\rTam z dala b³yszczy ob³ok? tam jutrzenka wschodzi?\rTo b³yszczy Dniestr, to wesz³a lampa Akermanu.\rStójmy! – jak cicho! – s³yszê ci¹gn¹ce ¿urawie,\rKtórych by nie doœcig³y Ÿrenice soko³a;\rS³yszê, kêdy siê motyl ko³ysa na trawie,\rKêdy w¹¿ œlisk¹ piersi¹ dotyka siê zio³a.\rW takiej ciszy – tak ucho natê¿am ciekawie,\r¯e s³ysza³bym g³os z Litwy. – JedŸmy, nikt nie wo³a.\r";
char wiersz2[]="W mojej ojczyŸnie, do której nie wrócê,\rJest takie leœne jezioro ogromne,\rChmury szerokie, rozdarte, cudowne\rPamiêtam, kiedy wzrok za siebie rzucê.\rI p³ytkich wód szept w jakimœ mroku ciemnym,\rI dno, na którym s¹ trawy cierniste,\rMew czarnych krzyk, zachodów zimnych czerwieñ,\rCyranek œwisty w górze porywiste.\rŒpi w niebie moim to jezioro cierni.\rPochylam siê i widzê tam na dnie\rBlask mego ¿ycia. I to, co straszy mnie,\rJest tam, nim œmieræ mój kszta³t na wieki spe³ni\r";
char wiersz3[]="Dlaczego ogórek nie œpiewa\rPytanie to w tytule\rPostawione tak œmia³o\rChoæby z najwiêkszym bólem\rRozwi¹zaæ by nale¿a³o –\rJeœli ogórek nie œpiewa\r(I to o ¿adnej porze),\rTo widaæ z woli Nieba\rPrawdopodobnie nie mo¿e –\rLecz jeœli pragnie. Gor¹co!\rJak dot¹d nikt. Jak skowronek.\rJe¿eli w s³oju noc¹\r£zy przelewa zielone?\rMijaj¹ lata, zimy.\rRaz s³oneczko, raz chmurka.\rA my obojêtnie przechodzimy\rKo³o niejednego ogórka.\r";
char wiersz4[]="A w tych borach olsztyñskich\rdobrze z psami wêdrowaæ.\rA w tych jarach olsztyñskich\rsoœnina i d¹browa.\rTêcza mosty rozstawia.\rJak Wenus pachnie sza³wia.\rPtak siada na ramieniu.\rKomar p³acze w promieniu.\rW dzieñ niebo siê zaœmiewa,\ra noc¹ siê rozgwie¿d¿a,\rgwiazdy w gniazda spadaja.\r¯al bêdzie st¹d odje¿d¿aæ.\r";
char wiersz5[]="Wspomnienie\rMimozami jesieñ siê zaczyna,\rZ³otawa, krucha i mi³a.\rTo ty, to ty jesteœ ta dziewczyna,\rKtóra do mnie na ulicê wychodzi³a.\rOd twoich listów pachnia³o w sieni,\rGdym wraca³ zdyszany ze szko³y,\rA po ulicach w lekkiej jesieni\rFruwa³y za mn¹ jasne anio³y.\rMimozami zwiêd³oœæ przypomina\rNieœmiertelnik ¿ó³ty - paŸdziernik.\rTo ty, to ty moja jedyna,\rPrzychodzi³aœ wieczorem do cukierni.\rZ przemodlenia, z przeomdlenia senny,\rW parku p³aka³em szeptanymi s³owy.\rM³odzik z chmurek przeœwitywa³ jesienny,\rOd mimozy z³otej - majowy.\rAch, czu³ymi, przemi³ymi snami\rZasypia³em z nim gasn¹cym o poranku,\rW snach dawnymi bawi¹c siê wiosnami,\rJak t¹ z³ot¹, jak t¹ wonn¹ wi¹zank¹...\r";
char wiersz6[]="Jeden wiatr - w polu wia³,\rdrugi wiatr - w sadzie gra³:\rCichuteñko, leciuteñko,\rliœcie pieœci³ i szeleœci³,\rmdla³...\rJeden wiatr – pêdziwiatr!\rFikn¹³ koz³a, plackiem spad³,\rskoczy³, zawia³, zaszybowa³,\rœwidrem w górê zako³owa³\ri przewróci³ siê, i wpad³\rna szumi¹cy senny sad,\rgdzie cichutko i leciutko\rliœcie pieœci³ i szeleœci³\rdrugi wiatr...\rSfrun¹³ œniegiem z wiœni kwiat,\rparskn¹³ œmiechem ca³y sad,\rwzi¹³ wiatr brata za kamrata,\rteraz z nim po polu lata,\rgoni¹ obaj chmury, ptaki,\rmkn¹, wpl¹tuj¹ siê w wiatraki,\rg³upkowate myl¹ œmigi,\rw prawo, w lewo, œwist, podrygi,\rdm¹ p³ucami ile si³,\r³obuzuj¹, pal je licho!...\rA w sadzie cicho, cicho...\r";
char wiersz7[]="Figielek\rRaz siê komar z komarem przekomarzaæ zacz¹³,\rMówi¹c, ¿e widzia³ raki, co siê winkiem racz¹.\rCietrzew siê zacietrzewi³, s³ysz¹c takie s³owa,\rSêp zasêpi³ siê strasznie, osowia³a sowa,\rura da³a drapaka, ¿e a¿ siê kurzy³o,\rZaj¹c zajêcza³ smêtnie, kurczê siê skurczy³o,\rKozio³ fikn¹³ kozio³ka, s³oñ siê ca³y s³ania³,\rBaran siê rozindyczy³, a indyk zbarania³.\r";






struct litmemo
{
   int status;   // 0-nieaktywny     1-slucha   2-wysyla   3-zamkniety
   int sock;
   int interval;
   char *wiersz;
   char *wierszpos;
   int last;
} litmemo_;


int lsock;     // socket nas³uchujacy
#define MAX_CONNECTION 30

struct litmemo conns[MAX_CONNECTION];
int maxsock=0;
int i;

fd_set read_fds;
fd_set write_fds;
fd_set exc_fds;
fd_set master_fds;
struct timeval       timeout;



void add_new_conn(int fd)
{
    for (i=0; i<MAX_CONNECTION; i++)
        if (conns[i].status == 0){
            conns[i].sock = fd;
            conns[i].status = 1;
            break;
        }
}


void korekta_znakow(char* s)
{
   int i, len = strlen(s);
   char *c = s;
   
   if (len>0)
      for (i=0; i<len; i++)
      {
         switch (*c)
         {
                    case -71:
                        *c='a';
                        break;
                    case -22:
                        *c='e';
                        break;
                    case -13:
                        *c='o';
                        break;
                    case -65:
                    case -97:
                        *c='z';
                        break;
                    case -26:
                        *c='c';
                        break;
                    case -15:
                        *c='n';
                        break;
                    case -77:
                        *c='l';
                        break;
                    case -100:
                        *c='s';
                        break;
                    case -81:          
                        *c='Z';
                        break;
                    case -116:      
                        *c='S';
                        break;
        }
        c++;
    }
}



void handle_conn(int idx)
{
    char buffer[27];
    int retval;
    int sum=0;
    char send1[] = "Wiersz dla ";
    char send2[] = "  \r\r";
    bzero(&buffer,sizeof(buffer));
    retval = recv(conns[idx].sock,buffer,25,0);
    if (retval>0)
    {
        FD_CLR(conns[idx].sock, &master_fds);
        //printf("odebrano nazwe %d\n",retval);
        for (int i=0; i<retval; i++)
            sum += (unsigned char)buffer[i];
         sum = sum & 0x7;
         switch (sum)
         {
             case 0:
                conns[idx].wiersz = wiersz0;
                conns[idx].wierszpos = wiersz0;
                break;
             case 1:
               conns[idx].wiersz = wiersz1;
               conns[idx].wierszpos = wiersz1;
               break;
             case 2:
               conns[idx].wiersz = wiersz2;
               conns[idx].wierszpos = wiersz2;
               break;
            case 3:
               conns[idx].wiersz = wiersz3;
               conns[idx].wierszpos = wiersz3;
               break;
            case 4:
               conns[idx].wiersz = wiersz4;
               conns[idx].wierszpos = wiersz4;
               break;
            case 5:
               conns[idx].wiersz = wiersz5;
               conns[idx].wierszpos = wiersz5;
               break;
            case 6:
               conns[idx].wiersz = wiersz6;
               conns[idx].wierszpos = wiersz6;
               break;
            default:
               conns[idx].wiersz = wiersz7;
               conns[idx].wierszpos = wiersz7;
               break;               

         }
         
         conns[idx].interval = 12 + rand() & 31;
         conns[idx].status = 2;
         korekta_znakow(conns[idx].wiersz);
         conns[idx].last =(int)strlen(conns[idx].wiersz);
         send(conns[idx].sock,send1,strlen(send1),MSG_NOSIGNAL);
         send(conns[idx].sock,buffer,retval,MSG_NOSIGNAL);
         send(conns[idx].sock,send2,strlen(send2),MSG_NOSIGNAL);
         
    }
    
}


int send_conn(int idx)
{
  int size, slen;
  if (conns[idx].status == 2){
      if (conns[idx].interval>=0){
        if (conns[idx].interval>0)conns[idx].interval--;
        if (conns[idx].interval == 0){
           conns[idx].interval =12+ rand() & 31; 
           size = 20 +  (rand() & 31);
           slen = strlen(conns[idx].wierszpos);
           if (slen==0)
           {
               printf("Endof string\n");
               //shutdown( conns[idx].sock, SHUT_RDWR );
               close(conns[idx].sock);
               conns[idx].status = 0;
               return 0;
            }
           if (slen<size) size=slen;             
           slen = send(conns[idx].sock,conns[idx].wierszpos,size,MSG_NOSIGNAL);
           if (slen>=0){
              conns[idx].wierszpos += slen;
              conns[idx].last -= slen;
              if (conns[idx].last>0)
                   return 1;
              else{
                  close(conns[idx].sock);
                  //shutdown( conns[idx].sock, SHUT_RDWR );
                  conns[idx].status = 0; 
                  return 0;
              }
           }
           else{
               printf("Write error\n");
               close(conns[idx].sock);
               //shutdown( conns[idx].sock, SHUT_RDWR );
               conns[idx].status = 0;
               return 0;
           }
                         
        }
        return 1; 
      }      
  }
  return 0;
}



#define SERWER_IP "0.0.0.0" 

int main(int argc, char *argv[])
{
    int portno;
    struct sockaddr_in serwer;
    int active_w;
    int active_r;

    int old_w = -1;
    int old_r = -1;

    srand(0x8976);
//    printf("w0 %d  w1 %d\n",strlen(wiersz0),strlen(wiersz1));
  

    bzero((char*) &serwer,sizeof(serwer));
    bzero((char*) &conns,sizeof(conns));
    
    if (argc<2){
        perror(" nie podano portu\n");
        exit(19);
    }
    portno = atoi(argv[1]);
    
    serwer.sin_family = AF_INET,
    serwer.sin_port = htons(portno);
    
    if( inet_pton( AF_INET, SERWER_IP, & serwer.sin_addr ) <= 0 )
    {
        perror( "inet_pton() ERROR" );
        exit( 1 );
    }
   
    lsock = socket( AF_INET, SOCK_STREAM, 0 );
    if(( lsock ) < 0 )
    {
        perror( "socket() ERROR" );
        exit( 2 );
    }
   
    char buffer[ 4096 ] = { };
    char sendbuffer[6000];
   
    socklen_t len = sizeof( serwer );//
    if( bind( lsock,(struct sockaddr *) &serwer, len ) < 0 )
    {
        perror( "bind() ERROR" );
        exit( 3 );
    }

    if( listen( lsock,20) < 0 )
    {
        perror( "listen() ERROR" );
        exit( 3 );
    }
    /*************************************************************/
    /* Initialize the timeval struct to 3 minutes.  If no        */
    /* activity after 3 minutes this program will end.           */
    /*************************************************************/
    timeout.tv_sec  = 0;
    timeout.tv_usec = 400000;


    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&exc_fds);
    FD_ZERO(&master_fds);

    FD_SET(0, &master_fds);
    FD_SET(lsock, &master_fds);
    if (lsock>maxsock)
        maxsock = lsock;
     
    while( 1 )
    {
         int retval;
         int active;
         int new_sd;
         int show_status = 0;
         char keyboard_message[200];
         memcpy(&read_fds, &master_fds, sizeof(master_fds));
         retval = select(maxsock+1, &read_fds, NULL, NULL, &timeout);
         if (retval==-1)
         {
             perror("Server-select() error");
             exit(31);
         }
         if (retval==0)
         {
           //  printf("timeout");             
         }
         if (FD_ISSET(0, &read_fds))
         {
            show_status =1;
            //fgets(keyboard_message,sizeof(keyboard_message)-1,0);
            keyboard_message[0] = getchar();
         }
         else
            show_status=0;
         if (retval>0)
         {
             active = retval;
             if (FD_ISSET(lsock, &read_fds)){
                 // nowe polaczenie
                 printf("Aktywnosc na lsock\n");
                 active--;
                 new_sd = accept(lsock, NULL, NULL);
                 printf("New sock %d\n",new_sd);
                 if (new_sd < 0)
                 {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("  accept() failed");
                        exit(32);
                     }
                     break;
                 }
                 add_new_conn(new_sd);
                 FD_SET(new_sd, &master_fds);
                 if (new_sd>maxsock)
                       maxsock = new_sd;
             }
             if (active>0)
                for (i=0; i<MAX_CONNECTION; i++){
                   if (FD_ISSET(conns[i].sock ,&read_fds))
                     handle_conn(i);
                }
         }

         active_r=0;
         active_w=0;
         for (i=0; i<MAX_CONNECTION; i++)
         {
           // if (i<3)
            if (show_status)
               if (conns[i].status>0)
                  printf(">>>%2d  status:%d  sock:%d  interval:%d  last:%d\n",i,conns[i].status,conns[i].sock,conns[i].interval,conns[i].last);
            if (conns[i].status == 1)
                active_r++;
            if (conns[i].status == 2)
            {
                send_conn(i);
                if (conns[i].status == 2)
                   active_w++;
                else
                {                  
                   FD_CLR(conns[i].sock, &master_fds);
                   close(conns[i].sock);
                }
                if (conns[i].last == 0)
                {
                   printf("Zamkniecie socketa %d %d\n",i,conns[i].sock);
                   FD_CLR(conns[i].sock, &master_fds);
                   close(conns[i].sock);
                   conns[i].interval = 999;
                   conns[i].status = 0;
                }
            }
         }       
         if (show_status)
             printf("End of list.\n");
         if ((old_w!=active_w) || (old_r != active_r) || (show_status>0))
              printf("Polaczen czytajacych %d  polaczen wysylajacych %d \n",active_r,active_w);
         old_r = active_r;
         old_w = active_w;
         usleep(300000);
    }
   
    shutdown( lsock, SHUT_RDWR );
}
// gcc server.c -g -Wall -o server && ./server
