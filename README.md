<!-- Projekt 2 Wzajemne udostępnianie plików (max 50 punktów)
Aplikacja służąca do wzajemnego udostępniania plików. W projekcie należy opracować zasady takiego udostępniania. Na przykład rozdzielenie zadań między serwer i klienta. Zadania serwera: przyjmowanie nowych klientów oraz logowanie ich w sieci, prowadzenie bazy danych o udostępnianych plikach, odłączanie klientów od sieci. Zadania klienta: dołączanie/odłączanie się od sieci, tworzenie listy udostępnianych plików i przekazywanie jej serwerowi, wyszukiwanie pliku na serwerze i składanie zamówienia na wybrany plik, realizacja złożonych zamówień – przesyłanie pliku. -->

# OGÓLNY OPIS

File Transfer Protocol (FTP) klient-serwer apklikacja ([RFC 959](https://www.rfc-editor.org/rfc/rfc959.txt)) zaimplementowana za użyciem C Stream Sockets biblioteki w języku C.

# SPOSÓB UŻYTKOWANIA FTP PROTOKOŁEM

1. Skopijować FTP Manager C folder w główny katalog użytkownika.

2. W terminalu:
```
	~$ cd FTP\ Manager\ C/

	~$ make
```
*  Ze strony serwera:
```
	~$ cd ./Server/
	~$ ./server <port number>
```
Serwer zostanie podniesiony z ustawieniem nasłuchiwania połączen na wybranym porcie.

* Ze strony klienta:
```
	~$ cd ./Client/
	~$ ./client <IP address of server> <port number>
```
Jako domylśny IP adres serwera jest używany: `127.0.0.1`

3. Dostępne komendy:

	* `ftp>put <filename>` (służy do zaladowania pliku o nazwie `filename` do serwera)
	* `ftp>get <filename>` (służy do pobrania pliku o nazwie `filename` z serwera)
	* `ftp>ls` (służy do wyswietłenia listy plików z bieżącego folderu na serwerze)
	* `ftp>!ls` (służy do wyswietłenia listy plików z bieżącego folderu na kliencie)
	* `ftp>cd <directory>` (służy do zmiany bieżącego folderu na serwerze)
	* `ftp>!cd <directory>` (służy do zmiany bieżącego folderu na kliencie)
	* `ftp>pwd` (służy do wyświetłenia ścieżki do bieżącego folderu na serwerze)
	* `ftp>!pwd` (służy do wyświetłenia ścieżki do bieżącego folderu na kliencie)
	* `ftp>quit` (służy do zamknięcia FTP sesji na kliencie i zwrotu do macierzynskiego środowiska)

# ZAKONCZENIE DZIALANIA

* Można zamknąć serwer za pomocy zwykłego sygnalu `Ctrl + C` w oknie terminalu z uruchomionym serwerem.
* Można zamknąć serwer za pomocy wywolania komendy `"quit"` w oknie terminalu z uruchomionym klientem.

# OGRANICZENIA

* Zaimplementowane są wyłącznie wyżej zaznaczone polecenia.
* Maksymalna liczba jednocześnie zalogowanych clientów nie może przekraczać 8.
* W nazwach ścierzek, które są używane jako bierzący katalog roboczy, nie może zaistnieć żadnych spacji (jest to określono tym, że `cd <directory>` jest zaimplementowana za użyciem `chdir(<directory>)`, która nie pozwala na użycie białych symboli).
* Komenda `get` przepisywuje istniejący plik, jeżeli taki istnieje w katalogu roboczym klienta. Podobnie, `put` komenda przepisywuje istniejący plik, jeżeli taki istnieje w katalogu roboczym serwera.

# WŁAŚCIWOŚCI

* Przyjmuje każdą z powyżej opisanych komend. Ponad tego, odpowiada komunikatem o blędzie w przypadku niepoprawnego polecenia: `"An invalid FTP command"`.
* Dopasowuje polecenia i wypisywuje odpowiednie dane w terminalu.
* Sprawdzenie konca pliku w przypadku `get` i `put` komend jest zaimplementowane za pomocy odwolania w odpowiednim procesie(`server/client`) do rozmiaru(czy liczbie bloków) pliku.
* Działa w trybie połączeniowym i współbieżnym.

# SZCZEGÓŁY IMPLEMENTACYJNE (OPIS DZIAŁANIA)

1. `server.c` - plik zawierający implementacje strony serweru protokolu FTP:

	*  Tworzy `soket` i dowiązuje jego do portu(określonego jako argument, przy wywolaniu procesu).
	*  Zaczyna nasłuchiwać na sokecie.
	*  Przyjmuje połączenie od klienta.
	*  Uzyskuje FTP polecenie od klienta.
	*  Dopasowuje FTP polecenie i sprawdza jego poprawność. Jeżeli wynika błąd podczas zapytu, wysyła komunikat o tym do klienta.
	*  Wszystkie polecenia są podawane w terminalu. Są konwerowane do poprawnej postaci i wywolane(klientem/serwerem w zależności od rządania).
	*  W przypadku uzyskania komendy `get`, sprawdza określony plik i wysyła jego do klienta przez połączenie danych (port danych jest generowany serwerem) w postaci blokowej.
	*  W przypadku uzyskania komendy `put`, uzyskuje określony plik od klienta przez połączenie danych (port danych jest generowany serwerem) w postaci blokowej.
	*  W przypadku uzyskania komendy `ls`, wywolane zostaje polecenie `popen("ls","r")`, zaczym wysyłany spis plikow przez połączenie danych (port danych jest generowany serwerem) w postaci blokowej(lista plików może być dużego rozmiaru).
	*  W przypadku uzyskania komendy `cd <directory>`, wywolane zostaje polecenie `chdir(<directory>)`, zaczym zmieniany katalog roboczy serwera.
	*  W przypadku uzyskania komendy `pwd`, wywolane zostaje polecenie `_getcwd`, jeżeli serwerem jest maszyna pod systemem Windows, czy `getcwd` w przeciwnym przypadku, żeby uzyskać katalog roboczy serwera i nadeśłać jego do klienta.
	*  Zamyka się połączenie z klientem, jeżeli zostało uzyskane polecenie `quit` od klienta.
	*  Czeka na nowe połączenia i powtaża cykliczne opisany proces, dopóki nie zostanie wyłączony.</p>

2. `client.cpp` - plik zawierający implementacje strony klienta protokolu FTP:

	*  Tworzy `soket` i dowiązuje jego do serweru za pomocy IP adresu serweru i portu(określonych jako argumenty, przy wywolaniu procesu).
	*  Dopasowuje FTP polecenie i sprawdza jego poprawność. Jeżeli wynika błąd podczas zapytu, wysyła komunikat o tym do klienta.
	*  Wszystkie polecenia są podawane w terminalu. Są konwerowane do poprawnej postaci i wywolane(klientem/serwerem w zależności od rządania).
	*  W przypadku uzyskania komendy `get`, uzyskuje plik przez połączenie danych (port danych jest generowany serwerem) w postaci blokowej.
	*  W przypadku uzyskania komendy `put`, sprawdza określony plik i wysyła jego do serwera przez połączenie danych (port danych jest generowany serwerem) w postaci blokowej.
	*  W przypadku uzyskania komendy `ls`, uzyskuje spis plikow przez połączenie danych (port danych jest generowany serwerem) w postaci blokowej(lista plików może być dużego rozmiaru).
	*  W przypadku uzyskania komendy `!ls`, użyte zostaje polecenie `system("ls")`, zaczym wypisywany spis plikow z katalogu bierzącego clienta.
	*  W przypadku uzyskania komendy `!cd <directory>`, wywolane zostaje polecenie `chdir(<directory>)`, zaczym zmieniany katalog roboczy klienta.
	*  W przypadku uzyskania komendy `pwd`, wywolane zostaje polecenie `_getcwd`, jeżeli serwerem jest maszyna pod systemem Windows, czy `getcwd` w przeciwnym przypadku, żeby uzyskać katalog roboczy klienta.
	*  Wraca się do macierzynskiego środowiska z klienta, jeżeli użyska polecenie `quit`.

# TESTY

1) Inicjalizacja połączenia:

Strona serwera | Strona klienta
-- | --
`~$./server 9999`</br>Server running...waiting for connections.</br>Received request...</br>Child created for dealing with client requests| `~$./client 127.0.0.1 9999`</br>ftp>

2) `ls`

Strona serwera | Strona klienta
-- | --
String received from client: ls|`ftp>ls`</br>server</br>server.c</br>test2

3) `!ls`

Strona serwera | Strona klienta
-- | --
String received from client: !ls|`ftp>!ls`</br>client</br>client.c</br>test

4) `put`

Strona serwera | Strona klienta
-- | --
String received from client: put test</br>Filename given is: test</br>File download done.|`ftp>put test`</br>File upload done.

5) `get`

Strona serwera | Strona klienta
-- | --
String received from client: get test2</br>Filename given is: test2</br>File upload done.|`ftp>get test2`</br>File download done.

6) `pwd`

Strona serwera | Strona klienta
-- | --
String received from client: pwd|`ftp>pwd`</br>/home/eouser/FTP\ Manager\ C/Server

7) `!pwd`

Strona serwera | Strona klienta
-- | --
String received from client: !pwd|`ftp>!pwd`</br>/Users/napolsky/FTP\ Manager\ C/Client

6) `cd`

Strona serwera | Strona klienta
-- | --
String received from client: cd ../Client</br>Path given is: ../Client|`ftp>cd ../Client`</br>Path given is: ../Client

7) `!cd`

Strona serwera | Strona klienta
-- | --
String received from client: !cd ../Server</br>Path given is: ../Server|`ftp>!cd ../Server`</br>Path given is: ../Server

8) `quit`

Strona serwera | Strona klienta
-- | --
String received from client: quit</br>The client has quit|`ftp>quit	`
