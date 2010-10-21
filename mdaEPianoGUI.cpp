#include <gtkmm.h>
#include <lv2gui.hpp>
#include "mdaEPiano.peg"

using namespace sigc;
using namespace Gtk;

class mdaEPianoGUI : public LV2::GUI<mdaEPianoGUI, LV2::URIMap<true>, LV2::WriteMIDI<false> > {
	public:

		mdaEPianoGUI(const std::string& URI)
		{
			Table* table = manage(new Table(2, 12));

			//initialize sliders
			envelope_decay_scale = manage(new HScale(p_ports[p_envelope_decay].min,
						p_ports[p_envelope_decay].max, 0.01));
			envelope_release_scale = manage(new HScale(p_ports[p_envelope_release].min,
						p_ports[p_envelope_release].max, 0.01));
			hardness_scale = manage(new HScale(p_ports[p_hardness].min,
						p_ports[p_hardness].max, 0.01));
			treble_boost_scale = manage(new HScale(p_ports[p_treble_boost].min,
						p_ports[p_treble_boost].max, 0.01));
			modulation_scale = manage(new HScale(p_ports[p_modulation].min,
						p_ports[p_modulation].max, 0.01));
			lfo_rate_scale = manage(new HScale(p_ports[p_lfo_rate].min,
						p_ports[p_lfo_rate].max, 0.01));
			velocity_sensitivity_scale = manage(new HScale(p_ports[p_velocity_sensitivity].min,
						p_ports[p_velocity_sensitivity].max, 0.01));
			stereo_width_scale = manage(new HScale(p_ports[p_stereo_width].min,
						p_ports[p_stereo_width].max, 0.01));
			polyphony_scale = manage(new HScale(p_ports[p_polyphony].min,
						p_ports[p_polyphony].max, 0.01));
			fine_tuning_scale = manage(new HScale(p_ports[p_fine_tuning].min,
						p_ports[p_fine_tuning].max, 0.01));
			random_tuning_scale = manage(new HScale(p_ports[p_random_tuning].min,
						p_ports[p_random_tuning].max, 0.01));
			overdrive_scale = manage(new HScale(p_ports[p_overdrive].min,
						p_ports[p_overdrive].max, 0.01));

			unsigned char scale_size = 50;
			envelope_decay_scale->set_size_request(scale_size, -1);
			envelope_release_scale->set_size_request(scale_size, -1);
			hardness_scale->set_size_request(scale_size, -1);
			treble_boost_scale->set_size_request(scale_size, -1);
			modulation_scale->set_size_request(scale_size, -1);
			lfo_rate_scale->set_size_request(scale_size, -1);
			velocity_sensitivity_scale->set_size_request(scale_size, -1);
			stereo_width_scale->set_size_request(scale_size, -1);
			polyphony_scale->set_size_request(scale_size, -1);
			fine_tuning_scale->set_size_request(scale_size, -1);
			random_tuning_scale->set_size_request(scale_size, -1);
			overdrive_scale->set_size_request(scale_size, -1);

			//connect widgets to control ports (change control values when sliders are moved)
			slot<void> od_slot = 
				compose(bind<0>(mem_fun(*this, &mdaEPianoGUI::write_control), p_overdrive),
						mem_fun(*overdrive_scale, &HScale::get_value));
			slot<void> hardness_slot = 
				compose(bind<0>(mem_fun(*this, &mdaEPianoGUI::write_control), p_hardness),
						mem_fun(*hardness_scale, &HScale::get_value));

			overdrive_scale->signal_value_changed().connect(od_slot);
			hardness_scale->signal_value_changed().connect(hardness_slot);

			//connect all faders to the 'notify' function to inform the plugin to recalculate
			hardness_scale->signal_value_changed().connect(
					mem_fun(*this, &mdaEPianoGUI::notify_param_change));
			overdrive_scale->signal_value_changed().connect(
					mem_fun(*this, &mdaEPianoGUI::notify_param_change));

			//add widgets to table...
			table->attach(*manage(new Label("Envelope Decay")), 0, 1, 0, 1);
			table->attach(*manage(new Label("Envelope Release")), 0, 1, 1, 2);
			table->attach(*manage(new Label("Hardness")), 0, 1, 2, 3);
			table->attach(*manage(new Label("Treble Boost")), 0, 1, 3, 4);
			table->attach(*manage(new Label("Modulation")), 0, 1, 4, 5);
			table->attach(*manage(new Label("LFO Rate")), 0, 1, 5, 6);
			table->attach(*manage(new Label("Velocity Sensitivity")), 0, 1, 6, 7);
			table->attach(*manage(new Label("Stereo Width")), 0, 1, 7, 8);
			table->attach(*manage(new Label("Polyphony")), 0, 1, 8, 9);
			table->attach(*manage(new Label("Fine Tuning")), 0, 1, 9, 10);
			table->attach(*manage(new Label("Random Tuning")), 0, 1, 10, 11);
			table->attach(*manage(new Label("Overdrive")), 0, 1, 11, 12);

			table->attach(*envelope_decay_scale, 1, 2, 0, 1);
			table->attach(*envelope_release_scale, 1, 2, 1, 2);
			table->attach(*hardness_scale, 1, 2, 2, 3);
			table->attach(*treble_boost_scale, 1, 2, 3, 4);
			table->attach(*modulation_scale, 1, 2, 4, 5);
			table->attach(*lfo_rate_scale, 1, 2, 5, 6);
			table->attach(*velocity_sensitivity_scale, 1, 2, 6, 7);
			table->attach(*stereo_width_scale, 1, 2, 7, 8);
			table->attach(*polyphony_scale, 1, 2, 8, 9);
			table->attach(*fine_tuning_scale, 1, 2, 9, 10);
			table->attach(*random_tuning_scale, 1, 2, 10, 11);
			table->attach(*overdrive_scale, 1, 2, 11, 12);

			//...and make the table the main widget
			add(*table);
		}
		void port_event(uint32_t port, uint32_t buffer_size, uint32_t format, const void* buffer) {
			if (port == p_overdrive)
				overdrive_scale->set_value(*static_cast<const float*>(buffer));
			else if (port == p_hardness)
				hardness_scale->set_value(*static_cast<const float*>(buffer));
		}


	protected:
		void notify_param_change() {
			write_control(p_param_changed, 1.0);
		}

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

		HScale* envelope_decay_scale;
		HScale* envelope_release_scale;
		HScale* hardness_scale;
		HScale* treble_boost_scale;
		HScale* modulation_scale;
		HScale* lfo_rate_scale;
		HScale* velocity_sensitivity_scale;
		HScale* stereo_width_scale;
		HScale* polyphony_scale;
		HScale* fine_tuning_scale;
		HScale* random_tuning_scale;
		HScale* overdrive_scale;
};

static int _ = mdaEPianoGUI::register_class("http://rekado.wurmus.de/lv2/mdaEPiano/gui");
