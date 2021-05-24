#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.h"
#include "structura.h"

using namespace std;

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_address server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;
	char buffer[2000];
	char id[11];

	fd_set read_fds; // multimea de citire folosita in select()
	fd_set tmp_fds;
	int fdmax; // valoare maxima fd din multimea read_fds

	if (argc < 3)
	{
		usage(argv[0]);
	}

	// se goleste multimea de descriptori de citire si multimea temporara
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

	int flag = 1, result;
	result = setsockopt(sockfd, IPPROTO_TCP, _NETINET_IN_H, (char *)&flag, sizeof(int));
	DIE(result < 0, "Nevile Error");

	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;
	char aux[BUFLEN];
	char *aux2;
	char command[100], topic[100];
	int type;

	strcpy(id, argv[1]);

	while (1)
	{
		tmp_fds = read_fds;
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		if (FD_ISSET(STDIN_FILENO, &tmp_fds))
		{
			struct mesaj mesaj_trimis;

			// se citeste de la tastatura
			memset(buffer, 0, 2000);
			fgets(buffer, 2000, stdin);
			buffer[strlen(buffer) - 1] = '\0';
			strcpy(mesaj_trimis.id_client, id);

			// Daca clientul vrea sa se deconecteze
			if (strncmp(buffer, "exit", 4) == 0)
			{
				memcpy(mesaj_trimis.text, buffer, 4);

				n = send(sockfd, &mesaj_trimis, 2000, 0);
				DIE(n < 0, "send");
				break;
			}
				aux2 = strtok(buffer, " ");
				strcpy(mesaj_trimis.text, aux2);

				aux2 = strtok(NULL, " ");
				aux2[strlen(aux2)] = '\0';
				strcpy(mesaj_trimis.topic, aux2);

				aux2 = strtok(NULL, " ");
				if (aux2 != NULL)
				{
					strcpy(mesaj_trimis.type, aux2);
				}
				n = send(sockfd, &mesaj_trimis, sizeof(struct mesaj), 0);
				DIE(n < 0, "send");

				// Returnez comanda data
				if (strcmp(mesaj_trimis.text, "subscribe") == 0)
					printf("Subscribed to topic.\n");
				else if (strcmp(mesaj_trimis.text, "unsubscribe") == 0)
					printf("Unsubscribed from topic.\n");
				continue;
		}
		// Daca acesta primeste o comanda de la server
		if (FD_ISSET(sockfd, &tmp_fds))
		{
			memset(buffer, 0, 2000);
			n = recv(sockfd, buffer, 2000, 0);
			DIE(n < 0, "recv");

			// primeste inchiderea serverului si se inchide si el
			if (strcmp(buffer, "exit") == 0)
				break;
			// Primeste o cerere de la server de a ii trimite id-ul
			if (strcmp(buffer, "id") == 0)
			{
				memset(buffer, 0, 2000);
				strcpy(buffer, id);
				n = send(sockfd, buffer, 2000, 0);
				DIE(n < 0, "send problems");
			}
			//Primeste un mesaj de tip UDP
			if (strcmp(buffer, "UDP") == 0)
			{
				struct mesajUDP mesaj;
				strcpy(mesaj.topic, buffer + 4);
				mesaj.tipp = buffer[56];
				strcpy(mesaj.ip, buffer + 1559);
				strcpy(mesaj.port, buffer + 1609);

				// Primeste un INT
				if (mesaj.tipp + 48 == '0')
				{
					uint8_t semn;

					uint32_t nr;
					memset(&semn, 0, 1);
					memset(&nr, 0, 4);

					memcpy(&semn, buffer + 57, 1);
					memcpy(&nr, buffer + 58, 4);

					nr = ntohl(nr);
					if (semn == 0)
						printf("%s:%s - %s - INT - %d\n", mesaj.ip, mesaj.port, mesaj.topic, nr);
					else
						printf("%s:%s - %s - INT - %d\n", mesaj.ip, mesaj.port, mesaj.topic, -nr);
				}
				// Primeste un real scurt
				if (mesaj.tipp + 48 == '1')
				{
					uint16_t nr;

					memset(&nr, 0, 2);

					memcpy(&nr, buffer + 57, 2);

					nr = ntohs(nr);
					float nr_aux = nr;
					nr_aux /= 100;
					printf("%s:%s - %s - SHORT_REAL - %.2f\n", mesaj.ip, mesaj.port, mesaj.topic, nr_aux);
				}
				// Primeste un float
				if (mesaj.tipp + 48 == '2')
				{
					uint8_t semn;
					uint32_t nr;
					uint8_t virgule;

					memset(&semn, 0, 1);
					memset(&nr, 0, 4);
					memset(&virgule, 0, 1);

					memcpy(&semn, buffer + 57, 1);
					memcpy(&nr, buffer + 58, 4);
					memcpy(&virgule, buffer + 62, 1);

					nr = ntohl(nr);
					float nr_aux = nr;
					for (int i = 0; i < virgule; i++)
					{
						nr_aux = nr_aux / 10;
					}
					if (semn == 0)
						printf("%s:%s - %s - FLOAT - %.*f\n", mesaj.ip, mesaj.port, mesaj.topic, virgule, nr_aux);
					else
						printf("%s:%s - %s - FLOAT - %.*f\n", mesaj.ip, mesaj.port, mesaj.topic, virgule, -nr_aux);
				}
				// Primeste un String
				if (mesaj.tipp + 48 == '3')
				{
					strcpy(mesaj.text, buffer + 57);
					printf("%s:%s - %s - STRING - %s\n", mesaj.ip, mesaj.port, mesaj.topic, mesaj.text);
				}
			}
		}
	}
	// Inchid socketul
	close(sockfd);

	return 0;
}
