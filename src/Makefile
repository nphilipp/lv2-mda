BUNDLE = lv2-mdaEPiano.lv2
INSTALL_DIR = /usr/local/lib/lv2

SOURCES = *.cpp
FLAGS = -fPIC -DPIC -O $(WARNINGS)
WARNINGS = #-g -Wall -pedantic

$(BUNDLE): manifest.ttl mdaEPiano.ttl mdaEPiano.so data/samples.raw presets
	rm -rf $(BUNDLE)
	mkdir $(BUNDLE)
	cp -r $^ $(BUNDLE)

mdaEPiano.so: $(SOURCES) mdaEPiano.peg
	$(CXX) $(FLAGS) -shared $(SOURCES) `pkg-config --cflags --libs lv2-plugin` -o $@

mdaEPiano.peg: mdaEPiano.ttl
	lv2peg $^ mdaEPiano.peg

install: $(BUNDLE)
	mkdir -p $(INSTALL_DIR)
	rm -rf $(INSTALL_DIR)/$(BUNDLE)
	cp -R $(BUNDLE) $(INSTALL_DIR)

clean:
	rm -rf $(BUNDLE) mdaEPiano.so mdaEPiano.peg
