#pragma once

#include <string>
#include <vector>
#include <memory>
#include <ctime>
#include <mutex> // scene comparision uses worker thread
#include "signals.h"
#include "csv_parser.h"
#include "Style.h"

#include <FL/Fl_Window.H>
#include <FL/Fl_Chart.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Chooser.H>


struct layoutParameters {
	int padding;
	int button_width;
	int button_height;
	int label_gap;
};

static layoutParameters* makeDefaultLayoutParams(){
	layoutParameters * result = new layoutParameters;
	result->padding = 20;
	result->button_width = 150;
	result->button_height = 40;
	result->label_gap = 10;

	return result;
}


class IApplicationScene {
protected:
	void (*navigation_request_handler)(int scene_code);

	layoutParameters* layout = makeDefaultLayoutParams();
	Fl_Window* parent_window;
	virtual void compose() = 0;
public:

	IApplicationScene(Fl_Window* parent_window, void (*nav)(int) = nullptr): parent_window(parent_window), navigation_request_handler(nav) {}
	
	void redraw() {
		parent_window->clear();
		parent_window->begin();
		compose();
		parent_window->end();
		parent_window->redraw();
	}

	void setAsCurrentScene() {
		redraw();
	}

	void navigate(int scene_code)  {
		if (navigation_request_handler) {
			navigation_request_handler(scene_code);
		}
	}
};

class SignalComparisionScene;
class SignalChannelChooser;
class MovingAverageFilterScene;
class SincFilterScene;

static void _on_user_choose_file(Fl_File_Chooser* _, void* data);
extern SignalChannelChooser* signal_chooser_scene;
extern MovingAverageFilterScene* mov_avg_filter_scene;
extern SincFilterScene* sinc_filter_scene;
extern SignalComparisionScene* current_sig_com_scene;
class SignalChannelChooser: public IApplicationScene {

	void reload_file(const std::string& signal_file, const char sep, time_t probing_period) {
		this->currentSignalChannels = CSV_READER::load_multichannel_signals_from_csv(signal_file, sep, probing_period);
		this->chosenSignalFile = signal_file;
		this->chosen_channel_index = 0;
		this->probing_period = probing_period;
	}

protected:

	std::string fileChooserLabel = "Nie Wybrano pliku";
	std::string channelChooserLabel = "Wybrany kanal: 0";

	// scene model variables
	std::string chosenSignalFile = ""; // path to currently opened signal file
	std::vector < std::unique_ptr<Signal<time_t>>> currentSignalChannels;
	size_t chosen_channel_index = 0;
	
	std::vector<std::string> choices;

	time_t probing_period = 100;
	char sep = ';';

	// scene widgets
	Fl_Box* file_label = 0;
	Fl_Box* channel_label = 0;

	Fl_Chart* oscillator = 0; // TODO: initialise variable
	Fl_Choice* channel_choice = 0;
	Fl_Button* choose_file_button = 0;
	Fl_File_Chooser* file_chooser = new Fl_File_Chooser("\\", "{*.csv}", Fl_File_Chooser::SINGLE, "Wybierz plik z sygnalem");

	void compose(); // TODO: implement this method 
	void clean();
public:
	SignalChannelChooser(Fl_Window* parent_window, void (*nav)(int)) : IApplicationScene(parent_window, nav) {
		signal_chooser_scene = this;
		file_chooser->callback(_on_user_choose_file);

	}
	bool anyFileLoaded() {
		return currentSignalChannels.size() > 0;
	}

	void onFileChange(const std::string & signal_file, const char sep) {
		reload_file(signal_file, sep, probing_period);
		redraw();
	}
	
	void onProbingPeriodChange(time_t probing_period) {
		this->probing_period = probing_period;
		if (this->anyFileLoaded()) {
			reload_file(chosenSignalFile, sep, probing_period);
			redraw();
		}
	};

	void setChoosenChannel(int channel) {

		channel = (channel < 0 || channel > this->currentSignalChannels.size()) ? 0 : channel;
		this->chosen_channel_index = channel;
		redraw();
	}

	void chooseFile() {
		this->file_chooser->show();
	}

	std::shared_ptr<Signal<time_t>> getChoosenChannel() {
		return std::make_shared< Signal<time_t > >(*currentSignalChannels[chosen_channel_index].get());
	}
};

static void _display_signal_file_chooser(Fl_Widget* _) {
	signal_chooser_scene->chooseFile();
}

static void _on_user_choose_file(Fl_File_Chooser * _, void * data) {
	if(_->value() && !_->visible())
		signal_chooser_scene->onFileChange(_->value(), ';');
}

static void _on_user_channel_choose(Fl_Widget* _) {
	signal_chooser_scene->setChoosenChannel(((Fl_Choice*)_)->value());
};



template<int nav_code>
void change_scene(Fl_Widget* _) {
	signal_chooser_scene->navigate(nav_code);
}

static void _on_user_choose_export_file(Fl_File_Chooser* _, void* data);

class SignalComparisionScene : public IApplicationScene {
	/*
		This scene is focused entirely on displaying two signals side by side
	*/

	// A bunch of coordinates tied to SIGNAL COMPARISION
	int y;
	int x;
	int height;
	int width;

	Fl_Chart* left_chart;
	Fl_Chart* right_chart;


	bool refresh_left_chart;
	bool refresh_right_chart;

	Fl_File_Chooser* file_chooser = new Fl_File_Chooser("\\", "{*.csv}", Fl_File_Chooser::CREATE, "Wybierz plik do zapisu");
protected:
	
	virtual void compose_interactive_section() = 0;
	virtual void compose() final;

	// Signals to compare
	std::shared_ptr<Signal<time_t>> left_signal;
	std::shared_ptr<Signal<time_t>> right_signal;

	// this method
	std::mutex lock; 


public:

	SignalComparisionScene(Fl_Window* parent_window, void (*nav)(int) = nullptr) : IApplicationScene(parent_window, nav) {

		refresh_left_chart = refresh_right_chart = true;
		setComparisionArea(0, parent_window->h() / 2, parent_window->w(), parent_window->h()/2);
		file_chooser->callback(_on_user_choose_export_file);

	}

	void setComparisionArea(int x, int y, int width, int height) {
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
	}

	void setLeftSignal(std::shared_ptr<Signal<time_t>> signal) { this->left_signal = signal; refresh_left_chart = true; };
	void setRightSignal(std::shared_ptr<Signal<time_t>> signal) { this->right_signal = signal; refresh_right_chart = true; };


	void redrawRightChart() {
		parent_window->remove(right_chart);
		int chart_width = width / 2 - 3 * layout->padding;
		int chart_height = height;
		
		right_chart = new Fl_Chart(x + chart_width + 2 * layout->padding, y, chart_width, chart_height);
		parent_window->add(right_chart);

		right_chart->color(window_color);
		right_chart->textcolor(FL_WHITE);
		right_chart->type(FL_LINE_CHART);
		if (auto ptr = right_signal.get()) {
			for (auto it = ptr->begin(); it != ptr->end(); ++it)
				right_chart->add(it->value, 0, FL_RED);
		}

		parent_window->redraw();
	}

	void redrawLeftChart() {
		parent_window->remove(left_chart);
		int chart_width = width / 2 - 3 * layout->padding;
		int chart_height = height;
	
		left_chart = new Fl_Chart(x, y, chart_width, chart_height);
		parent_window->add(left_chart);


		left_chart->color(window_color);
		left_chart->textcolor(FL_WHITE);
		left_chart->type(FL_LINE_CHART);
		if (auto ptr = left_signal.get()) {
			for (auto it = ptr->begin(); it != ptr->end(); ++it)
				left_chart->add(it->value, 0, FL_GREEN);
		}

	}

	void chooseExportFile() {
		this->file_chooser->show();
	}

	void exportRightSignal(std::string filename) {
		CSV_READER::save_single_channel_signal_to_csv(filename, right_signal);
	}

	std::mutex& getLock() { return lock; };
};

static void _on_user_choose_export_file(Fl_File_Chooser* _, void* data) {
	if (_->value() && !_->visible())
		current_sig_com_scene->exportRightSignal(_->value());
}

static void _display_export_file_chooser(Fl_Widget* _) {
	current_sig_com_scene->chooseExportFile();
}

class MovingAverageFilterScene : public SignalComparisionScene {
	int moving_avg_window_size = 16;
	int maximum_size = 128;
	Fl_Slider* window_size_slider = nullptr;
	// TODO: implement this class

	void compose_interactive_section();

	void recalculate_right_signal() {
		if (auto signal = left_signal.get()) {
			maximum_size = sqrt(signal->size());
			Signal<time_t> denoised_signal = FourierTransform::denoise_signal_using_moving_average(*signal, moving_avg_window_size, 100);
			setRightSignal(std::make_shared<Signal<time_t>>(denoised_signal));
		}
		else {
			setRightSignal(nullptr);
		}
	}

public: 

	MovingAverageFilterScene(Fl_Window* parent_window, void (*nav)(int) = nullptr) : SignalComparisionScene(parent_window, nav) {
		mov_avg_filter_scene = this;
	}

	void set_base_signal(std::shared_ptr<Signal<time_t>> base) {
		moving_avg_window_size = 4;

		setLeftSignal(base);
		recalculate_right_signal();
		redraw();
	}

	void set_moving_window_size(int new_size) {

		this->moving_avg_window_size = new_size;
		recalculate_right_signal();
		redrawRightChart();
	}

	int get_maximum() {
		return maximum_size;
	}

};

class SincFilterScene : public SignalComparisionScene {
	int moving_avg_window_size = 16;
	int maximum_size = 256;
	Fl_Slider* window_size_slider = nullptr;
	// TODO: implement this class

	void compose_interactive_section();

	void recalculate_right_signal() {
		if (auto signal = left_signal.get()) {
			maximum_size = sqrt(signal->size());
			Signal<time_t> denoised_signal = FourierTransform::denoise_signal_using_moving_average(*signal, moving_avg_window_size, 100);
			setRightSignal(std::make_shared<Signal<time_t>>(denoised_signal));
		}
		else {
			setRightSignal(nullptr);
		}
	}

public:

		SincFilterScene(Fl_Window* parent_window, void (*nav)(int) = nullptr) : SignalComparisionScene(parent_window, nav) {
		sinc_filter_scene = this;
	}

	void set_base_signal(std::shared_ptr<Signal<time_t>> base) {
		moving_avg_window_size = 4;

		setLeftSignal(base);
		recalculate_right_signal();
		redraw();
	}

	void set_moving_window_size(int new_size) {

		this->moving_avg_window_size = new_size;
		recalculate_right_signal();
		redrawRightChart();
	}

	int get_maximum() {
		return maximum_size;
	}

};

// TODO: avoid 
static void _on_user_changed_mov_avg_window_size(Fl_Widget* _) {
	Fl_Slider* slider = (Fl_Slider*)_;
	int value = slider->value();
	mov_avg_filter_scene->set_moving_window_size(value);
}

static void _on_user_changed_sinc_window_size(Fl_Widget* _) {
	Fl_Slider* slider = (Fl_Slider*)_;
	int value = slider->value();
	sinc_filter_scene->set_moving_window_size(value);
}

class FilterDenoisingScene : public SignalComparisionScene {


	void recalculate_right_signal() {
		if (auto signal = left_signal.get()) {
			Signal<time_t> denoised_signal = FourierTransform::denoise_signal_by_using_filters(*signal, 100);
			setRightSignal(std::make_shared<Signal<time_t>>(denoised_signal));
		}
		else {
			setRightSignal(nullptr);
		}
	}

	void compose_interactive_section() {
		setComparisionArea(0, 0, parent_window->w(), parent_window->h()/2);
	}
public:

	FilterDenoisingScene(Fl_Window* parent_window, void (*nav)(int) = nullptr) : SignalComparisionScene(parent_window, nav) {
	}

	void set_base_signal(std::shared_ptr<Signal<time_t>> base) {

		setLeftSignal(base);
		recalculate_right_signal();
	}
};
