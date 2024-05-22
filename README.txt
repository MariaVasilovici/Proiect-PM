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

Registrele de deplasare (shift registers) au fost folosite deoarece aveam nevoie de mai multi pini I/O decat sunt pe Arduino. Astfel, am folosit doua registre de tip SIPO (serial-in parallel-out) si acum pot controla cele 5 LED-uri RGB (care ar avea nevoie in total de 15 pini) cu doar 3 pini de output din Arduino.

Pini Arduino folositi:
    - 2-8: tastatura numerica
    - 9: buzzer
    - 10-13: modul card microSD
    - A1-A3: registru de deplasare pentru LED-uri
    - A4-A5: 


Principalele functii implementate:

   * guessNumber() reprezinta implementarea primului joc. Initial se genereaza un numar, iar apoi, intr-o bucla infinita, utilizatorul poate introduce cate 4 cifre pentru a-l ghici. Jucatorul are incercari nelimitate, iesindu-se din bucla de ghicituri doar atunci cand numarul corect este introdus.

   * guessWord() este cel de-al doilea joc. Dupa generarea cuvantului, numerele introduse de utilizator prin intermediul tastaturii sunt transformate in litere cu ajutorul functiei numbersToWord(). Dupa introducerea celor 5 litere, se va verifica daca acestea alcatuiesc un cuvant valid in limba engleza, verificand fisierul de cuvinte. Apoi se aprind LED-urile corespunzatoare si se afiseaza a cata incercare este aceasta (din maxim 6).

   * checkWord() verifica daca ceea ce a introdus jucatorul se regaseste in vreunul dintre fisierele de cuvinte (altfel, nu este considerat un cuvant valid introdus iar utilizatorul trebuie sa scrie altceva). Initial aveam toate cuvintele intr-un singur fisier dar dura destul de mult sa fie parcurse toate liniile acestuia, asa ca l-am impartit in fisiere .txt pentru fiecare litera. Cu aceasta optimizare, cuvantul de 5 litere introdus se va cauta doar in fisierul literei de inceput a acestuia, lucru realizat aproape instantaneu.

   * delayTimer() este o functie care implementeaza delay folosind Timer1, intocmai ca in laboratorul 3, cu registre (TCCR1B, TIFR1, TCNT1, OCR1A). Aceasta e folosita pentru a afisa diverse lucruri pe ecran doar pentru un anumit timp.

   * changeLeds() se foloseste de shiftOut_16() din biblioteca ShiftOutX.h pentru a trimite culoarea pe care trebuie sa o ia fiecare intre cele 15 LED-uri. Functia este apelata de obicei dupa makeGreen(int nrLed) sau alte culori, si functioneaza astfel: fiecare LED are atribuit cate 3 biti, pentru cele 3 culori RGB (pot fi si combinate). Se alcatuieste un numar unsigned long format din 16 biti (folosesc 15 pentru cele 5 LED-uri) si acesta este transmis catre registrele de deplasare care impart valorile bitilor corespunzator pentru fiecare LED si culoare.

   * lightShow() are scopul de a "rasplati" un joc castigat cu LED-uri colorate care alterneaza in diverse pattern-uri.

   * beepPress() si beepWrong() sunt functii sonore, generate la apasarea tastelor, a doua fiind folosita pentru a indica faptul ca jucatorul nu poate realiza o anumita actiune (de exemplu sa dea enter la mai putin de 5 litere).

   * loseScreen() si winScreen() afiseaza pe ecran mesaje relevante sfarsitului unui joc, iar apoi se va putea alege din nou inceperea unuia dintre cele doua jocuri.



