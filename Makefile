NAME := wyvern
BINDIR := ./build
VERSION := $(shell git describe --tags 2>/dev/null)
LDFLAGS := -X 'main.version=$(VERSION)'

.PHONY: build
build:
	@go build -ldflags "$(LDFLAGS)" .

.PHONY: build-windows
build-windows:
	@GOOS=windows GOARCH=amd64 go build -o $(BINDIR)/windows-amd64/$(NAME).exe -ldflags "$(LDFLAGS)" .

.PHONY: build-darwin
build-darwin:
	@GOOS=darwin GOARCH=amd64 go build -o $(BINDIR)/darwin-amd64/$(NAME) -ldflags "$(LDFLAGS)" .

.PHONY: build-linux
build-linux:
	@GOOS=linux GOARCH=amd64 go build -o $(BINDIR)/linux-amd64/$(NAME) -ldflags "$(LDFLAGS)" .

.PHONY: clean
clean:
	@-rm -rf $(BINDIR)

.PHONY: compress
compress:
	make build
	@./wyvern -v ${SRC} ./asm/packed.wyv && cd asm && make build && cd ..

.PHONY: test
test:
	@echo "**Go**"
	@go test
	@echo "\n**Assembly**"
	@cd ./asm && make test


TEST := ./test/
POKERED := $(TEST)pokered/
POKECRY := $(TEST)pokecrystal/
LADX := $(TEST)ladx/
TARGETS := $(TEST)cenotaph.atr $(TEST)cenotaph.chr $(TEST)cenotaph.map $(POKERED)abra.2bpp $(POKERED)red.2bpp $(POKECRY)chris.2bpp $(POKECRY)diploma.2bpp $(POKECRY)ditto.2bpp $(POKECRY)suicune_jump.2bpp $(LADX)animated_tiles.w32.cgb.2bpp $(LADX)camera_shop.2bpp $(LADX)christine.cgb.2bpp $(LADX)intro_1.cgb.2bpp $(LADX)oam_color_dungeon.2bpp

.PHONY: benchmark
benchmark:
	@make build
	@for target in $(TARGETS); do ./wyvern -b -v $$target; done

.PHONY: benchmark-exhal
benchmark-exhal:
	@go build -o tools/bin/exhal tools/exhal/exhal.go
	@for target in $(TARGETS); do echo $$target && ./tools/bin/inhal -n $$target comped 1> /dev/null && ./tools/bin/exhal comped; done
	@rm -rf comped
