#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

int create_socket(int,char *);

#ifdef WINDOWS
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
 #endif

#define MAXLINE 4096 /* Maksymalna długość linii tekstu */

int main(int argc, char **argv)
{
   int sockfd;
   struct sockaddr_in servaddr;
   char sendline[MAXLINE] /*, recvline[MAXLINE]*/;

   // Sprawdzanie argumentów
   // można dodać opcjonalne
   if (argc !=3) {
      fprintf(stderr, "Usage: ./a.out <IP address of the server> <port number>\n");
      exit(1);
   }

   // Utworzenie soketa dla klienta
   // Jeżeli sockfd<0 rzucony jest bląd wynikający z nieudałego utworzenia soketu
   if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
      fprintf(stderr, "Problem in creating the socket\n");
      exit(2);
   }

   // Tworzenie soketa
   memset(&servaddr, 0, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr= inet_addr(argv[1]);
   servaddr.sin_port =  htons(atoi(argv[2])); // Przeksztalcenie do big-endian porządku

   // Połączenie klienta z soketem
   if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
      fprintf(stderr, "Problem in connecting to the server\n");
      exit(3);
   }

   printf("ftp>");

   while (fgets(sendline, MAXLINE, stdin) != NULL) {

      send(sockfd, sendline, MAXLINE, 0);
      char *token,*dummy;
      dummy=sendline;
      token=strtok(dummy," ");

      if (strcmp("quit\n",sendline)==0)  {
         //close(sockfd);
         return 0;
      }

      else if (strcmp("ls\n",sendline)==0)  {
         char buff[MAXLINE],check[MAXLINE]="1",port[MAXLINE];
         int data_port,datasock;
         recv(sockfd, port, MAXLINE,0); // Uzyskiwanie portu połączeniowego
         data_port=atoi(port);
         datasock=create_socket(data_port,argv[1]);
         while(strcmp("1",check)==0) { // Zawiadomienie o większej ilości bloków nasuwających
            recv(datasock,check,MAXLINE,0);
            if(strcmp("0",check)==0) // Nie ma więcej bloków
               break;
            recv(datasock, buff, MAXLINE,0);
            printf("%s", buff);
         }

      }

      else if (strcmp("!ls\n",sendline)==0)  {
         system("ls");
      }

      else if (strcmp("pwd\n",sendline)==0)  {
         char curr_dir[MAXLINE];
         recv(sockfd, curr_dir, MAXLINE,0);
         printf("%s\n", curr_dir);
      }

      else if (strcmp("!pwd\n",sendline)==0)  {
         system("pwd");
      }

      else if (strcmp("cd",token)==0)  {
         char check[MAXLINE];
         token=strtok(NULL," \n");
         printf("Path given is: %s\n", token);
         recv(sockfd,check,MAXLINE,0);
         if(strcmp("0",check)==0) {
            fprintf(stderr, "Directory doesn't exist. Check Path\n");
         }

      }

      else if (strcmp("!cd",token)==0)  {
         token=strtok(NULL," \n");
         printf("Path given is: %s\n", token);
         if(chdir(token)<0) {
            fprintf(stderr, "Directory doesn't exist. Check path\n");
         }
      }

      else if (strcmp("put",token)==0)  {
         char port[MAXLINE], buffer[MAXLINE],char_num_blks[MAXLINE],char_num_last_blk[MAXLINE];
         int data_port,datasock,lSize,num_blks,num_last_blk,i;
         FILE *fp;
         recv(sockfd, port, MAXLINE,0); // Uzyskiwanie portu danych
         data_port=atoi(port);
         datasock=create_socket(data_port,argv[1]);
         token=strtok(NULL," \n");
         if ((fp=fopen(token,"r"))!=NULL)
         {
            // Rozmiar pliku
            send(sockfd,"1",MAXLINE,0);
            fseek (fp, 0, SEEK_END);
            lSize = ftell (fp);
            rewind (fp);
            num_blks = lSize/MAXLINE;
            num_last_blk = lSize%MAXLINE;
            sprintf(char_num_blks,"%d",num_blks);
            send(sockfd, char_num_blks, MAXLINE, 0);
            //printf(num_blks<<"	"<<num_last_blk<<endl;

            for(i= 0; i < num_blks; i++) {
               fread (buffer,sizeof(char),MAXLINE,fp);
               send(datasock, buffer, MAXLINE, 0);
               //printf(buffer<<"	"<<i<<endl;
            }
            sprintf(char_num_last_blk,"%d",num_last_blk);
            send(sockfd, char_num_last_blk, MAXLINE, 0);
            if (num_last_blk > 0) {
               fread (buffer,sizeof(char),num_last_blk,fp);
               send(datasock, buffer, MAXLINE, 0);
               //printf(buffer<<endl;
            }
            fclose(fp);
            printf("File upload done.\n");
         }
         else{
            send(sockfd,"0",MAXLINE,0);
            fprintf(stderr, "Error in opening file. Check filename\nUsage: put filename\n");
         }
      }

      else if (strcmp("get",token)==0)  {
         char port[MAXLINE], buffer[MAXLINE],char_num_blks[MAXLINE],char_num_last_blk[MAXLINE],message[MAXLINE];
         int data_port,datasock,/*lSize,*/ num_blks,num_last_blk,i;
         FILE *fp;
         recv(sockfd, port, MAXLINE,0);
         data_port=atoi(port);
         datasock=create_socket(data_port,argv[1]);
         token=strtok(NULL," \n");
         recv(sockfd,message,MAXLINE,0);
         if(strcmp("1",message)==0) {
            if((fp=fopen(token,"w"))==NULL)
               printf("Error in creating file\n");
            else
            {
               recv(sockfd, char_num_blks, MAXLINE,0);
               num_blks=atoi(char_num_blks);
               for(i= 0; i < num_blks; i++) {
                  recv(datasock, buffer, MAXLINE,0);
                  fwrite(buffer,sizeof(char),MAXLINE,fp);
                  //printf("%s\n", buffer);
               }
               recv(sockfd, char_num_last_blk, MAXLINE,0);
               num_last_blk=atoi(char_num_last_blk);
               if (num_last_blk > 0) {
                  recv(datasock, buffer, MAXLINE,0);
                  fwrite(buffer,sizeof(char),num_last_blk,fp);
                  //printf("%s\n", buffer);
               }
               fclose(fp);
               printf("File download done.\n");
            }
         }
         else{
            fprintf(stderr, "Error in opening file. Check filename\nUsage: put filename\n");
         }
      }
      else{
         fprintf(stderr, "Error in command. Check Command\n");
      }
      printf("ftp>");

   }

   exit(0);
}


int create_socket(int port,char *addr)
{
   int sockfd;
   struct sockaddr_in servaddr;


   // Utworzenie socketa dla klienta
   // Jeżeli sockfd<0 rzucony jest bląd wynikający z nieudałego utworzenia soketu
   if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
      fprintf(stderr, "Problem in creating the socket\n");
      exit(2);
   }

   // Tworzenie soketu
   memset(&servaddr, 0, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr= inet_addr(addr);
   servaddr.sin_port =  htons(port); // Przeksztalcenie do big-endian porządku

   // połączenie klienta do soketu
   if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
      fprintf(stderr, "Problem in creating data channel\n");
      exit(3);
   }

   return(sockfd);
}
