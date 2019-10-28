# cc7

https://www.sigbus.info/compilerbook をやって C 言語を勉強しています。

## Development

### Compile

```
docker-compose run --rm base make
```

### Format

Code formatting by clang-format.

```
docker-compose run --rm base make format
```

### Test

```
docker-compose run --rm base make test
```
