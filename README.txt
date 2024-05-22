Proiectul implementat consta in doua minigame-uri similare, in care utilizatorul trebuie sa ghiceasca fie un numar de 4 cifre, fie un cuvant in limba engleza format din 5 litere.

Primul joc este inspirat din Mastermind (board game), acela folosind culori in loc de cifre. Pentru a ghici numarul generat random, jucatorul poate incerca in mod repetat sa introduca 4 cifre. La fiecare incercare, i se va indica pe display cate dintre cifrele alese sunt in numarul pe care trebuie sa il ghiceasca, si cate dintre acestea sunt si pe pozitia corecta. Numerele generate sunt cuprinse intre 1000 si 9999, avand toate cifrele distincte.

Jocul cu litere se bazeaza pe Wordle, utilizatorul avand doar 6 incercari sa descopere cuvantul. Nu pot fi introduse 5 litere oarecare, luandu-se in considerare doar cuvintele valide in limba engleza. Spre deosebire de primul joc, aici se va stii exact care dintre litere sunt pe pozitia corecta sau nu, cu ajutorul LED-urilor pozitionate deasupra acestora (verde = corect, galben = litera exista in cuvant dar pe alta pozitie, LED stins = litera nu exista in cuvant).

Ambele jocuri au efecte sonore, care se pot dezactiva prin apasarea tastei 3 in scena de start.

Ca input, jucatorul se va folosi de cele 12 butoane (cifre de la 0 la 9, avand asociate si cate 3-4 litere fiecare, in stilul telefoanelor cu butoane, si inca doua butoane: Delete si Enter). Initial, acesta va alege unul dintre cele doua jocuri, apasand pe 1 sau 2. Cuvintele de 5 litere sunt stocate intr-un fisier de pe MicroSD.



Listă de piese:
    - Arduino UNO
    - Tastatura numerica
    - LCD cu interfata I2C
    - Card MicroSD
    - 5 x LED RGB
    - Buzzer pasiv
    - 2 x Registru de deplasare 74hc595
    - Fire, Breadboard, Rezistente

