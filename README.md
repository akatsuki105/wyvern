# GB Compress

This program is re-implementation for "GB Compress" in GBTD using Go.

## Usage

**Build**

```sh
$ gh repo clone pokemium/gbcompress && cd ./gbcompress
$ make build
```

**Compression**

```sh
$ gbcompress input [output]                        
```

If you don't specify output filename, use Stdin. 

**Decompression on Cmd**

```sh
$ gbcompress -d input [output]                        
```

If you don't specify output filename, use Stdin. 

**Decompression on GameBoy**

Please use the function `gb_decompress` in `asm/decompress.asm`.

If you try decompression, try `asm/hello.gb` is built when you execute `make build`.

## Benchmark

```sh
$ ./gbcompress ./test/cenotaph.atr
# Compression: 360 Bytes => 199 Bytes (55%)

$ ./gbcompress ./test/cenotaph.chr
# Compression: 4000 Bytes => 3417 Bytes (85%)

$ ./gbcompress ./test/cenotaph.map
# Compression: 360 Bytes => 338 Bytes (93%)
```

## References

- [GBTD_GBMB](https://github.com/untoxa/GBTD_GBMB)
- [Game Boy Compression Playground](https://gitendo.github.io/gbcp/)
- [gitendo/helloworld](https://github.com/gitendo/helloworld)
