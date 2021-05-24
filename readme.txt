Nume: Popa Catalin Gabriel
Grupa: 324CC

Tema2: Protocoale de comunicatii README

				Detalii tema

  Tema a durat in jur de 4 zile, majoritatea timpului a fost pierdut pe checker
deoarece acesta uneori merge alteori nu, uneori imi da maxim, alteori pica
teste de sf la subscriber 2.
  Depinde foarte mult de cate ori a fost folosit portul inainte, am observat ca
la un restart al masinii virtuale nimic nu mai pica, de asemenea am incercat sa
fac testele care pica de mana si nu am putut sa reproduc eroarea :(.

  De asemenea am schimbat numele din fisierului client.c in subscriber.c,
si am folosit C++ pentru accesul la structuri deja definite in limbaj cum este
map-ul. Ma folosesc de acestea pentru a imi usura viata in ce inseamna accesul
la anumite date, precum ce topicuri sunt pe server deschise daca un client
este online, daca un client are mesaje pe care trebuie sa le primeasca la
reconectare pe server etc.
  Am implementat toate cerintele din tema, serverul poate comunica si cu
clientii TCP si UDP, cat si sa foloseasca functiile de subscribe unsubscribe
astfel incat sa primeasca mesaje de la UDP. De asemenea si functia de store
forward este implementata, de asemenea algoritmul lui Neagle este dezactivat,
de asemenea nu am ignorat nici un apel de sistem.
  Am folosit 2 structuri mari pentru mesaje unul de TCP si unul de UDP, cel de
TCP este de obicei primit de la clientul TCP de catre server, si contine atat
un mesaj care in cazul acesta este doar un placeholder pentru mesaje adica
pentru 'exit', subscribe si unsubscribe. De asemenea contine un camp de tip
care nu reprezinta decat valoarea SF, de asemenea un camp topic care imi arata
despre ce topic este vorba.
  A doua structura este de tip mesaj de la UDP, ea contine un camp in care
scriu 'UDP' pentru ca un client sa stie ca este vorba de un mesaj de tip UDP
venit dinspre server, de asemenea acesta prezinta topicul, si mesajul care este
salvat in campul text si care trebuie afisat la stdout. Mai exista un camp care
imi arata ce tip de mesaj este, adica poate lua 4 valori(String, float, int,
shor_real). Am adaugat 2 campuri pentru a putea trimite de la server la client
ip-ul clientului UDP, cat si portul pe care a venit mesajul.


				Detalii Implementare

		Server:

	Detalii generale server:

  Am initializat 2 socketi la inceput, amandoi sunt inactivi, unul este de tip
TCP pe care primeste doar mesaje de cerere de conectare la server de la
clientii TCP, de asemenea unul pentru UDP care este folosit in acelasi scop
aproximativ, doar ca clientii UDP nu se aboneaza sau se inregistreaza pe server
ei doar trimit date serverului, si dupa serverul se ocupa de transmiterea lor
mai departe. De asemenea serverul poate primii mesaje de la stdin, dar singura
comanda pentru care face ceva este comanda "exit" care inchide serverul si
trimite tuturor clientilor conectati pe server mesajul "exit", care inchide
practic clientii, dar acest lucru o sa il explic mai in amanunt la paragraful
Client.
  Am folosit 4 map-uri in server, acestea sunt explicate si in cod, dar o sa
le explic si aici, unul dintre ele il folosesc pentru a vedea cine este online
in orice moment de timp, iar atunci cand acesta se deconecteaza este updatata
si valoarea din map-ul respectiv. Un alt map este folosit pentru a creea
relatia intre un id si socket-ul prin care primeste acesta mesaje de la server
, un altul reprezinta toate topic-urile deschise pe server impreuna cu date
despre clientii care sunt sau au fost abonati. Datele continute sunt daca
subscriber-ul este abonat, daca acesta are sau nu activata functia de SF si
id-ul acestuia. Ultimul map este relatia dintre un id si mesajele pe care
nu le-a primit cand a fost deconectat de la server dar avea SF activat. Toate
aceste map-uri fac cautarea si adaugarea sa fie in O(1), de asemenea pot fi
accesate sau modificate de oriunde din cod.

	Cerere de conectare TCP:

  Serverul poate primii mesaje pe socketul TCP inactiv, in acest caz doresc sa
primesc si id-ul clientului, astfel trimit un mesaj inapoi pe socketul cu care
a vrut sa se conecteze clientul. Verific prin id-ul primit daca cumva am mai
avut acest client conectat la server, caz in care inseamna ca acesta se
reconecteaza caz in care eu trebuie sa ii updatez valorile din map-ul care
contine perechea id-socket cu noul socket, si desigur sa ii modific practic
valoarea din map-ul prin care vad daca acesta este sau nu online. Adaug noul
socket in lista de socketi si continui.
  Celalalt caz este cand el este deja conectat, si practic incearca sa se
conecteze degeaba, asa ca ii trimit inapoi pe socket mesajul 'exit' ca sa
inchid clientul care a trimis cererea. Daca acest client nu a mai fost conectat
la server pana acum, ii voi creea in map-ul prin care vad oamenii online, cat
si in cel in care tin perechea id-socket un entry nou.

	Comenzi de la clienti UDP la server:

  Urmatorul tip de mesaj este de la clientii UDP, practic atunci cand primesc
un mesaj de acest tip il parsez si imi scriu eu tot ce am nevoie in el. Verific
daca topicul este in lista mea de topic-uri de pe server, daca nu, atunci il
ignor. Daca da insa, o sa parcurg toti clientii care au fost abonati sau sunt
abonati si verific care este abonat si care nu. In cazul in care acestia nu
sunt abonati atunci ii ignor si merg mai departe, daca sunt insa abonati atunci
ii caut sa vad daca sunt online, daca sunt online le trimit direct pachetul,
iar daca nu sunt verific daca sunt abonati cu SF 1. Caz in care le salvez
mesajul intr-un map, in ordinea in care au aparut, daca acesta este abonat cu
SF 1, daca nu atunci ignor.
	Am folosit o functie auxiliara care sa imi transforme valoarea portului
intr-un string, ca sa poata fi trimis super usor prin intermediul mesajului
creeat.

	Comenzi de la clienti TCP la server:

  Urmatorarele lucruri pe care le primeste serverul sunt mesaje pe socketii de
la clinetii deja conectati la server, acestea sunt de 3 tipuri 'exit',
'subscribe' si 'unsubscribe'. Daca serverul primeste comanda exit de la un
client inseamna ca acestia se deconecteaza de la server, si eu atunci le
modific valorile din map-uri atat in cel id-socket cat si in cel id-online
facandu-le -1, ceea ce semnifica ca acestia sunt deconectati.
  Comanda 'subscribe' o primeste de la un user, care doreste sa se aboneze la
un anumit topic, in primul rand verific daca topicul are deja deschis entry-ul
in map-ul topic-clienti_abonati, daca nu atunci creez eu un nou entry cu
singurul abonat momentan clientul care a trimis cererea. Daca topicul a fost
deja deschis mai devreme de catre un alt client atunci caut prin vectorul de
clienti abonati sa vad daca clientul care a trimis cererea a fost conectat de
fapt in trecut, daca a fost gasit atunci ii updatez valorile cu cele primite
(adica valoarea de subscribe si de sf). Daca acesta nu a fost gasit in vectorul
de clienti al topicului atunci ii salvez datele anume id-ul si sf-ul ii intr-o
structura care sa primeasca acestea, si tot in acea structura ii adaug si
valoarea de subscribed sa fie 1.
  Comanda 'unsubscribe' o primeste de la un client care doreste sa se
dezaboneze de la un anumit topic, astfel primul lucru pe care il fac este sa
vad daca exista deja un topic in map-ul topic-clienti, daca nu exista atunci
pur si simplu ignor comanda, daca exista atunci iterez prin toti clientii
pe care ii are acel topic si verific daca vreunul dintre ei este cel care vrea
sa se dezaboneze, daca este atunci ii schimb doar valoarea pe care o are in
topic in subscribed in -1, ca sa arat ca acesta nu mai este abonat. Daca nu il
gasesc atunci ignor comand si merg mai departe.

		Client:

	Mesaje de la stdin:

  Clientul poate primii mesaje de la stdin, si acestea sunt de 3 tipuri fie
un mesaj de unsubscribe care contine pe langa comanda si topicul, fie un mesaj
de unsubscribe care contine pe langa topic si valoarea SF, cat si comanda exit.
  Daca mesajul este 'exit' atunci clientul trimite mesaj serverului ca se
inchide si clientul se inchide, daca mesajul este subscribe atunci este format
un mesaj care contine o toate detaliile descrise mai sus, si le trimite
serverului, la fel si in cazul in care acesta primeste unsubscribe. Mesajele
eronate de exemplu greseala de tipar la primul cuvant, acestea sunt ignorate.

	Mesaje de la server:

  Acestea sunt de 2 feluri, ori mesaj prin care serverul cere id-ul de la client
si acesta o returneaza, ori mesaj de tip UDP, pe care il sparg in componentele
necesare, si le afisez, daca acestea sunt de tip string atunci pur si simplu le
printez, daca sunt de tip float atunci am in vedere bitul de semn, si numarul
de virgule, castez practic valoarea primita in float, o impart de atatea ori
cat imi arata bitul de putere al lui 10, si ma folosesc de puterea printf-ului
si folosesc optiunea de %.*f pentru a arata cu atatea virgule cat este necesar.
  Pentru short int procedez in acelasi mod ca la float doar ca de data asta pot
pune direct in loc de * 2, in cazul intului este vorba de acelasi procedeu,
doar ca de data asta ii dau cast la int si in functie de bitul de semn il
afisez pe ecran.

