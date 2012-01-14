BUNDLE = lv2-mdaEPiano.lv2
INSTALL_DIR = /usr/local/lib/lv2


$(BUNDLE): manifest.ttl mdaEPiano.ttl mdaEPiano.so samples.raw
	rm -rf $(BUNDLE)
	mkdir $(BUNDLE)
	cp $^ $(BUNDLE)

mdaEPiano.so: mdaEPiano.cpp mdaEPianoVoice.cpp mdaEPiano.peg
	g++ -shared -g -O -Wall -Weffc++ -pedantic -fPIC -DPIC mdaEPiano.cpp mdaEPianoVoice.cpp `pkg-config --cflags --libs lv2-plugin` -o mdaEPiano.so

mdaEPiano.peg: mdaEPiano.ttl
	lv2peg mdaEPiano.ttl mdaEPiano.peg

install: $(BUNDLE)
	mkdir -p $(INSTALL_DIR)
	rm -rf $(INSTALL_DIR)/$(BUNDLE)
	cp -R $(BUNDLE) $(INSTALL_DIR)

clean:
	rm -rf $(BUNDLE) mdaEPiano.so mdaEPiano.peg
