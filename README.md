# Wyvern

Wyvern is the high-performance compression program for GameBoy.

## Usage

**Build**

Requirements

- Go 1.15
- make

```sh
$ gh repo clone pokemium/wyvern && cd ./wyvern
$ make build
$ ./wyvern -h # Help option
```

**Compression**

Currently, you cannot use multiple input files option 

```sh
$ wyvern ./test/cenotaph.atr # Input: ./test/cenotaph.atr, Output: Stdout
$ wyvern ./test/cenotaph.chr ./test/cenotaph.chr.wyv # Input: ./test/cenotaph.atr, Output: ./test/cenotaph.chr.wyv
```

**Decompression on Cmd**

```sh
$ wyvern -d ./test/cenotaph.atr.wyv # Input: ./test/cenotaph.atr.wyv, Output: Stdout
$ wyvern -d ./test/cenotaph.chr.wyv ./test/cenotaph.chr # Input: ./test/cenotaph.chr.wyv, Output: ./test/cenotaph.chr
```

**Decompression on GameBoy**

Please use `decompress` function in `asm/decompress.asm`.

If you want to try decompression, try `asm/hello.gb` is built when you execute `cd asm && make build`.

## Benchmark

Decompression cycle benchmark uses `decompress` function in `asm/decompress.asm`.

If you want to know more, jump to [Benchmark document](./benchmark.md).

```sh
$ ./wyvern ./test/cenotaph.atr
# Compression: 360 Bytes => 196 Bytes (54%)
# Decompression: 20996 Cycles

$ ./wyvern ./test/cenotaph.chr
# Compression: 4000 Bytes => 3397 Bytes (84%)
# Decompression: 190972 Cycles

$ ./wyvern ./test/cenotaph.map
# Compression: 360 Bytes => 336 Bytes (93%)
# Decompression: 14984 Cycles
```

## References

- [GBTD_GBMB](https://github.com/untoxa/GBTD_GBMB)
- [Game Boy Compression Playground](https://gitendo.github.io/gbcp/)
- [gitendo/helloworld](https://github.com/gitendo/helloworld)
- [pret/pokered](https://github.com/pret/pokered)
- [pret/pokecrystal](https://github.com/pret/pokecrystal)
