# Wyvern

This program is compression program for GameBoy.

## Usage

**Build**

```sh
$ gh repo clone pokemium/wyvern && cd ./wyvern
$ make build
```

**Compression**

```sh
$ wyvern input [output]                        
```

If you don't specify output filename, use Stdin. 

**Decompression on Cmd**

```sh
$ wyvern -d input [output]                        
```

If you don't specify output filename, use Stdin. 

**Decompression on GameBoy**

Please use `decompress` function in `asm/decompress.asm`.

If you want to try decompression, try `asm/hello.gb` is built when you execute `make build`.

## Benchmark

```sh
$ ./wyvern ./test/cenotaph.atr
# Compression: 360 Bytes => 199 Bytes (55%)

$ ./wyvern ./test/cenotaph.chr
# Compression: 4000 Bytes => 3417 Bytes (85%)

$ ./wyvern ./test/cenotaph.map
# Compression: 360 Bytes => 338 Bytes (93%)
```

## References

- [GBTD_GBMB](https://github.com/untoxa/GBTD_GBMB)
- [Game Boy Compression Playground](https://gitendo.github.io/gbcp/)
- [gitendo/helloworld](https://github.com/gitendo/helloworld)
