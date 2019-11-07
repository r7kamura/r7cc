# r7cc

[![](https://github.com/r7kamura/r7cc/workflows/push/badge.svg)](https://github.com/r7kamura/r7cc/actions)

勉強用に開発している C コンパイラです。

## Development

Ubuntu 上でコンパイルするために、docker-compose で base という名前のサービスを用意しています。

### Compile

コンパイルすると、入力文字列を元にアセンブリを標準出力する r7cc という実行ファイルができます。

```
docker-compose run --rm base make
```

### Format

clang-format を使ってコードをフォーマットします。

```
docker-compose run --rm base make format
```

### Test

test.sh に記述した簡易的なテストコードを実行できます。

```
docker-compose run --rm base make test
```
