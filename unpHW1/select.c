/* include fig01 */
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
	int					i, maxi, maxfd, listenfd, connfd, sockfd;
	int					nready, client[FD_SETSIZE];
	ssize_t				n;
	fd_set				rset, allset;
	char				buf[2048];
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(8088);

	bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	listen(listenfd, 10);

	maxfd = listenfd;			/* initialize */
	maxi = -1;					/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;			/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
/* end fig01 */

/* include fig02 */
	for ( ; ; ) {
		rset = allset;		/* structure assignment */
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(listenfd, &rset)) {	/* new client connection */
			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
#ifdef	NOTDEF
			printf("new client: %s, port %d\n",
					Inet_ntop(AF_INET, &cliaddr.sin_addr, 4, NULL),
					ntohs(cliaddr.sin_port));
#endif

			for (i = 0; i < FD_SETSIZE; i++)
				if (client[i] < 0) {
					client[i] = connfd;	/* save descriptor */
					break;
				}
			if (i == FD_SETSIZE)
				perror("too many clients");

			FD_SET(connfd, &allset);	/* add new descriptor to set */
			if (connfd > maxfd)
				maxfd = connfd;			/* for select */
			if (i > maxi)
				maxi = i;				/* max index in client[] array */

			if (--nready <= 0)
				continue;				/* no more readable descriptors */
		}

		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &rset)) {
				if ( (n = read(sockfd, buf, 2047)) == 0) {
						/*4connection closed by client */
					close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				}
                else {
					printf("New Client\n");
					printf("%s\n", buf);

					// forbit other requests
					if (strncmp(buf, "GET ", 4) && strncmp(buf, "get ", 4)) {
						perror("Received request other than GET\n");
						exit(1);
					}
					
					if (!strncmp(buf, "GET /unp-img.jpg", 16)) {	// request image
						printf("Sending image...\n");

						int fdImg, len;
						if ( (fdImg = open("unp-img.jpg", O_RDONLY)) == -1 ) {
							perror("Opening image failed.");
							exit(1);
						}

                        char buff[2048] = "\0";
						strcpy(buff, "HTTP/1.0 200 OK\r\n");
						strcat(buff, "Content-Type:image/jpeg\r\n\r\n");
						send(sockfd, buff, strlen(buff), 0);
						
						while( (len = read(fdImg, buff, 2047)) > 0 )
							send(sockfd, buff, len, 0);
						
						printf("Successfully sent image.\n");
						close(fdImg);
					}
					else {	// request html
						printf("Sending html...\n");

						int fdWeb, len;
						if ( (fdWeb = open("web2.html", O_RDONLY)) == -1 ) {
							perror("Opening html failed.");
							exit(1);
						}

                        char buff[2048] = "\0";
						strcpy(buff, "HTTP/1.0 200 OK\r\n");
						strcat(buff, "Content-Type:text/html\r\n\r\n");
						send(sockfd, buff, strlen(buff), 0);

						while( (len = read(fdWeb, buff, 2047)) > 0 )
							send(sockfd, buff, len, 0);

						printf("Successfully sent html\n");
						close(fdWeb);
					}

					close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
                }

				if (--nready <= 0)
					break;				/* no more readable descriptors */
			}
		}
	}
}
/* end fig02 */