# Shepherds on narrow bridge

### example of different results in dependence of defined moving "velocity" of shepherds

## 1)

velocity:
```C
#define kTimeToCrossAlone 10 //usec
#define kTimeToCrossWithHerd 30 //usec
```

result:
```C
shepherd #0: came
shepherd #0: crossing (=>)
shepherd #1: came
shepherd #0: left hat on rigt side
shepherd #0: crossing (<=)
shepherd + herd #0: crossing (=>)
shepherd #2: came
shepherd + herd #0: finished crossing
shepherd + herd #1: crossing (=>)
shepherd #3: came
shepherd + herd #1: finished crossing
shepherd + herd #2: crossing (=>)
shepherd + herd #2: finished crossing
shepherd + herd #3: crossing (=>)
shepherd #4: came
shepherd + herd #3: finished crossing
shepherd #0: took my hat
shepherd + herd #4: crossing (<=)
shepherd #5: came
shepherd + herd #4: finished crossing
shepherd + herd #5: crossing (<=)
shepherd #6: came
shepherd + herd #5: finished crossing
shepherd + herd #6: crossing (<=)
shepherd #7: came
shepherd + herd #6: finished crossing
shepherd #7: crossing (=>)
shepherd #7: left hat on rigt side
shepherd #7: crossing (<=)
shepherd #8: came
shepherd + herd #7: crossing (=>)
shepherd #9: came
shepherd + herd #7: finished crossing
shepherd + herd #8: crossing (=>)
shepherd + herd #8: finished crossing
shepherd + herd #9: crossing (=>)
shepherd + herd #9: finished crossing
shepherd #7: took my hat

```

## 2)

velocity:
```C
#define kTimeToCrossAlone 10000 //usec
#define kTimeToCrossWithHerd 30000 //usec
```

result:
```C
shepherd #0: came
shepherd #0: crossing (=>)
shepherd #1: came
shepherd #2: came
shepherd #3: came
shepherd #4: came
shepherd #5: came
shepherd #6: came
shepherd #7: came
shepherd #8: came
shepherd #9: came
shepherd #0: left hat on rigt side
shepherd #0: crossing (<=)
shepherd + herd #0: crossing (=>)
shepherd + herd #0: finished crossing
shepherd + herd #1: crossing (=>)
shepherd + herd #1: finished crossing
shepherd + herd #2: crossing (=>)
shepherd + herd #2: finished crossing
shepherd + herd #3: crossing (=>)
shepherd + herd #3: finished crossing
shepherd + herd #7: crossing (=>)
shepherd + herd #7: finished crossing
shepherd + herd #8: crossing (=>)
shepherd + herd #8: finished crossing
shepherd + herd #9: crossing (=>)
shepherd + herd #9: finished crossing
shepherd #0: took my hat
shepherd + herd #4: crossing (<=)
shepherd + herd #4: finished crossing
shepherd + herd #5: crossing (<=)
shepherd + herd #5: finished crossing
shepherd + herd #6: crossing (<=)
shepherd + herd #6: finished crossing

```