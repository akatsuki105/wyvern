package main

import (
	"io/ioutil"
	"testing"
)

func TestRotate(t *testing.T) {
	src := byte(0x4b)
	rotated := rotate(src)

	if rotated != 0xd2 {
		t.Fatalf("TestRotate failed")
	}

	rotated2 := rotate(rotated)
	if rotated2 != src {
		t.Fatalf("TestRotate failed")
	}
}

func TestRun(t *testing.T) {
	var filenames = []string{
		"./test/cenotaph.atr",
		"./test/cenotaph.chr",
		"./test/cenotaph.map",
		"./test/pokered/abra.2bpp",
		"./test/pokered/red.2bpp",
		"./test/pokecrystal/suicune_jump.2bpp",
	}

	for _, filename := range filenames {
		src, err := ioutil.ReadFile(filename)
		if err != nil {
			t.Fatalf(err.Error())
		}
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
