# GB Compress

This program is re-implementation for "GB Compress" in GBTD using Go.

## Usage

**Build**

```sh
$ gh repo clone pokemium/gbcompress && cd ./gbcompress
$ go build .
```

**Compression**

```sh
$ gbcompress input [output]                        
```

If you don't specify output filename, use Stdin. 

**Decompression**

```sh
$ gbcompress -d input [output]                        
```

If you don't specify output filename, use Stdin. 

## References

- [GBTD_GBMB](https://github.com/untoxa/GBTD_GBMB)
- [Game Boy Compression Playground](https://gitendo.github.io/gbcp/)
- [gitendo/helloworld](https://github.com/gitendo/helloworld)
