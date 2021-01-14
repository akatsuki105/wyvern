# Wyvern

This program is compression program for GameBoy.

## Usage

**Build**

Requirements

- Go 1.15
- make

```sh
$ gh repo clone pokemium/wyvern && cd ./wyvern
$ make build
```

**Compression**

Currently, you cannot use multiple input files option 

```sh
$ wyvern ./test/cenotaph.atr # Input: ./test/cenotaph.atr, Output: Stdout
$ wyvern ./test/cenotaph.chr ./test/cenotaph.chr.wyv # Input: ./test/cenotaph.atr, Output: ./test/cenotaph.chr.wyv
```

**Decompression on Cmd**

```sh
$ wyvern -d input [output]  
$ wyvern -d ./test/cenotaph.atr.wyv # Input: ./test/cenotaph.atr.wyv, Output: Stdout
$ wyvern -d ./test/cenotaph.chr.wyv ./test/cenotaph.chr # Input: ./test/cenotaph.chr.wyv, Output: ./test/cenotaph.chr                   
```

**Decompression on GameBoy**

Please use `decompress` function in `asm/decompress.asm`.

If you want to try decompression, try `asm/hello.gb` is built when you execute `make build`.

## Benchmark

Decompression cycle benchmark uses `decompress` function in `asm/decompress.asm`.

If you want to know more, jump to [Benchmark document](./benchmark.md).

```sh
$ ./wyvern ./test/cenotaph.atr
# Compression: 360 Bytes => 199 Bytes (55%)
# Decompression: 11580 Cycles

$ ./wyvern ./test/cenotaph.chr
# Compression: 4000 Bytes => 3417 Bytes (85%)
# Decompression: 117440 Cycles

$ ./wyvern ./test/cenotaph.map
# Compression: 360 Bytes => 338 Bytes (93%)
# Decompression: 9544 Cycles
```

## References

- [GBTD_GBMB](https://github.com/untoxa/GBTD_GBMB)
- [Game Boy Compression Playground](https://gitendo.github.io/gbcp/)
- [gitendo/helloworld](https://github.com/gitendo/helloworld)
