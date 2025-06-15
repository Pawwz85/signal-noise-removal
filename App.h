#pragma once

#include "scenes.h" 
#include <thread>

class App;
extern App* global_app;
void app_navigate(int nav_code);

class App {
	bool isRunning_;
	SignalChannelChooser signalChooserScene_;  // nav code 0
	MovingAverageFilterScene denoisingScene1_; // nav code 1
	SincFilterScene denoisingScene2_;  // nav code 2 

	IApplicationScene* current_scene;

public:
	SignalChannelChooser & signalChooserScene() { return signalChooserScene_; };
	MovingAverageFilterScene& denoisingScene1() { return denoisingScene1_; };
	SincFilterScene& denoisingScene2() { return denoisingScene2_; };
	bool isRunning() { return isRunning_; };
	void stop() { isRunning_ = false; };

	void setCurrentScene(IApplicationScene& scene) {
		scene.setAsCurrentScene();
		current_scene = &scene;
	}

	IApplicationScene& getCurrentScene() {
		return *current_scene;
	}

	App(Fl_Window* window): signalChooserScene_(window, app_navigate),
			denoisingScene1_(window, app_navigate),
			denoisingScene2_(window, app_navigate)
	{
		setCurrentScene(signalChooserScene_);
		isRunning_ = true;
	}
	
	void start() {
		isRunning_ = true;
		global_app = this;
	}
};

extern App* global_app;

void app_navigate(int nav_code) {
	std::shared_ptr<Signal<time_t>> base;
	switch (nav_code) {
	case 0:
		global_app->signalChooserScene().setAsCurrentScene();
		break;
	case 1:
		base = global_app->signalChooserScene().getChoosenChannel();
		global_app->setCurrentScene(global_app->denoisingScene1());
		global_app->denoisingScene1().set_base_signal(base);
		current_sig_com_scene = &global_app->denoisingScene1();
		break;
	case 2:
		base = global_app->signalChooserScene().getChoosenChannel();
		global_app->setCurrentScene(global_app->denoisingScene2());
		global_app->denoisingScene2().set_base_signal(base);
		current_sig_com_scene = &global_app->denoisingScene2();
		break;
	}
}

