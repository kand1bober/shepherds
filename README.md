# Shepherds on narrow bridge

условие задачи:
```C
Пастухи, для перевода стада через узкий мост, на котором невозможно разойтись двум стадам, используют следующий алгоритм:

Если пастух перед входом на мост видит стада, ожидающие перехода или движущиеся в попутном направлении, то он пристраивается в хвост колонны. Если пастух гонит стадо в одном направлении и перед входом никого нет, он останавливает стадо, в одиночку проходит мост, оставляет на его противоположном конце свою шапку и возвращается к стаду, проводит его через мост, забирает шапку и следует дальше.

Если пастух гонит стадо в другом направлении, и перед входом нет шапки — он смело входит в опасный участок.

При наличии шапки он ждет, пока встречные стада не пройдут полностью. Постройте корректную модель прохода стад через мост, описав поведение каждого пастуха с помощью монитора Хоара.
```


### build
```C
gcc shepherd.c fifo/fifo.c -o <name>
```

### пример различия в результатах в зависимоси от временной дельты между появлениями пастухов

## 1)

delta:
```C
int time_delta = 5; //in usec
```

result:
```C
#0: came
#0: crossing (=>)
#1: came
#0: put down my hat
#0: crossing (<=)
#0 + herd: crossing (=>)
#2: came
#0 + herd: finished
#0: took my hat
#3: came
#1 + herd: crossing (=>)
#4: came
#1 + herd: finished
#2 + herd: crossing (=>)
#5: came
#2 + herd: finished
#6: came
#3 + herd: crossing (=>)
#7: came
#3 + herd: finished
#6 + herd: crossing (<=)
#8: came
#6 + herd: finished
#5 + herd: crossing (<=)
#9: came
#5 + herd: finished
#4 + herd: crossing (<=)
#4 + herd: finished
#7: crossing (=>)
#7: put down my hat
#7: crossing (<=)
#7 + herd: crossing (=>)
#7 + herd: finished
#7: took my hat
#8 + herd: crossing (=>)
#8 + herd: finished
#9 + herd: crossing (=>)
#9 + herd: finished

```

## 2)

delta:
```C
int time_delta = 50; //in usec
```

result:
```C
#0: came
#0: crossing (=>)
#0: put down my hat
#0: crossing (<=)
#1: came
#0 + herd: crossing (=>)
#2: came
#0 + herd: finished
#0: took my hat
#3: came
#1 + herd: crossing (=>)
#1 + herd: finished
#4: came
#3 + herd: crossing (=>)
#5: came
#3 + herd: finished
#2 + herd: crossing (=>)
#6: came
#2 + herd: finished
#7: came
#7 + herd: crossing (=>)
#7 + herd: finished
#8: came
#8 + herd: crossing (=>)
#8 + herd: finished
#9: came
#9 + herd: crossing (=>)
#9 + herd: finished
#6 + herd: crossing (<=)
#6 + herd: finished
#4 + herd: crossing (<=)
#4 + herd: finished
#5 + herd: crossing (<=)
#5 + herd: finished

```