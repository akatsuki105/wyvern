package main

import (
	"encoding/binary"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
)

var version string

const (
	title   = "GB Compress"
	exeName = "gbcompress"
)

// exit code
const (
	exitCodeOK int = iota
	exitCodeError
)

var (
	outBuf   = []byte{}
	maxSize  = 0
	outIndex = 0
)

func init() {
	if version == "" {
		version = "Develop"
	}

	flag.Usage = func() {
		usage := fmt.Sprintf(`Usage:
    %s [arg] [input] [output]
input: a filename
	with no FILE, or when FILE is - or stdin, read standard input
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
		doDecompression = flag.Bool("d", false, "decompress")
		showVersion     = flag.Bool("v", false, "show version")
	)
	flag.Parse()

	switch {
	case *showVersion:
		printVersion()
		return exitCodeOK
	default:
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
				fmt.Printf("Result: %+v\n\n", result)
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
			fmt.Printf("Result: %+v\n\n", result)
		}
		fmt.Printf("Compression: %d Bytes => %d Bytes (%d%%)\n", len(src), len(result), 100*len(result)/len(src))
		return exitCodeOK
	}
}

func printVersion() {
	fmt.Println(title+":", version)
}

func compress(src []byte) []byte {
	maxSize = len(src)
	outBuf = make([]byte, maxSize)

	var bufPtr, trashSize int
	for bufPtr < maxSize {
		// a,a,a,a,a,...
		x := src[bufPtr]
		r_rb := byte(1)
		for (bufPtr+1 < maxSize) && (bufPtr+int(r_rb) < maxSize) && (src[bufPtr+int(r_rb)] == x) && (r_rb < 64) {
			r_rb++
		}

		// a,b,a,b,a,b,a,b,...
		// a,a,a,a -> 2aa
		y := uint16(src[bufPtr]) << 8 // MEM: 0x01 -> 0x02 => Val: (0x01 << 8) | 0x02
		if bufPtr+1 < maxSize {
			y |= uint16(src[bufPtr+1])
		}
		r_rw := byte(1)
		for (bufPtr+int(r_rw)*2+1 < maxSize) && (binary.BigEndian.Uint16(src[bufPtr+int(r_rw)*2:]) == y) && (r_rw < 64) {
			r_rw++
		}

		rr, sr, r_rs := 0, 0, byte(0)
		for rr < bufPtr {
			// 01234567
			// abcdebcd
			//      ↑12
			// sr=-4 r_rs=3
			rl := 0
			for (rr+rl < bufPtr) && (bufPtr+rl < maxSize) && (src[rr+rl] == src[bufPtr+rl]) && (rl < 64) {
				rl++
			}

			if rl > int(r_rs) {
				sr = rr - bufPtr
				r_rs = byte(rl)
			}
			rr++
		}

		switch {
		case r_rb > 2 && r_rb > r_rw && r_rb > r_rs:
			if trashSize > 0 {
				writeTrash(byte(trashSize), src[bufPtr-trashSize:])
				trashSize = 0
			}
			writeByte(r_rb, x)
			bufPtr += int(r_rb)
		case (r_rw > 2) && (r_rw*2 > r_rs):
			if trashSize > 0 {
				writeTrash(byte(trashSize), src[bufPtr-trashSize:])
				trashSize = 0
			}
			writeWord(r_rw, y)
			bufPtr += (int(r_rw) * 2)
		default:
			switch {
			case r_rs > 3:
				if trashSize > 0 {
					writeTrash(byte(trashSize), src[bufPtr-trashSize:])
					trashSize = 0
				}
				writeString(r_rs, uint16(sr))
				bufPtr += int(r_rs)
			case trashSize >= 64:
				writeTrash(byte(trashSize), src[bufPtr-trashSize:])
				trashSize = 0
			default:
				trashSize++
				bufPtr++
			}
		}
	}

	if trashSize > 0 {
		writeTrash(byte(trashSize), src[bufPtr-trashSize:])
	}

	writeEnd()
	return outBuf
}

func decompress(src []byte) []byte {
	outBuf = []byte{}
	index := 0
	for index < len(src) {
		b := src[index]
		index++
		if b == 0 {
			break
		}

		switch {
		case b&0b1000_0000 > 0 && b&0b0100_0000 > 0: // if bit6 and bit7 are set, use trash function
			length := int(b&63 + 1)
			for i := 0; i < length; i++ {
				outBuf = append(outBuf, src[index])
				index++
			}
		case b&0b1000_0000 > 0: // if bit7 is set, use string function
			length, upper, lower := int(b&63+1), src[index+1], src[index]
			offset := int16(upper)<<8 | int16(lower)

			index += 2
			from := len(outBuf) + int(offset)
			for i := 0; i < length; i++ {
				outBuf = append(outBuf, outBuf[from+i])
			}
		case b&0b0100_0000 > 0: // if bit6 is set, use word function
			length, upper, lower := int(b&63+1), src[index], src[index+1]
			index += 2
			for i := 0; i < length; i++ {
				outBuf = append(outBuf, upper)
				outBuf = append(outBuf, lower)
			}
		default:
			// RLE Byte
			length, data := int((b&63)+1), src[index]
			index++
			for i := 0; i < length; i++ {
				outBuf = append(outBuf, data)
			}
		}
	}

	return outBuf
}

func writeByte(len, data byte) {
	if outIndex+2 >= maxSize {
		panic("OutBuf too small")
	}

	// aaaaaa -> 6,a
	len = (len - 1) % 64
	outBuf[outIndex] = len
	outBuf[outIndex+1] = data
	outIndex += 2
}

func writeWord(len byte, data uint16) {
	if outIndex+3 >= maxSize {
		panic("OutBuf too small")
	}

	// ababab -> 3ab
	len = ((len - 1) % 64) | 0b0100_0000
	outBuf[outIndex] = len
	outBuf[outIndex+1] = byte(data >> 8)
	outBuf[outIndex+2] = byte(data)
	outIndex += 3
}

func writeString(len byte, data uint16) {
	if outIndex+3 >= maxSize {
		panic("OutBuf too small")
	}

	// abcdebcd (len=3 data=-4)-> abcde
	i := ((len - 1) % 64) | 0b1000_0000
	outBuf[outIndex] = i
	outBuf[outIndex+1] = byte(data)
	outBuf[outIndex+2] = byte(data >> 8)
	outIndex += 3
}

func writeTrash(len byte, pos []byte) {
	if outIndex+int(len) > maxSize {
		panic("OutBuf too small")
	}

	c := ((len - 1) % 64) | 0b1100_0000
	outBuf[outIndex] = c
	outIndex++

	for i := 0; i < int(len); i++ {
		outBuf[outIndex] = pos[i]
		outIndex++
	}
}

func writeEnd() {
	if outIndex+1 >= maxSize {
		panic("OutBuf too small")
	}

	outBuf[outIndex] = 0
	outIndex++

	// trim extra slice
	from := outBuf
	outBuf = make([]byte, outIndex)
	for i := 0; i < outIndex; i++ {
		outBuf[i] = from[i]
	}
}
