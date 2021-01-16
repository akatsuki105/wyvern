package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"strings"
)

type byteStream []byte

const (
	title   = "c64pack"
	exeName = "c64pack"
)

// exit code
const (
	exitCodeOK int = iota
	exitCodeError
)

var (
	srcIndex = 0
)

func init() {
	flag.Usage = func() {
		usage := fmt.Sprintf(`Usage:
    %s [arg] [input] [output]
input: a filename

output: a filename
  with no FILE, write standard output

Arguments: 
`, exeName)

		fmt.Fprintf(os.Stderr, usage)
		flag.PrintDefaults()
	}
}

func main() {
	os.Exit(Run())
}

// Run program
func Run() int {
	var (
		doDecompression = flag.Bool("d", false, "decompression")
	)
	flag.Parse()

	args := flag.Args()
	if len(args) == 0 {
		fmt.Fprintf(os.Stderr, "parse error: at least input must be specified\n")
		return exitCodeError
	}

	input := flag.Arg(0)

	src, err := ioutil.ReadFile(input)
	if err != nil {
		fmt.Fprintf(os.Stderr, "file error: fail to load file\n")
		return exitCodeError
	}

	if *doDecompression {
		result := decompress(src)
		if len(args) > 1 {
			output := flag.Arg(1)
			err := ioutil.WriteFile(output, result, os.FileMode(os.ModePerm))
			if err != nil {
				fmt.Fprintf(os.Stderr, "file error: fail to write file\n")
				return exitCodeError
			}
		} else {
			fmt.Printf("Result: %s\n\n", byteStream(result))
		}
		fmt.Printf("Decompression: %d Bytes => %d Bytes (%d%%)\n", len(src), len(result), 100*len(result)/len(src))
		return exitCodeOK
	}

	// compression
	result := compress(src)
	if len(args) > 1 {
		output := flag.Arg(1)
		err := ioutil.WriteFile(output, result, os.FileMode(os.ModePerm))
		if err != nil {
			fmt.Fprintf(os.Stderr, "file error: fail to write file\n")
			return exitCodeError
		}
	} else {
		fmt.Printf("Result: %s\n\n", byteStream(result))
	}

	fmt.Printf("Compression: %d Bytes => %d Bytes (%d%%)\n", len(src), len(result), 100*len(result)/len(src))
	return exitCodeOK
}

func compress(src []byte) []byte {
	return []byte{}
}

func decompress(src []byte) []byte {
	decompressed := []byte{}

	// srcIndex := 1
	// esc := src[srcIndex]

	// srcIndex++
	// escMask := src[srcIndex]
	// nescMask := ^escMask

	// regy := 0
	// for {
	// 	b := src[srcIndex]
	// 	srcIndex++

	// 	if (b & escMask) != esc {
	// 		// .normal
	// 	} else {
	// 		regy = 0

	// 		tmp1 := b & nescMask
	// 		tmp2 := tmp1 >> 1 // srl
	// 		if tmp1%2 == 1 {
	// 			// .lz
	// 		} else {

	// 		}
	// 	}
	// }

	return decompressed
}

func (bs byteStream) String() string {
	builder := &strings.Builder{}
	builder.WriteString("[")
	for i, b := range bs {
		builder.WriteString(fmt.Sprintf("%d", b))
		if i < len(bs)-1 {
			builder.WriteString(", ")
		}
	}
	builder.WriteString("]")
	return builder.String()
}

func bit(b byte, n int) bool {
	return b&(1<<n) != 0
}

func extendSlice(src []byte) []byte {
	result := make([]byte, len(src)*2)
	for i, b := range src {
		result[i] = b
	}
	return result
}
