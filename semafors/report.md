# SOI Zadanie 3 - semafory
# Raport autorstwa Kacpra Skrodzkiego

## Wstęp

W ramach trzeciego zadania należało utworzyć czterech producentów i trzech konsumentów korzystających z czterech buforów, których pracę należy zsynchronizować przy pomocy semaforów.

## Omówienie rozwiązania

Aby móc wprowadzić synchronizację do dostępu do buforów należało wprowadzić po trzy semafory _(implementacja semafora z biblioteki <semaphore.h>)_ dla każdego bufora - _mutex_, _full_ i _empty_. Pierwszy z nich, _mutex_, pozwalał na dokonywanie operacji dodania lub usunięcia z bufora. Inicjalizowany z wartością jeden, gwarantował poprawność obsługi sekcji krytycznej.

Semafory _full_ i _empty_ przetrzymują informację o tym, ile jest zajętych oraz pustych miejsc w buforze. To one determinują czy producent może w ogóle coś dodać oraz czy też konsument coś pobrać z bufora. Dzięki tym semaforom jest kontrolowany stan danego bufora, zabezpiecza przed przepełnieniem albo pobieraniem danych, które w ogóle nie istnieją.

## Testy implementacji - symulacja

Aby przetestować poprawność implementacji semaforów stworzyłem program, który symuluje pracę wszystkich producentów i konsumentów przez okres 10 sekund. Wyniki zostały zapisane w plikach __result_1.txt__ i __result_2.txt__. Każda z symulacji została przeprowadzona przy innych rozmiarach buforów (30 oraz 3 elementy). Dodatkowo, wątki konsumentów były jako pierwsze zainicjowane, aby w każdej symulacji przetestować, czy konsumenci nie biorą danych z pustych buforów. Do utworzenia osobnych wątków wykorzystałem bibliotekę <pthread.h>.

### Pierwsza symulacja

W ramach pierwszej symulacji sprawdziłem, czy synchronizacja przebiega pomyślnie. Warunkami tego, aby synchronizacja była poprawna są:

* Dane składniki na pierogi będą występowały tylko raz, szczególnie ciasta
* Produkty z buforów będą brane zgodnie z założeniami FIFO (nr 0 -> nr 1 -> nr 2 itd.)
* Żaden konsument nię będzie zagłodzony -> każdy będzie miał szansę na dostanie współdzielonego składnika _(ciasto)_
* Bufory nie są przepełnione, ani składniki nie są brane z nikąd

W przypadku pierwszej symulacji można zauważyć, że prawie wszystkie warunki są spełnione. Składniki nie są dublowane, czyli nie było dwóch konsumentów naraz w sekcji krytycznej - można to ocenić śledząc produkcję i zużycie _ciasta_.

Każdy ze składników jest wybierany zgdonie z FIFO -> następne pierogi są robione ze starszymi produktami, np.: _Making pierog with M; (dough id: 8, filling id 3)_, gdzie w buforze są mięsa o id: 4, 5, 6, 7, 8.

Dodatkowo nie występuje zagłodzenie. Pierogi robią się prawie naprzemiennie, np.: M -> C -> S -> M. Czasami są lekkie zaburzenia, ale one wynikają z samej natury działania wątków i procesów _(blokady, opóźnienia, sprzęt, planista, itp.)_.

Niestety nie dało się zauważyć tego, czy pojemność buforów została zachowana, gdyż symulacja trwała za krótko, aby można było wypełnić bufory po brzegi. Dlatego druga symulacja skupia się konkretnie na tym, czy jest zachowana pojemność buforów i czy sa zachowane blokady dla producentów.

### Druga symulacja

Druga symulacja miała bufory o niskiej pojemności - tylko 3 elementowe bufory. Miało to na celu ocenienie, czy ochrona buforów przed przepełnieniem działa.

Na samym początku symulacji można zauważyć identyczne zachowanie jak w pierwszej symulacji. Nie ma duplikatów, struktura FIFO jest zachowana oraz nie ma zagłodzenia. Jednakże na początku symulacji bufory jeszcze w pełni się nie zapełniły. Na szczęście w dalszej części można zauważyć, że bufory się napełniły, gdzyż ilość komunikatów o dodaniu składnika do bufora zmalała, a ilość komunikatów o dodaniu i konsumpcji się zgadza. Dodatkowo można zauważyć blokady producentów - komunikat o włożeniu składnika do buforu jest o wiele później niż komunikat o wyprodukowaniu danego składnika.

# Podsumowanie

Implementacja semaforów dla problemu producentów i konsumentów jest poprawna i zapewnia bezpieczną synchronizację ze spełnionymi założeniami sekcji krytycznej.