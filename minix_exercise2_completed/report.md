# SOI Zadanie 2 - planista
# Raport autorstwa Kacpra Skrodzkiego

## Wstęp - założenia planisty

W ramach drugiego zadania należało utworzyć w systmie MINIX 2.0.3 planistę obsługującego trzy grupy priorytetowe: 1, 2 i 3, gdzie najwyższy priorytet jest dla grupy 1 oraz każda z grup używała innego algorytmu szeregowania. Pierwsza posługiwała się RR - Round Robin, druga - Starzenie, a trzecia - SJF (Shortest Job First). Dodatkowo w ramach testowania stworzonego planisty należało utworzyć cztery testy.

## Ogólne omówienie zmian

Większość zmian wprowadzono w pliku _proc.c_ znajdującego się w _usr/src/kernel_, gdzie bazowo jest logika planisty MINIX-a. Tam dodałem swoje własne funkcje, które obsługiwały trzy nowe grupy. W niektórych przypadkach modyfikowałem oryginalne funkcje, gdyż nie było sensu tworzyć osobnych funkcji dla nowych grup.

W _proc.h_ definiowałem nowe stałe oraz pola w strukturze _proc_ użyte w obsłudze priorytetów oraz implementacji algorytmów szeregowania.

Dodane pola w strukturze _proc_:
+ __sched_group__ - numer danej grupy priorytetowej; 1, 2 lub 3
+ __cpu_estimate__ - przewidywany czas pracy CPU dla procesów z grupy 3
+ __wait_ticks__ - licznik czasu oczekiwania dla procesów z grupy 2
+ __rr_counter__ - licznik czasu działania procesów z grupy 1

W pliku _system.c_ zostało zmodyfikowane wywołanie __do_fork()__, aby mogło inicjalizować nowe procesy z domyślnymi wartościami dla nowo dodanych pól w strukturze _proc_. Jeszcze tutaj zostało dodane nowe wywołanie do mikrojądra - __do_setgroup()__, lecz będzie ono omówione później.

Ostatnim plikiem, gdzie zaszły znaczące zmiany dotyczące planisty jest plik _clock.c_, gdzie w funkcji __clock_handler()__ dodałem liczenie starzenie i kwantów dla grup 2 i 1 poprzez funkcję __handle_clock_tick()__ znajdującą się w _proc.c_.


## Omówienie zmian - _proc.c_

### pick_proc():

Funkcja odpowiadająca za wybór procesu, któremu udostępnia się czas i zasoby systemu. W przypadku drugiej grupy jest tu zaimplementowany wybór procesu o największej wartości pola wait_ticks. Zachowałem bazowe grupy MINIX-a, warunki wyboru procesów z moich kolejek umieściłem między obsługą grupy _SERVER_, a _USER_.

Kolejność wyboru procesów: __grupa 1__ > __grupa 2__ > __grupa 3__.

### ready_with_group(rp):

Nowa funkcja mająca na celu ustawienie procesów oznaczonych jako _ready_ (patrz, proces pod wskaźnikiem __rp__) w odpowiadające im kolejki priorytetowe. W przypadku grupy 1 i 2 sprawa jest prosta: jeśli kolejka jest pusta, to ustaw proces na początku, jeśli nie to ustaw proces na końcu.

Inny sposób wstawiania jest dla grupy 3. W związku z implementacją algorytmu SJF, procesy są stawiane w kolejkę tak, aby zawsze była posortowana rosnąco względem pola __cpu_estimate__: 'najszybszy' proces na początku kolejki. To gwarantuje spełnienie założeń SJF.

### ready(rp):

Dodanie wywołania __ready_with_group(rp)__ dla procesów z grupy _USER_, które maja przypisaną wartość _int_ z zakresu 1-3 w polu __sched_group__. Bazowe dodanie do innych kolejek pozostało bez zmian. Wywołanie jest umieszczone znów pomiędzy obsługą procesu z grupy _SERVER_, a _USER_.

### unready_from_group(rp, head_ptr, tail_ptr):

Funkcja mająca usuwać z odpowiedniej kolejki ten proces (__rp__), który już nie może się dalej wykonywać lub jest zablokowany (np. czeka na dane z IO). Funkcja jest uniwersalna, gdyż przymuje wskaźniki do początku i końca danej kolejki priorytetowej grupy, z której jest usuwany proces (patrz, wskaźniki __head_ptr__ i __tail_ptr__).

### unready(rp):

Dodanie wywołanie __unready_from_group(rp, head_ptr, tail_ptr)__ dla procesów z grupy _USER_, które maja przypisaną wartość _int_ z zakresu 1-3 w polu __sched_group__. Bazowe usuwanie procesów z kolejek pozostało bez zmian. Wywołanie jest umieszczone znów pomiędzy obsługą procesu z grupy _SERVER_, a _USER_.

### update_aging_for_queue2():

Co wywołanie zwiększa wartość pola __wait_ticks__ dla procesów z grupy 2, które oczekują na przydzielenie w kolejce priorytetowej albo są zablokowane. Każdy proces ma limit wartości __wait_ticks__ wynoszący 1000. Funkcja ta jest wywoływana co każdy tick w __handle_clock_tick()__.

### handle_clock_tick():

Funkcja obsługująca zliczanie czasu i zapisywanie go w polach __wait_ticks__ oraz __rr_counter__. Jest ona wywoływana co każdy tick w __clock_handle()__. Dla zliczania __wait_ticks__ jest wywoływana funkcja __update_aging_for_queue2()__. Za to __rr_counter__ jest zwiększany tylko dla aktywnych procesów z grupy 1, albowiem ma on reprezentować pozostały przydzielony czas w ramach jednego kwantu czasu. Jeśli kwant się wyczerpie, zostaje wywoływana funkcja __lock_sched()__, która obsłuży przełączenie procesów w grupie 1 w ramach algorytmu RR.

### sched():

Funkcja ta została zmodyfikowana w taki sposób, aby obsługiwała przerzucanie aktywnego procesu z grupy 1 na tył kolejki priorytetowej w ramach implementacji RR - skończenie kwantu czasu. Bazowa zawartość funkcji jest obsługiwana później.


## Omówienie zmian - _system.c_

### do_fork(m_ptr):

Nie ma tu dużych zmian w porównaniu do bazowej wersji. Dodano wartości domyślne dla nowych pól ze struktury _proc_.

### do_setgroup(m_ptr):

Wywołanie do mikrojądra mające na celu ustawienie wartości nowo dodanych pól w strukturze _proc_. Używane do testów planisty. Wywoływane przez wywołanie systemowe _SETGRUP_, które jest obsługiwane w module MM.


## Testowanie planisty

W ramach sprawdzenia poprawności implementacji planisty, napisałem cztery testy, które znajdują się w pliku __test_sched.c__ znajdującego się w __usr/test_scripts__.

__Nazwy testów i cel testów:__
+ __test_hierarchy()__ - sprawdzenie czy priorytety między grupami są zachowane.
+ __test_round_robin()__ - sprawdzenie działania algorytmu szregowania dla grupy 1 (RR).
+ __test_aging()__ - sprawdzenie działania algorytmu szregowania dla grupy 2 (Aging).
+ __test_sjf()__ - sprawdzenie działania algorytmu szregowania dla grupy 2 (Aging).

Zanim omówię konkretne testy z tej listy, warto przedstawić parę kluczowych funkcji użytych w testach. Zaczynając od funkcji __setgroup(group, name)__, jest to już wcześniej wymienione wywołanie do mikrojądra, które ustawia pola w strukturze _proc_ i tym sposobem przydziela do danej grupy priorytetowej.

Następna funkcja, krótka lecz ważna, to __start_handler(sig)__ - odpowiada za synchroniczne uruchomienie wykonywania poszczególnych procesów. Kluczowa funckja, która pozwala na zaobserwowanie działania planisty. Działa na zasadzie wysyłanych sygnałów do procesów (moduł <signal.h>).

Ostatnia ważna funkcja - __child_process(group, estimate, name)__ - odpowiada za symulowanie pracy procesów z poszczególnych grup. Każda grupa ma trochę inną symulację zależną od natury danej grupy. Czasami zajmują sporo czasu procesora, aby następnie zasymulować zablokowanie poprzez _sleep()_.

__ADNOTACJA__ - przez to, że nie da się w tym samym czasie dodać wszystkich procesów (nawet z synchronizacją ich dodania) mogą wystąpić sytuację, które można łatwo uznać za błędne. Wynika to z faktu, że planista jak tylko zobaczy, że jest dostępny proces do aktywacji to go aktywuje zanim pojawią się następne. Nie jest to złe to zjawisko, ale trzeba je mieć z tyłu głowy w czasie pisania i analizy testów.

### test_hierarchy()

Ma na celu sprawdzenie czy procesy będą się wykonywały zgodnie z priorytetami, czyli procesy z grupy 1 wykonają się przed procesami z grupy 2, a te z grupy 2 - przed procesami z grupy 3.

Są zasymulowane po dwa procesy z każdej grupy, tak aby można było zauważyć działanie szeregowania dla każdej z grup. W teorii powinny wykonać się najpierw procesy z grupy 1, potem z grupy 2, a następnie z grupy 3. Jednakże w związku z symulacją blokad, uwzględnieniem kwantów dla grupy 1 oraz z różnym funkcjonowaniem samego systemu wygląda to trochę inaczej.

Na samym starcie rozpoczynają się wykonywać procesy z grupy 1 zgodnie z RR, jednakże w momencie kiedy oba procesy będą zablokowane (patrz symulacja czekania na dane z IO) kolejka grupy 1 staje się pusta, więc procesy z grupy 2 mogą się wykonać.

Wykonanie procesów z grupy 2, które też poddają się blokadzie, które jest dość szybkie powoduje, że zasymulowana procesy z grupy 1 nadal nie są gotowe do działania. Co za tym idzie procesy z grupy mogą się wykonać.

I w tym momencie występuje prawdziwy test tego, czy planista dobrze zadziała. Pierwszy proces z kolejki grupy 3 jest na tyle szybki (niski estimate), że ma prawo wykonać się do końca bez przerwań, takie jest założenie. Za to dłuższe procesy mają też zasymulowane obsługę IO, więc kiedy drugi proces, już dłuższy w wykonaniu, zostaje zablokowany, następuje przełączenie i z powrotem wracamy do procesów z grupy 1, gdyż już zdążyły powrócić do stanu _ready_.

Dalsza część symulacji odbywa się podobnie, procesy z priorytetem 1 wykonują się naprzemiennie do momentu zablokowania, następnie wykonują się procesy z priorytetem 2, a gdy one się zablokują i procesy z priorytetem 1 nadal nie są gotowe, ostatni proces z priorytetem 3 dostaje czas na działanie, aż zostanie zablokowany.

Na pierwszy rzut oka może wydawać się, że procesy nie działają zgodnie z założeniami planisty, lecz jest to mylne wrażenie. Mała liczba procesów i blokady tych zmiennych, które trochę trwają, wpływają na kolejność wykonywania zadań, ale te przełączenia kontekstów między danymi grupami priorytetowymi potwierdzają poprawność działania planisty.

### test_round_robin()

W tym teście utworzono trzy procesy z priorytetem 1, które powinny się wykonywać naprzemiennie (A >> B >> C i tak w kółko aż do zakończenia działania). Czasami wykonanie jest idealne i procesy prawdziwie wykonują się po kolei. Jednakże czasami program  zachowuje się inaczej po początkowym działaniu procesu A, gdyż potem przez chwilę działają naprzeminnie procesy B i C. Potem wszystko wraca do normy.

To nie jest błędne zachowanie. Wynika to z faktu, iż blokady mogą trochę trwać, stąd proces A mógł przez jakiś czas się nie wykonywać. Dodatkowo w związku z wprowadzniem kwantów czasu procesy mogą być zablokowane zanim uruchomią _sleep(1)_, gdyż nie zdążyły wykonać obliczeń. Dlatego czasami działa to jak człowiek zakłada: A >> B >> C >> A >> B >> C >> A >> itd. a czasami tak: A >> B >> C >> B >> C >> B >> A >> itd. (wraca do normy).

### test_aging()

W tym teście zasymulowano trzy procesy o priorytecie 2, lecz dwa pierwsze z nich są utworzone wcześniej od ostatniego. Dodatkowo proces A i C są procesmai IO, czyli cały czas wywołują _sleep(1)_, a proces B jest mieszanką - liczy, a potem inicjuje _sleep(1)_. Jest to zrobione po to, aby mogło zajść proces starzenia.

Przewidywalnie, procesy są aktywowane w odpowiedniej kolejności. Najpierw wykonywanie procesów wygląda tak: A >> B >> A. Następnie, kiedy starzejący się proces C w końcu zostanie dodany do kolejki, nastąpi zmiana wykonywania zadań: C >> A/B >> B/A >> C >> A/B >> itd. (A/B w zależności, w którym momencie zostanie dodany proces C do kolejki).

### test_sjf()

W tym teście zasymulowano cztery procesy o priorytecie 3 o następujących wartościach __cpu_estimate__: 1, 10, 2, 5. Wykonają się zgodnie z algorytmem SJF - proces o najmniejszym czasie wykonania się jako pierwszy, czyli kolejność wykonania wygląda tak: 1 >> 2 >> 5 >> 10.

Warto nadmienić, że dla procesów o __cpu_estimate__ większym od 5 występuje symulacja blokowań, więc jeśli byłoby więcej procesów o takim parametrze to kolejność byłaby trochę inna, na przykład: 8 >> 10 >> 8 >> 10 >> itd.


# Podsumowanie

Planista został odpowiednio zaimplementowany i przetestowany. Największe problemy wystąpiły przy pisaniu testów, gdyż trzeba było dobrze się zastanowić, czy przypadkiem one dają dobre wyniki.