#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

int create_socket(int);
int accept_conn(int);

#ifdef WINDOWS
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
 #endif

#define MAXLINE 4096 /* Maksymalna długość linii tekstu */
#define LISTENQ 8 /* Maksymalna ilość połączen klientów */

int main (int argc, char **argv)
{
   int listenfd, connfd, n;
   pid_t childpid;
   socklen_t clilen;
   char buf[MAXLINE];
   struct sockaddr_in cliaddr, servaddr;

   if (argc !=2) { // Sprawdzenie poprawności danych wejściowych
      fprintf(stderr, "Usage: ./a.out <port number>\n");
      exit(1);
   }


   // Tworzenie soketa dla klienta
   // Jeżeli sockfd<0 rzucony jest bląd wynikający z nieudałego utworzenia soketu
   if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
      fprintf(stderr, "Problem in creating the socket\n");
      exit(2);
   }


   // Przetwarzanie adresu soketowego
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   if(atoi(argv[1])<=1024) {
      fprintf(stderr, "Port number must be greater than 1024\n");
      exit(5);
   }
   servaddr.sin_port = htons(atoi(argv[1]));

   // Dowiązanie soketu
   bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

   // Nasłuciwanie soketu za pomocy tworzenia kolejki połączen, zaczym czekanie na klientów
   listen (listenfd, LISTENQ);

   printf("Server running...waiting for connections.\n");

   for (;; ) {

      clilen = sizeof(cliaddr);
      // Przyjmowanie polączenia
      connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);

      printf("Received request...\n");

      if ( (childpid = fork ()) == 0 ) { // Jezeli jest równe 0, to jest proces potomny(dziecko)

         printf("Child created for dealing with client requests\n");

         // Zamknięcie nasłuchującego soketu
         close (listenfd);
         int data_port=1024; // Dla danych połączenia
         while ( (n = recv(connfd, buf, MAXLINE,0)) > 0)  {
            printf("String received from client: %s", buf);
            char *token,*dummy;
            dummy=buf;
            token=strtok(dummy," ");

            if (strcmp("quit\n",buf)==0)  {
               printf("The client has quit\n");
            }

            if (strcmp("ls\n",buf)==0)  {
               FILE *in;
               char temp[MAXLINE],port[MAXLINE];
               int datasock;
               data_port=data_port+1;
               if(data_port==atoi(argv[1])) {
                  data_port=data_port+1;
               }
               sprintf(port,"%d",data_port);
               datasock=create_socket(data_port); // Utworzenie soketu dla połączenia danych
               send(connfd, port,MAXLINE,0); // Wysłanie numeru portu połączenia danych do klienta
               datasock=accept_conn(datasock); // Przyjęcie połączenia od klienta
               if(!(in = popen("ls", "r"))) {
                  printf("error\n");
               }
               while(fgets(temp, sizeof(temp), in)!=NULL) {
                  send(datasock,"1",MAXLINE,0);
                  send(datasock, temp, MAXLINE, 0);

               }
               send(datasock,"0",MAXLINE,0);
               pclose(in);
               //printf("file closed\n");
            }

            if (strcmp("pwd\n",buf)==0)  {
               char curr_dir[MAXLINE];

               GetCurrentDir(curr_dir,MAXLINE-1);
               send(connfd, curr_dir, MAXLINE, 0);
               //printf("%s\n", curr_dir);
            }

            if (strcmp("cd",token)==0)  {
               token=strtok(NULL," \n");
               printf("Path given is: %s\n", token);
               if(chdir(token)<0) {
                  send(connfd,"0",MAXLINE,0);
               }
               else{
                  send(connfd,"1",MAXLINE,0);
               }
            }

            if (strcmp("put",token)==0)  {
               char port[MAXLINE],buffer[MAXLINE],char_num_blks[MAXLINE],char_num_last_blk[MAXLINE],check[MAXLINE];
               int datasock,num_blks,num_last_blk,i;
               FILE *fp;
               token=strtok(NULL," \n");
               printf("Filename given is: %s", token);
               data_port=data_port+1;
               if(data_port==atoi(argv[1])) {
                  data_port=data_port+1;
               }
               sprintf(port,"%d",data_port);
               datasock=create_socket(data_port); // Utworzenie soketu dla połączenia danych
               send(connfd, port,MAXLINE,0); // Wysłanie numeru portu połączenia danych do klienta
               datasock=accept_conn(datasock);  // Przyjęcie połączenia od klienta
               recv(connfd,check,MAXLINE,0);
               printf("%s\n",check);
               if(strcmp("1",check)==0) {
                  if((fp=fopen(token,"w"))==NULL)
                     printf("Error in creating file\n");
                  else
                  {
                     recv(connfd, char_num_blks, MAXLINE,0);
                     num_blks=atoi(char_num_blks);
                     for(i= 0; i < num_blks; i++) {
                        recv(datasock, buffer, MAXLINE,0);
                        fwrite(buffer,sizeof(char),MAXLINE,fp);
                        //printf("%s\n", buffer);
                     }
                     recv(connfd, char_num_last_blk, MAXLINE,0);
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
            }

            if (strcmp("get",token)==0)  {
               char port[MAXLINE],buffer[MAXLINE],char_num_blks[MAXLINE],char_num_last_blk[MAXLINE];
               int datasock,lSize,num_blks,num_last_blk,i;
               FILE *fp;
               token=strtok(NULL," \n");
               printf("Filename given is: %s\n",token);
               data_port=data_port+1;
               if(data_port==atoi(argv[1])) {
                  data_port=data_port+1;
               }
               sprintf(port,"%d",data_port);
               datasock=create_socket(data_port); // Utworzenie soketu dla połączenia danych
               send(connfd, port,MAXLINE,0); // Wysłanie numeru portu połączenia danych do klienta
               datasock=accept_conn(datasock);  // Przyjęcie połączenia od klienta
               if ((fp=fopen(token,"r"))!=NULL)
               {
                  //size of file
                  send(connfd,"1",MAXLINE,0);
                  fseek (fp, 0, SEEK_END);
                  lSize = ftell (fp);
                  rewind (fp);
                  num_blks = lSize/MAXLINE;
                  num_last_blk = lSize%MAXLINE;
                  sprintf(char_num_blks,"%d",num_blks);
                  send(connfd, char_num_blks, MAXLINE, 0);
                  //printf("%s $s\n", num_blks, num_last_blk);

                  for(i= 0; i < num_blks; i++) {
                     fread (buffer,sizeof(char),MAXLINE,fp);
                     send(datasock, buffer, MAXLINE, 0);
                     //printf("%s %s\n", buffer, i);
                  }
                  sprintf(char_num_last_blk,"%d",num_last_blk);
                  send(connfd, char_num_last_blk, MAXLINE, 0);
                  if (num_last_blk > 0) {
                     fread (buffer,sizeof(char),num_last_blk,fp);
                     send(datasock, buffer, MAXLINE, 0);
                     //printf("%s\n", buffer);
                  }
                  fclose(fp);
                  printf("File upload done.\n");

               }
               else{
                  send(connfd,"0",MAXLINE,0);
               }
            }

         }

         if (n < 0)
            printf("Read error\n");

         exit(0);
      }
      // Zamknięcie soketu serwera
      close(connfd);
   }
}

int create_socket(int port)
{
   int listenfd;
   struct sockaddr_in dataservaddr;


// Tworzenie soketa dla klienta
// Jeżeli sockfd<0 rzucony jest bląd wynikający z nieudałego utworzenia soketu
   if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
      fprintf(stderr, "Problem in creating the data socket\n");
      exit(2);
   }


// Przetwarzanie adresu soketowego
   dataservaddr.sin_family = AF_INET;
   dataservaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   dataservaddr.sin_port = htons(port);

   if ((bind (listenfd, (struct sockaddr *) &dataservaddr, sizeof(dataservaddr))) <0) {
      fprintf(stderr, "Problem in binding the data socket\n");
      exit(6);
   }

   // Nasłuciwanie soketu za pomocy tworzenia kolejki połączen, zaczym czekanie na klientów
   listen (listenfd, 1);

   return(listenfd);
}

int accept_conn(int sock)
{
   int dataconnfd;
   socklen_t dataclilen;
   struct sockaddr_in datacliaddr;

   dataclilen = sizeof(datacliaddr);
   // Przyjmowanie polączenia
   if ((dataconnfd = accept (sock, (struct sockaddr *) &datacliaddr, &dataclilen)) <0) {
      fprintf(stderr, "Problem in accepting the data socket\n");
      exit(7);
   }

   return(dataconnfd);
}
