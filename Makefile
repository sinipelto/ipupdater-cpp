CXXFILES+=main.cpp ip.cpp utils.cpp curlpp/*.cpp curlpp/internal/*.cpp

HEADERS+=-I./include

LIBS+=-Wl,-Bstatic -lcurl -lz -lm -lssl -lcrypto -ldl -lgnutls -lnghttp2 -lresolv -lpthread
STATICLIBS+=-DCURL_STATICLIB -static-libgcc

#CXXFLAGS+=-Wall -Wextra -Wpedantic
#CXXFLAGS+=-Ofast
CXXFLAGS+=-static
CXXFLAGS+=-o bin/updater
CXXFLAGS+=$(HEADERS)

LDFLAGS += -L./lib $(LIBS) $(STATICLIBS)

all: prepare ipupdater

prepare:
	mkdir -p bin/
	mkdir -p build/
	cp -nrT skeleton/ bin/

ipupdater: $(CXXFILES)
	$(CXX) $(CXXFLAGS) $(CXXFILES) $(LDFLAGS)

clean:
	rm -rf bin/
	rm -rf build/
