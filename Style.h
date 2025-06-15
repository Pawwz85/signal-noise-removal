#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Button.H>
const Fl_Color window_color = FL_BLACK;

const Fl_Color text_color = FL_WHITE;


const Fl_Color button_color = FL_BLUE;
const Fl_Color button_color2 = FL_DARK_BLUE;
const Fl_Color button_text_color = FL_WHITE;

static void style_button(Fl_Button* button) {
	button->color(button_color, button_color2);
	button->labelcolor(button_text_color);
	button->box(FL_ROUNDED_BOX);
}