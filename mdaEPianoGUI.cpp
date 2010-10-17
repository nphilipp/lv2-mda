#include <gtkmm.h>
#include <lv2gui.hpp>
#include "mdaEPiano.peg"

using namespace sigc;
using namespace Gtk;

class mdaEPianoGUI : public LV2::GUI<mdaEPianoGUI, LV2::URIMap<true>, LV2::WriteMIDI<false> > {
public:
  
  mdaEPianoGUI(const std::string& URI) : m_button("Click me!") {
    m_button.signal_pressed().connect(mem_fun(*this, &mdaEPianoGUI::send_note_on));
    m_button.signal_released().connect(mem_fun(*this, &mdaEPianoGUI::send_note_off));
    pack_start(m_button);
  }
  
protected:
  
  void send_note_on() {
    uint8_t event[] = { 0x90, 0x40, 0x40 };
    write_midi(p_midi, 3, event);
  }

  void send_note_off() {
    uint8_t event[] = { 0x80, 0x40, 0x40 };
    write_midi(p_midi, 3, event);
  }

  void send_event() {
    uint8_t event[] = { 0x80, 0x40, 0x40 };
    write_midi(p_midi, 3, event);
  }
  
  Button m_button;
};


static int _ = mdaEPianoGUI::register_class("http://rekado.wurmus.de/lv2/mdaEPiano/gui");
