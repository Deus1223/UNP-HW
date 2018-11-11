#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/select.h>

int main()
{
	int listenfd, connfd;
	struct sockaddr_in servaddr,cliaddr;
	
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(8080);

	bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

	listen(listenfd,10);
	for(;;)
	{
        socklen_t cli_len = sizeof(cliaddr);

		if( (connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &cli_len)) < 0)
		{
			printf("Connection Failed\n");
			continue;   /* back to for() */
		}

		if( fork() == 0 )   /* child process */
		{
            close(listenfd);

            char buff[2048] = "\0";
			recv(connfd, buff, 2047, 0);
			printf("%s\n", buff);

            strcpy(buff, "HTTP/1.0 200 OK\r\n");
			strcat(buff, "Content-Type:text/html\r\n\r\n");
			send(connfd, buff, strlen(buff), 0);

            int fp = open("web1.html", O_RDONLY);
            int len;
			while( (len = read(fp, buff, 2047)) > 0)
				send(connfd, buff, len, 0);
            
			close(connfd);
			exit(0);
		}

		wait();
		close(connfd);
	}	
	return 0;
}