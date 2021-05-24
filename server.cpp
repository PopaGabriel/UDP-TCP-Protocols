#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <unordered_set>
#include "helpers.h"
#include "structura.h"

using namespace std;

// Transforma valoarea unui port intr-un string
// si o salveza intru-un char pe care il primeste
void transform_int_char(int port, char *final)
{
	char initial[10];
	int k = 0, i = 0;

	while (port)
	{
		initial[k++] = port % 10 + 48;
		port = port / 10;
	}
	k--;
	while (k > -1)
	{
		final[i++] = initial[k--];
	}
	final[i] = '\0';
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	char buffer[2000];
	char received_mesage[2000];
	struct sockaddr_in serv_addr, cli_addr;
	int n, i, ret;
	socklen_t clilen;

	int sockets[1000];
	int no_sockets = 0;

	fd_set read_fds;
	fd_set tmp_fds;
	int fdmax;
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	if (argc < 2) {return 0;}
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket TCP ERROR");

	int sockfdUDP = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(sockfdUDP < 0, "socket UDP ERROR");
	
	portno = atoi(argv[1]);
	DIE(portno == 0, "atoi");

	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(portno);
	addr.sin_addr.s_addr = INADDR_ANY;

	// Initializare socket inactiv UDP
	int b = bind(sockfdUDP, (struct sockaddr *)&addr, sizeof(addr));
	DIE(b < 0, "udpSocket");

	// Initializare socket inactiv TCP
	ret = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind");

	int flag = 1;
	int result = setsockopt(sockfd, IPPROTO_TCP, _NETINET_IN_H, (char *)&flag, sizeof(int));
	DIE(result < 0, "Nevile Error");

	ret = listen(sockfd, MAX_CLIENTS);
	DIE(ret < 0, "listen");

	//Adaugam in lista temorara si cea normala
	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(sockfd, &read_fds);
	FD_SET(sockfdUDP, &read_fds);
	fdmax = sockfd;

	//map cu toate topicurile din server si clientii lor
	unordered_map<string, struct clienti> topic_subscriptions_map;

	// map cu toate mesajele nereceptionate de un user abonat dar care
	// nu este online
	unordered_map<string, struct sf_topics> sf_messages;

	// map cu toate legaturile id-socket
	unordered_map<string, int> id_socket_map;

	// care imi arata cine este online in momentul de fata
	unordered_map<string, int> online_clients;

	while (1)
	{
		tmp_fds = read_fds;

		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (i = 0; i <= fdmax; i++)
		{
			if (FD_ISSET(i, &tmp_fds))
			{
				// Daca este input de la tastatura
				if (i == STDIN_FILENO)
				{
					// Se citeste de la tastatura
					scanf("%s", buffer);
					// Se trimite tuturor socketilor conectati un mesaj de exit
					// Pentru ca se inchide serverul
					if (!strcmp(buffer, "exit"))
					{
						for (int j = 0; j < no_sockets; j++)
						{
							send(sockets[j], buffer, strlen(buffer), 0);
							close(j);
						}
						close(sockfd);
						close(sockfdUDP);
						return 0;
					}
				}
				// Cerere de conexiune pe socketul inactiv
				// Serverul o accepta si verifica daca este deja conectat
				// Sau daca nu a fost vreodata
				else if (i == sockfd)
				{
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
					DIE(newsockfd < 0, "accept error");
					
					// Am nevoie si de id-ul celui care s-a conectat
					// Asa ca ii trimit cerere de id pe socketul pe care a intrat
					// Si astept sa primesc id-ul

					memset(buffer, 0, 2000);
					strcpy(buffer, "id");
					n = send(newsockfd, buffer, 2000, 0);
					DIE(n < 0, "send id error");

					memset(buffer, 0, 2000);
					n = recv(newsockfd, buffer, 2000, 0);
					DIE(n < 0, "Id not received");

					// Aici verific daca clientul a mai fost vreodata conectat la server
					if (online_clients.find(string(buffer)) == online_clients.end())
					{
						// Daca nu a mai fost vreodata aici atunci il adaug in lista de socketi
						// Il introduc in map-ul pentru oamenii care sunt conectati
						// Cat si in cel in care ii salvez socketul pe care este conectat
						char id[50];
						result = setsockopt(newsockfd, IPPROTO_TCP, _NETINET_IN_H, (char *)&flag, sizeof(int));
						DIE(result < 0, "Nevile Error");

						FD_SET(newsockfd, &read_fds);

						if (newsockfd > fdmax)
						{
							fdmax = newsockfd;
						}
						// Il adaug in lista tuturor socketilor conectati la server
						sockets[no_sockets] = newsockfd;
						no_sockets += 1;

						strcpy(id, buffer);
						id_socket_map.insert(make_pair(string(id), newsockfd));
						online_clients.insert(make_pair(string(id), 1));
						printf("New client %s connected from %s:%d.\n", id, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
					}
					// Acesta a mai fost conectat la server inainte
					else
					{
						// Aici verifiic daca clientul este momentan on si incearca deci sa se conecteze
						// De pe alt device la server, daca nu este online si nu nici un socket conectat
						// Al lui atunci inseammna ca se reconecteaza si trebuie sa updatez map-ul
						// de online si de id-socket
						if (online_clients[string(buffer)] == -1 && id_socket_map[string(buffer)] == -1)
						{
							result = setsockopt(newsockfd, IPPROTO_TCP, _NETINET_IN_H, (char *)&flag, sizeof(int));
							DIE(result < 0, "Nevile Error");

							FD_SET(newsockfd, &read_fds);

							if (newsockfd > fdmax)
							{
								fdmax = newsockfd;
							}

							sockets[no_sockets] = newsockfd;
							no_sockets += 1;
							
							// Acum clientul este on
							online_clients[string(buffer)] = 1;

							// noul socket cu care este conectat
							id_socket_map[string(buffer)] = newsockfd;

							// Daca s-a reconectat, automat verific daca cat timp acesta a fost plecat
							// a primit vreun mesaj sau notificare pe un topic
							if (sf_messages.find(string(buffer)) != sf_messages.end())
							{
								char mesaj[2000];
								// Aici ii trimit toate pachetele pe care acesta nu le-a primit
								// cat timp a fost plecat
								for (int j = 0; j < sf_messages[string(buffer)].size; j++)
								{
									memcpy(mesaj, sf_messages[string(buffer)].vector[j], 2000);
									n = send(newsockfd, mesaj, 2000, 0);
									DIE(n < 0, "Send SF Error");
								}
								sf_messages[string(buffer)].size = 0;
							}

							printf("New client %s connected from %s:%d.\n", buffer, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
						}
						else
						{
							// Daca acesta este deja online atunci inseamna ca el incearca
							// Sa se conecteze din greseala sau eronat
							printf("Client %s already connected.\n", buffer);
							memset(buffer, 2000, 0);
							strcpy(buffer, "exit");

							n = send(newsockfd, buffer, 2000, 0);
							DIE(n < 0, "send");
						}
					}
				}
				// Acesta este un mesaj de la un client UDP
				else if (i == sockfdUDP)
				{
					clilen = sizeof(cli_addr);

					struct mesajUDP *de_trimis = (struct mesajUDP *)malloc(sizeof(struct mesajUDP));
					char mesaj[2000];
					char topic[51];
					memset(mesaj, 0, 2000);

					n = recvfrom(sockfdUDP, mesaj, 2000, 0, (struct sockaddr *)&cli_addr, &clilen);
					DIE(n < 0, "receive");

					// Prelucrez mesajul in structura pe care doresc sa o trimit
					// In care includ portul si ip-ul clientului UDP
					// Cat restul de mesaj UDP
					strcpy(de_trimis->udp, "UDP");
					strcpy(de_trimis->topic, mesaj);
					strcpy(topic, mesaj);
					de_trimis->tipp = mesaj[50];
					memcpy(de_trimis->text, mesaj + 51, 1500);
					transform_int_char(ntohs(cli_addr.sin_port), de_trimis->port);
					strcpy(de_trimis->ip, inet_ntoa(cli_addr.sin_addr));

					char id[50];
					// Verific daca a fost creeat deja topicul mesajului de la UDP
					// Daca nu atunci il ignor
					if (topic_subscriptions_map.find(string(topic)) != topic_subscriptions_map.end())
					{
						// Daca exista deja topicul deschis
						// Iterez prin toti clientii abonati
						for (int j = 0; j < topic_subscriptions_map[string(topic)].size; j++)
						{
							// Da clientii nu sunt subscribed atunci nu le trimit nimic
							if (topic_subscriptions_map[string(topic)].socketi_clienti[j]->subscribed == 1)
							{
								strcpy(id, topic_subscriptions_map[string(topic)].socketi_clienti[j]->id_client);

								//Daca clientul este online atunci ii trimim mesajul de UDP
								if (online_clients[string(topic_subscriptions_map[string(topic)].socketi_clienti[j]->id_client)] == 1)
								{
									n = send(id_socket_map[string(id)], de_trimis, 2000, 0);
									DIE(n < 0, "Send UDP ERROR");
								}
								// Daca are SF activat atunci ii salvez mesajul de la UDP in lista de mesaje 
								// pe care trebuie sa le primeasca clientul la reconectare
								else if (topic_subscriptions_map[string(topic)].socketi_clienti[j]->SF - 48 > 0)
								{
									// Daca nu are salvat deja un camp al lui
									// Inseamna ca e prima data cand primeste un mesaj cand el
									// Nu este activ, si deci atunci ii creez eu un camp
									// Si ii salvez mesajul acolo
									if (sf_messages.find(string(id)) == sf_messages.end())
									{
										struct sf_topics sf_topic_ms;
										sf_topic_ms.size = 1;
										sf_topic_ms.size_max = 100;
										sf_topic_ms.vector = (char **)malloc(100 * sizeof(char *));
										char *message = (char *)malloc(2000);
										memcpy(message, de_trimis, 2000);

										sf_topic_ms.vector[0] = message;
										sf_messages.insert(make_pair(string(id), sf_topic_ms));
									}
									else
									{
										// Daca nu este primul mesaj de tip UDP cu SF 1 pe care il primeste
										// Cand nu este conectat atunci il adaug in lista de mesaje pe care trebuie
										// Sa le primeasca
										char *message = (char *)malloc(2000);
										memcpy(message, de_trimis, 2000);

										sf_messages[string(id)].vector[sf_messages[string(id)].size++] = message;
									}
								}
							}
						}
					}
				}
				else
				{
					// Acesta este un mesaj de la client
					memset(received_mesage, 0, 2000);
					n = recv(i, received_mesage, 2000, 0);
					DIE(n < 0, "recv");

					// Daca acesta este de tip unsubscribe
					// Atunci verific daca exista topicul salvat in server
					// si dupa parsez toti clientii abonati la topic
					// si cand il gasesc ii schimb valoarea in -1
					// adica unsubscribed
					if (strncmp(received_mesage + 65, "unsubscribe", 11) == 0)
					{
						int socket_aux = id_socket_map[string(received_mesage + 2)];
						if (topic_subscriptions_map.find(string(received_mesage + 13)) != topic_subscriptions_map.end())
						{
							// Parcurg toti clientii subscribed si daca il gasesc pe cel dorit ii schimb valoarea
							for (int j = 0; j < topic_subscriptions_map[string(received_mesage + 13)].size; j++)
							{
								if (topic_subscriptions_map[received_mesage + 13].socketi_clienti[j]->subscribed == 1)
								{
									topic_subscriptions_map[received_mesage + 13].socketi_clienti[j]->subscribed = -1;
									break;
								}
							}
						}
					}
					// Daca clientul vrea sa se aboneze atunci
					else if (strncmp(received_mesage + 65, "subscribe", 9) == 0)
					{

						char topic[51];
						strcpy(topic, received_mesage + 13);
						// Verific daca exista deja topicul deschis
						// Daca nu atunci pur si simplu il creez eu
						// Si imi abonez clientul la acel topic
						if (topic_subscriptions_map.find(string(topic)) == topic_subscriptions_map.end())
						{
							// Aici creez structura care contine nr de abonati si fosti abonati
							// Cat si un vector plin cu detalii despre clienti
							struct clienti clienti_noi;
							clienti_noi.socketi_clienti = (struct id_topica **)malloc(sizeof(struct id_topica *) * 100);

							clienti_noi.size_max = 100;
							clienti_noi.size = 1;

							// Creez structura pentru client
							// Care contine detalii precum daca este sau nu un client subscribed
							// si daca acesta are SF activat
							// De asemenea si detalii despre id-ul clientului;
							struct id_topica *client_nou = (struct id_topica *)malloc(sizeof(struct id_topica));
							strcpy(client_nou->id_client, received_mesage + 2);
							client_nou->subscribed = 1;
							client_nou->SF = received_mesage[0];

							clienti_noi.socketi_clienti[0] = client_nou;
							topic_subscriptions_map.insert(make_pair(string(topic), clienti_noi));
						}
						// Inseamna ca topicul a fost deja deschis de altcineva inaintea clientului curent
						else
						{
							int ok = 0;
							// Caut prin lista de abonati sa vad daca este un fost abonat care s-a
							// Dezabonat inainte si acum vrea sa se reaboneze
							for (int j = 0; j < topic_subscriptions_map[string(topic)].size; j++)
							{
								// Verific daca acesta este clientul meu
								if (strcmp(topic_subscriptions_map[string(topic)].socketi_clienti[j]->id_client, received_mesage + 2) == 0)
								{
									// In cazul in care acesta este unsubscribed atunci il reabonez
									if (topic_subscriptions_map[string(topic)].socketi_clienti[j]->subscribed == -1)
									{
										topic_subscriptions_map[string(topic)].socketi_clienti[j]->subscribed = 1;
										topic_subscriptions_map[string(topic)].socketi_clienti[j]->SF = received_mesage[0];
									}
									ok = 1;
									break;
								}
							}
							// Daca nu a fost gasit abonatul atunci inseamna ca trebuie sa ii creez eu o noua structur
							// cu datele lui pe care sa le salvez in structura de clienti al topicului
							if (ok == 0)
							{
								struct id_topica *client_nou = (struct id_topica *)malloc(sizeof(struct id_topica));
								strcpy(client_nou->id_client, received_mesage + 2);
								client_nou->subscribed = 1;
								client_nou->SF = received_mesage[0];
								topic_subscriptions_map[string(topic)].socketi_clienti[topic_subscriptions_map[string(topic)].size++] = client_nou;
							}
						}
					}
					// Daca acesta doreste sa se deconecteze de pe server ii updatez datele despre socketul
					// curent cat si despre faptul daca este online
					else if (n == 0 || strncmp(received_mesage + 65, "exit", 4) == 0)
					{
						printf("Client %s disconnected.\n", received_mesage + 2);
						online_clients[string(received_mesage + 2)] = -1;
						id_socket_map[string(received_mesage + 2)] = -1;

						// Ii scot socketul din lista de socketi
						FD_CLR(i, &read_fds);
					}
				}
			}
		}
	}

	return 0;
}
