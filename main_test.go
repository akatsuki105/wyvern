package main

import (
	"io/ioutil"
	"testing"
)

func TestRun(t *testing.T) {
	var filenames = []string{
		"./test/cenotaph.atr",
		"./test/cenotaph.chr",
		"./test/cenotaph.map",
	}

	for _, filename := range filenames {
		src, _ := ioutil.ReadFile(filename)
		compressed := compress(src)
		decompressed := decompress(compressed)

		if len(src) != len(decompressed) {
			t.Fatalf("TestRun failed: wrong size of decompressed data. want=%d, got=%d", len(decompressed), len(src))
		}

		for i, b := range src {
			if b != decompressed[i] {
				t.Fatalf("TestRun failed: %d's data is wrong in decompressed data. want=0x%02x, got=%02x", i, b, decompressed[i])
			}
		}
	}
}
