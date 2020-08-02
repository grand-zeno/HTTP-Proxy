/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include "proxy_parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
void error(char* msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    unsigned int clilen;
    int sockfd, CFD, portnum;
    int n;
    char buffer[100000];
    struct sockaddr_in server_addr, cli_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *) &server_addr, sizeof(server_addr));
    portnum = atoi(argv[1]);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portnum);
    if (bind(sockfd, (struct sockaddr *) &server_addr,
                sizeof(server_addr)) < 0)
        error("ERROR on binding");
    int count=0;
    listen(sockfd,5);
    while(count<10){
        clilen = sizeof(cli_addr);
        CFD = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if(fork()==0){
            fflush(stdout);
            if (CFD < 0)
                error("ERROR on accept");
            bzero(buffer,100000);
            n = read(CFD,buffer,100000-1);
            if (n < 0) error("ERROR reading from socket");
            struct ParsedRequest* req= ParsedRequest_create();
            //char *c="GET http://www.google.com/ HTTP/1.0\r\n\r\n";
            if(ParsedRequest_parse(req,buffer,strlen(buffer))<0)
            {
                printf("Parse Failure\n");
                exit(1);
            }
            //printf(req->path);
            char *temp="GET";
            if(strcmp(req->method,temp)!=0)
            {
                printf("Not Implemented\n");
                exit(1);
            }
           // printf("hi");fflush(stdout);
            struct hostent *server;struct sockaddr_in server_addr;
            server = gethostbyname(req->host);
            int Sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (Sockfd < 0)
                error("ERROR opening socket");
            bzero((char *) &server_addr, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            bcopy((char *)server->h_addr,
                    (char *)&server_addr.sin_addr.s_addr,
                    server->h_length);
            server_addr.sin_port = htons(80);
            if (connect(Sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0)
                error("ERROR connecting");
            char temp_buf[100000];char temp_buf1[100000];
            bzero((char *) &temp_buf, sizeof(temp_buf));
             bzero((char *) &temp_buf1, sizeof(temp_buf1));
            char*tp=(char*)malloc(100000*(sizeof(char)));
            tp[0]='\0';
            tp=strcat(tp,"GET ");
            tp=strcat(tp,req->path);
            tp=strcat(tp," HTTP/1.0 \r\n");
            tp=strcat(tp,"Host: ");
            tp=strcat(tp,req->host);
            tp=strcat(tp,"\r\n");
            tp=strcat(tp,"Connection: close\r\n");
            ParsedRequest_unparse_headers(req, temp_buf1, 100000-1);
            tp=strcat(tp,temp_buf1);
            printf(tp);
            write(Sockfd,tp,strlen(tp));
            bzero(temp_buf,100000);

            int read_len=-1;
            while( 0!=(read_len =read(Sockfd,temp_buf,100000-1))){
                sleep(1);
                write(1, temp_buf,read_len);
                write(CFD,temp_buf,read_len);}
            close(Sockfd);
            close(CFD);
            if (n < 0) error("ERROR writing to socket");
            return 0;
        }
        count++;
        close(CFD);
    }
close(sockfd);
    return 0;
}


