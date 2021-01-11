NAME := gbcompress
VERSION := $(shell git describe --tags 2>/dev/null)
LDFLAGS := -X 'main.version=$(VERSION)'

.PHONY: build
build:
	@go build -ldflags "$(LDFLAGS)" .

.PHONY: test
test:
	@go test
