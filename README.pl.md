<h1 align="center">
    IPC Publish-Subscribe 💬
</h1>

_Dostępne również w wersji po: [English](README.md)_

## O projekcie

Stworzony projekt zaliczeniowy pod tytułem "IPC Publish-Subscribe" pozwala na komunikację pomiędzy zalogowanymi użytkownikami systemu, przekazując ich rozgłaszane wiadomości na podstawie wcześniej zasubskrybowanych przez nich typów. Odpowiedzialny za poprawne działanie systemu jest serwer, przez który przechodzą wszystkie działania, a zatem również sama komunikacja pomiędzy klientami jest pośrednia, gdyż zawsze uczestniczy w niej wspomniany serwer.

Funkcjonalność klienta
========
1. Logowanie

Klient loguje się do systemu podając swój identyfikator oraz nazwę, co następnie zostaje przetworzone przez serwer, który wysyła informację zwrotną do klienta o rezultacie jego żądania. Możliwe rezultaty to informacja o pomyślnym logowaniu, informacja o tym, iż w systemie widnieje już użytkownik o takim identyfikatorze, informacja o tym, iż w systemie widnieje już użytkownik o takiej nazwie, informacja o tym, iż w systemie widnieje już taki użytkownik (podano zarówno identyfikator jak i nazwę widniejące już w systemie). W przypadku gdy klient otrzyma jedną z ostatnich trzech wymienionych informacji, ma on prawo do ponownej próby zalogowania się, lecz w przypadku gdy przekroczy on maksymalną liczbę prób (stała MAX_LOGIN_ATTEMPTS, którą można dowolnie konfigurować) domyślnie ustawioną na limit trzech, to serwer zablokuje możliwość kolejnych prób i zamknie program klienta. Istnieje jeszcze jeden możliwy komunikat od serwera w trakcie próby zalogowania się, jest to informacja o tym, iż serwer jest zapełniony, co oznacza, że do systemu zalogowali się użytkownicy w liczbie określonej przez stałą MAX_NUMBER_OF_USERS, którą również można dowolnie zmieniać i domyślnie ustawiona jest na pięciu klientów.

2. Wyświetlanie menu

Funkcja wyświetlająca użytkownikowi działania, które może wykonać w ramach programu.

3. Wyświetlenie dostępnych typów wiadomości do subskrypcji

Działanie polegające na przekazaniu przez serwer informacji do klienta o wszystkich zarejestrowanych typach wiadomości w systemie, podając ich identyfikatory.

4. Rejestracja do systemu rozgłaszania

Działanie polegające na subskrypcji danego typu wiadomości, w celu otrzymywania później wiadomości tego typu od innych użytkowników. Istnieją dwa rodzaje subskrypcji - stała i przejściowa, subskrypcja stała jest dożywotnia, a przejściowa pozwala na otrzymywanie tylko ograniczonej ilości wiadomości danego typu. Subskrypcja i ilość otrzymywanych wiadomości w przypadku wyboru subskrypcji przejściowej dokonywana jest przez samego klienta w trakcie rejestracji. Dodatkowo klient otrzymuje informacje zwrotną od serwera, o tym czy udało się zasubskrybować dany typ wiadomości, a w przeciwnym przypadku informację o tym, dlaczego się nie udało (w bazie nie istnieje podany przez użytkownika identyfikator typu wiadomości lub po prostu użytkownik już wcześniej zapisał się na odbiór tego typu wiadomości).

5. Rejestracja typu wiadomości

Działanie polegające na dodaniu do bazy serwera nowego typu wiadomości o podanym przez klienta identyfikatorze i nazwie, bądź zażądaniu utworzenia nowego typu wiadomości przez serwer. W przypadku tego drugiego, serwer tworzy nowy typ wiadomości o nazwie "systemowy typ" i najniższej dostępnej wartości identyfikatora (większej od 0). W przypadku sukcesu klient otrzyma informację o powodzeniu, a aktualnie zalogowani w systemie inni użytkownicy natychmiast dostaną informację o tym, iż został utworzony nowy typ wiadomości. W przypadku niepowodzenia, serwer poda informację do procesu rejestrującego nowy typ wiadomości o tym, dlaczego działanie się nie powiodło (istnieje typ o podanym identyfikatorze, istnieje typ o podanej nazwie, bądź istnieje dokładnie taki sam typ).

6. Rozgłoszenie nowej wiadomości

Działanie polegające na rozgłoszeniu nowej wiadomości w systemie, podając przy tym identyfikator jej typu oraz priorytet.

7. Odebranie wiadomości w sposób synchroniczny

Działanie polegające na odbiorze jednej, bądź wielu wiadomości typów zasubskrybowanych przez klienta i wyświetleniu ich w kolejności według ich priorytetów, to znaczy, iż wiadomość o wyższym priorytecie zostanie wyświetlona jako pierwsza. W przypadku gdy żadna wiadomość nie została wysłana do klienta uruchamiającego odbiór synchroniczny, proces blokuje się do czasu pojawia się wiadomości skierowanej do niego.

8. Odebranie wiadomości w sposób asynchroniczny

Działanie polegające na odbiorze jednej, bądź wielu wiadomości typów zasubskrybowanych przez klienta i wyświetleniu ich w kolejności według ich priorytetów, to znaczy, iż wiadomość o wyższym priorytecie zostanie wyświetlona jako pierwsza. W przypadku gdy żadna wiadomość nie została wysłana do klienta uruchamiającego odbiór asynchroniczny, proces odbioru wiadomości będzie działać w tle, to znaczy, iż klient dalej może korzystać z programu, lecz w dowolnym momencie, w którym otrzyma wiadomość skierowaną do niego, zostanie ona od razu wyświetlona na ekranie, a klient będzie mógł następnie dalej kontynuować swoją pracę. W przypadku gdy klient spróbuje odebrać wiadomość w sposób asynchroniczny drugi raz, mimo braku odbioru wiadomości od ostatniego wywołania odbioru asynchronicznego, zostanie wyświetlony odpowiedni komunikat. Po odebraniu wiadomości w sposób asynchroniczny, do kolejnego odbioru przyszłych wiadomości również w sposób asynchroniczny konieczny jest ponowny wybór odpowiedniego działania przez klienta (w tym przypadku numer 7).

9. Blokowanie użytkowników

Działanie polegające na zablokowaniu danego użytkownika, to znaczy brak odbioru od niego wiadomości mimo zgodnego zasubskrybowanego typu wiadomości. Przy blokowaniu klient otrzyma informację o tym czy operacja się powiodła, a jeśli nie to informację o tym z jakiego powodu się nie powiodła. Dodatkowo należy tutaj wspomnieć, iż przyjęto, że w przypadku subskrypcji przejściowej wiadomość od zablokowanego użytkownika również zmniejsza licznik pozostałych wiadomości do odbioru.

10. Wylogowanie

Działanie polegające na wylogowaniu się z systemu przez klienta.

Serwer
========
Stworzony serwer dodatkowo wypisuje informacje o podejmowanych przez niego działaniach i ich skutkach, co było pomocne i wygodne w trakcie pisania czy wykonywania testów programu. Dodatkowo wprowadzone zostało ograniczenie czasowe jego działania, to znaczy, iż serwer będzie działał maksymalnie tyle sekund (po tym jak ostatni klient wyloguje się z systemu) ile wynosi wartość SERVER_STANDBY_TIME, którą można dowolnie konfigurować, a domyślnie ustawiona jest na 10 sekund.

Zawartość poszczególnych struktur
========
W przypadku programu klienta, struktura o nazwie "user" przechowuje informacje o identyfikatorze i nazwie danego użytkownika, jego subskrypcjach czy zablokowanych przez niego innych użytkownikach. Struktura o nazwie "messages", przechowuje zawsze chwilowo odebrane przez klienta wiadomości wraz z ich priorytetami w celu ich posortowania do odpowiedniego wyświetlenia według priorytetu.

W przypadku programu serwera, struktura o nazwie "users" przechowuje informacje o ilości, identyfikatorach czy nazwach wszystkich zalogowanych użytkowników. Struktura o nazwie "messages_types" przechowuje informacje o identyfikatorach i nazwach typów wiadomości, użytkownikach (identyfikatory i nazwy) i sposobie subskrypcji odpowiednich typów wiadomości przez tych użytkowników (wraz z ilością odbieranych wiadomości w przypadku subskrypcji przejściowej). 

Kompilacja
========
Unix:

<code>gcc -Wall server.c -o server</code>\
<code>gcc -Wall client.c -o client</code>

Uruchamianie
========
Uruchamianie programu rozpoczynamy od serwera:

<code>./server</code>

Następnie w każdym nowym oknie konsoli mamy możliwość uruchomienia programu klienta, komunikującego się z innymi uruchomionymi w ten sposób klientami (oczywiście pośrednio przez serwer):

<code>./client</code>
