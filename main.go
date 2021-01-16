package main

import (
	"encoding/binary"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
)

type byteStream []byte

var version string

const (
	title   = "Wyvern"
	exeName = "wyvern"
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

var (
	byteCtr           = 0
	byteValues        = []byte{}
	byteLens          = []byte{}
	wordCtr           = 0
	wordLens          = []byte{}
	incByteCtr        = 0
	longStringCtr     = 0
	longStringLens    = []byte{}
	longStringOffsets = []int16{}
	shortStringCtr    = 0
	shortStringLens   = []byte{}
	rotateStringCtr   = 0
	trashCtr          = 0
	trashLens         = []byte{}
)

func init() {
	if version == "" {
		version = "Develop"
	}

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
		showVersion     = flag.Bool("V", false, "display Version number and exit")
		doDecompression = flag.Bool("d", false, "decompression")
		verbose         = flag.Bool("v", false, "verbose mode")
		benchmark       = flag.Bool("b", false, "benchmark mode(for debug)")
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
			} else if !*benchmark {
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
		} else if !*benchmark {
			fmt.Printf("Result: %s\n\n", byteStream(result))
		}

		if *verbose {
			fmt.Printf("Byte: %d, Word: %d, IncB: %d, LongS: %d, ShortS: %d, RotateS: %d, Trash: %d\n", byteCtr, wordCtr, incByteCtr, longStringCtr, shortStringCtr, rotateStringCtr, trashCtr)
		}

		if *benchmark {
			_, filename := filepath.Split(input)
			fmt.Printf("%s ", filename)
		}
		fmt.Printf("Compression: %d Bytes => %d Bytes (%d%%)\n\n", len(src), len(result), 100*len(result)/len(src))
		return exitCodeOK
	}
}

func printVersion() {
	fmt.Println(title+":", version)
}

func compress(src []byte) []byte {
	maxSize = len(src)
	outBuf = make([]byte, maxSize)
	outIndex = 0

	var bufPtr, trashSize int
	for bufPtr < maxSize {
		// a,a,a,a,a,...
		curByte := src[bufPtr]
		byteLen := byte(1)
		for (bufPtr+1 < maxSize) && (bufPtr+int(byteLen) < maxSize) && (src[bufPtr+int(byteLen)] == curByte) && (byteLen < 64) {
			byteLen++
		}

		// a,b,a,b,a,b,a,b,...
		curWord := uint16(src[bufPtr]) << 8 // MEM: 0x01 -> 0x02 => Val: (0x01 << 8) | 0x02
		if bufPtr+1 < maxSize {
			curWord |= uint16(src[bufPtr+1])
		}
		wordLen := byte(1)
		for (bufPtr+int(wordLen)*2+2 < maxSize) && (binary.BigEndian.Uint16(src[bufPtr+int(wordLen)*2:]) == curWord) && (wordLen < 32) {
			wordLen++
		}

		// 1,2,3,4,5,... -> (1, 5)
		// incrementalByteLen := byte(1)
		// for (bufPtr+1 < maxSize) && (bufPtr+int(incrementalByteLen) < maxSize) && (src[bufPtr+int(incrementalByteLen)] == curByte+incrementalByteLen) && (incrementalByteLen < 16) {
		// 	incrementalByteLen++
		// }

		// offset: 01234567
		// bytes:  abcdebcd
		// rr:          ↑
		// [sOff, sLen]: [-4, 3]
		rr := 0
		sOff, sLen := 0, byte(0)
		for rr < bufPtr {
			rl := 0
			for (rr+rl < bufPtr) && (bufPtr+rl < maxSize) && (src[rr+rl] == src[bufPtr+rl]) && (rl < 32) {
				rl++
			}

			if rl > int(sLen) {
				sOff = bufPtr - rr
				sLen = byte(rl)
			}
			rr++
		}

		// 0x10, 0x20, 0x40, 0x08, 0x04, 0x02 -> 0x10, 0x20, 0x40, (-3, 3)
		rotateSPtr := 0
		rotateSOff, rotateSLen := 0, byte(0)
		for rotateSPtr < bufPtr {
			rl := 0
			for (rotateSPtr+rl < bufPtr) && (bufPtr+rl < maxSize) && (rotate(src[rotateSPtr+rl]) == src[bufPtr+rl]) && (rl < 16) {
				rl++
			}

			if rl > int(rotateSLen) {
				rotateSOff = bufPtr - rotateSPtr
				rotateSLen = byte(rl)
			}
			rotateSPtr++
		}

		switch {
		case byteLen > 2 && byteLen > wordLen && byteLen > sLen:
			if trashSize > 0 {
				writeTrash(byte(trashSize), src[bufPtr-trashSize:])
				trashSize = 0
			}
			writeByte(byteLen, curByte)
			bufPtr += int(byteLen)
		// case (incrementalByteLen > 2) && incrementalByteLen > wordLen && incrementalByteLen > sLen:
		// 	if trashSize > 0 {
		// 		writeTrash(byte(trashSize), src[bufPtr-trashSize:])
		// 		trashSize = 0
		// 	}
		// 	writeIncrementalByte(incrementalByteLen, curByte)
		// 	bufPtr += int(incrementalByteLen)
		case (wordLen > 2) && (wordLen*2 > sLen):
			if trashSize > 0 {
				writeTrash(byte(trashSize), src[bufPtr-trashSize:])
				trashSize = 0
			}
			writeWord(wordLen, curWord)
			bufPtr += (int(wordLen) * 2)
		default:
			switch {
			case sLen > 3:
				if trashSize > 0 {
					writeTrash(byte(trashSize), src[bufPtr-trashSize:])
					trashSize = 0
				}
				if sOff > 0xff {
					writeLongString(sLen, uint16(sOff))
				} else {
					writeShortString(sLen, byte(sOff))
				}
				bufPtr += int(sLen)
			case rotateSLen > 3:
				if trashSize > 0 {
					writeTrash(byte(trashSize), src[bufPtr-trashSize:])
					trashSize = 0
				}
				writeRotateString(rotateSLen, uint16(rotateSOff))
				bufPtr += int(rotateSLen)
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
		case bit(b, 7) && bit(b, 6): // if bit6 and bit7 are set, use trash function
			length := int(b&0b0011_1111 + 1)
			for i := 0; i < length; i++ {
				outBuf = append(outBuf, src[index])
				index++
			}
		case bit(b, 7) && bit(b, 5): // if (bit7, bit6, bit5) is (1, 0, 1), use long-string function
			length, upper, lower := int(b&0b0001_1111+1), src[index+1], src[index]
			offset := int16(upper)<<8 | int16(lower)

			index += 2
			from := len(outBuf) + int(offset)
			for i := 0; i < length; i++ {
				outBuf = append(outBuf, outBuf[from+i])
			}
		case bit(b, 7): // if (bit7, bit6, bit5) is (1, 0, 0), use short-string function
			length, offset := int(b&0b0001_1111+1), src[index]
			index++
			from := len(outBuf) - int(offset)
			for i := 0; i < length; i++ {
				outBuf = append(outBuf, outBuf[from+i])
			}
		case bit(b, 6) && !bit(b, 5): // if bit6 is set, use word function
			length, upper, lower := int(b&0b0001_1111+1), src[index], src[index+1]
			index += 2
			for i := 0; i < length; i++ {
				outBuf = append(outBuf, upper)
				outBuf = append(outBuf, lower)
			}
		// case bit(b, 6) && bit(b, 5) && bit(b, 4): // incremental byte
		// 	length, data := int(b&0b0000_1111+1), src[index]
		// 	index++
		// 	for i := 0; i < length; i++ {
		// 		outBuf = append(outBuf, data+byte(i))
		// 	}
		case bit(b, 6) && bit(b, 5): // rotate string
			length, upper, lower := int(b&0b0000_1111+1), src[index+1], src[index]
			offset := int16(upper)<<8 | int16(lower)

			index += 2
			from := len(outBuf) + int(offset)
			for i := 0; i < length; i++ {
				outBuf = append(outBuf, rotate(outBuf[from+i]))
			}
		default:
			// RLE Byte
			length, data := int((b&0b0011_1111)+1), src[index]
			index++
			for i := 0; i < length; i++ {
				outBuf = append(outBuf, data)
			}
		}
	}

	return outBuf
}

func writeByte(length, data byte) {
	if outIndex+2 >= len(outBuf) {
		outBuf = extendSlice(outBuf)
		maxSize = len(outBuf)
	}
	byteCtr++
	byteValues = append(byteValues, data)
	byteLens = append(byteLens, length)

	// aaaaaa -> 6,a
	length = (length - 1) % 64
	outBuf[outIndex] = length
	outBuf[outIndex+1] = data
	outIndex += 2
}

// func writeIncrementalByte(length, data byte) {
// 	if outIndex+2 >= len(outBuf) {
// 		outBuf = extendSlice(outBuf)
// 		maxSize = len(outBuf)
// 	}
// 	incByteCtr++

// 	length = ((length - 1) % 16) | 0b0111_0000
// 	outBuf[outIndex] = length
// 	outBuf[outIndex+1] = data
// 	outIndex += 2
// }

func writeWord(length byte, data uint16) {
	if outIndex+3 >= maxSize {
		outBuf = extendSlice(outBuf)
		maxSize = len(outBuf)
	}
	wordCtr++
	wordLens = append(wordLens, length)

	// ababab -> 3ab
	length = ((length - 1) % 32) | 0b0100_0000
	outBuf[outIndex] = length
	outBuf[outIndex+1] = byte(data >> 8)
	outBuf[outIndex+2] = byte(data)
	outIndex += 3
}

func writeLongString(length byte, offset uint16) {
	if outIndex+3 >= maxSize {
		outBuf = extendSlice(outBuf)
		maxSize = len(outBuf)
	}
	longStringCtr++
	longStringLens = append(longStringLens, length)
	longStringOffsets = append(longStringOffsets, int16(offset))

	// abcdebcd (length=3 offset=-257)-> abcde
	offset = uint16(-int(offset))
	i := ((length - 1) % 32) | 0b1010_0000
	outBuf[outIndex] = i
	outBuf[outIndex+1] = byte(offset)
	outBuf[outIndex+2] = byte(offset >> 8)
	outIndex += 3
}

func writeShortString(length byte, offset byte) {
	if outIndex+2 >= maxSize {
		outBuf = extendSlice(outBuf)
		maxSize = len(outBuf)
	}
	shortStringCtr++
	shortStringLens = append(shortStringLens, length)

	// abcdebcd (length=3 offset=130)-> abcde
	i := ((length - 1) % 32) | 0b1000_0000
	outBuf[outIndex] = i
	outBuf[outIndex+1] = offset
	outIndex += 2
}

func writeRotateString(length byte, offset uint16) {
	if outIndex+3 >= maxSize {
		outBuf = extendSlice(outBuf)
		maxSize = len(outBuf)
	}
	rotateStringCtr++

	offset = uint16(-int(offset))
	i := ((length - 1) % 16) | 0b0110_0000
	outBuf[outIndex] = i
	outBuf[outIndex+1] = byte(offset)
	outBuf[outIndex+2] = byte(offset >> 8)
	outIndex += 3
}

func writeTrash(length byte, pos []byte) {
	if outIndex+int(length) >= maxSize {
		outBuf = extendSlice(outBuf)
		maxSize = len(outBuf)
	}
	trashCtr++
	trashLens = append(trashLens, length)

	c := ((length - 1) % 64) | 0b1100_0000
	outBuf[outIndex] = c
	outIndex++

	for i := 0; i < int(length); i++ {
		outBuf[outIndex] = pos[i]
		outIndex++
	}
}

func writeEnd() {
	if outIndex+1 >= maxSize {
		outBuf = extendSlice(outBuf)
		maxSize = len(outBuf)
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

func (bs byteStream) String() string {
	builder := &strings.Builder{}
	builder.WriteString("[")
	for i, b := range bs {
		builder.WriteString(fmt.Sprintf("%02x", b))
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

// 1000_0000 -> 0000_0001
// 0010_1010 -> 0101_0100
func rotate(b byte) byte {
	result := byte(0)
	for i := 0; i < 8; i++ {
		if b&(1<<i) != 0 {
			result |= (1 << (7 - i))
		}
	}
	return result
}

func printDetail() {
	fmt.Printf("Byte Value list: %v\n", byteStream(byteValues))
	fmt.Printf("Byte: %v\n", byteStream(byteLens))
	fmt.Printf("Word: %v\n", wordLens)
	fmt.Printf("LongS: %v\n", longStringLens)
	fmt.Printf("LongString offset list: %v\n", longStringOffsets)
	fmt.Printf("ShortS: %v\n", shortStringLens)
	fmt.Printf("Trash: %v\n", trashLens)
}
