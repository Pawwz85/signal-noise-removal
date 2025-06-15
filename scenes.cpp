#include "scenes.h"

#include "FL/Fl_Box.H"
#include <FL/Fl_Slider.H>
#include <sstream>

SignalChannelChooser* signal_chooser_scene = nullptr;
MovingAverageFilterScene* mov_avg_filter_scene = nullptr;
SincFilterScene* sinc_filter_scene = nullptr;
SignalComparisionScene* current_sig_com_scene = nullptr;

void SignalChannelChooser::compose()
{
        int padding = layout->padding;
        int button_width = layout->button_width;
        int button_height = layout->button_height;
        int label_gap = layout->label_gap;

        int window_width = parent_window->w();
        int window_height = parent_window->h();

        int center_x = window_width / 2;

        bool displayChannel = false;

       
        if (displayChannel = anyFileLoaded()) {
            fileChooserLabel = "Wybrany plik: " + chosenSignalFile;
            channelChooserLabel = "Wybrany kanal: " + std::to_string(chosen_channel_index + 1);
        }
        else {
            fileChooserLabel = "Nie wybrano pliku";
        }

        // ===============================
        // "Wybrany plik: ..." + "Wybierz plik" button
        // ===============================
        file_label = new Fl_Box(center_x - 200, padding, 300, button_height, fileChooserLabel.c_str());
        file_label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        file_label->labelcolor(text_color);

        choose_file_button = new Fl_Button(center_x + 120, padding, button_width, button_height, "Wybierz plik");
        choose_file_button->callback(_display_signal_file_chooser);
        style_button(choose_file_button);
        if (displayChannel){

       

            // ===============================
            // "Wybrany kana³: ..." + "Wybierz kana³" choice
            // ===============================
            int second_row_y = padding + button_height + label_gap;

            choose_file_button->position(center_x - 200 + padding, second_row_y);
            
            /*
            channel_label = new Fl_Box(center_x - 200, second_row_y, 300, button_height, channelChooserLabel.c_str());
            channel_label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
            */
            channel_choice = new Fl_Choice(center_x + 120, second_row_y, button_width, button_height, "Wybierz kanal");
            channel_choice->labelcolor(text_color);

            choices.clear();
            for (size_t channel_index = 0; channel_index < currentSignalChannels.size(); ++channel_index) {
                choices.push_back(std::to_string(channel_index + 1).c_str());
                channel_choice->add(choices.back().c_str());
            }
            
            channel_choice->callback(_on_user_channel_choose);

            // ===============================
            // Oscillator chart
            // ===============================
            int chart_y = second_row_y + button_height + 2 * label_gap;
            int chart_height = 400;
            int chart_width = 400;

            oscillator = new Fl_Chart(center_x - chart_width / 2, chart_y, chart_width, chart_height);
            oscillator->type(FL_LINE_CHART);
            oscillator->bounds(-1.5, 1.5);
            oscillator->color(window_color);
            oscillator->textcolor(FL_WHITE);
            oscillator->clear();

            if(currentSignalChannels.size()) for (auto it = currentSignalChannels[chosen_channel_index].get()->begin(); it != currentSignalChannels[chosen_channel_index].get()->end(); ++it) {
                oscillator->add(it->value, 0, FL_GREEN);
            }

          // ===============================
          // Nav buttons
          // ===============================
            int nav_buttons_y = chart_y + chart_height + layout->padding;
            Fl_Button* nav_button1 = new Fl_Button(center_x - 200 + padding, nav_buttons_y, button_width, button_height, "Moving Average");
            style_button(nav_button1);
            nav_button1->callback(change_scene<1>);

            Fl_Button* nav_button2 = new Fl_Button(padding + window_width/2, nav_buttons_y, button_width, button_height, "Sinc filter");
            nav_button2->callback(change_scene<2>);
            style_button(nav_button2);
        }
}

void SignalComparisionScene::compose()
{
    compose_interactive_section();

    int chart_width = width/2 - 3 * layout->padding;
    int chart_height = height;

    Signal<time_t>* ptr;

    left_chart = new Fl_Chart(x + layout->padding, y, chart_width, chart_height);
    left_chart->color(window_color);
    left_chart->textcolor(FL_WHITE);
    left_chart->type(FL_LINE_CHART);

    if (ptr = left_signal.get()) {
        for (auto it = ptr->begin(); it != ptr->end(); ++it)
            left_chart->add(it->value, 0, FL_GREEN);
    }
    

    right_chart = new Fl_Chart(x + chart_width + 2 * layout->padding, y, chart_width, chart_height);
    right_chart->color(window_color);
    right_chart->textcolor(FL_WHITE);
    right_chart->type(FL_LINE_CHART);
    if (ptr = right_signal.get()) {
        for (auto it = ptr->begin(); it != ptr->end(); ++it)
            right_chart->add(it->value, 0, FL_RED);
    }



    Fl_Button* back = new Fl_Button(x + width / 3 - layout->button_width / 2, y + chart_height, layout->button_width, layout->button_height, "Wybierz inny sygnal");
    back->callback(change_scene<0>);
    style_button(back);

    Fl_Button* export_result = new Fl_Button(x + 2*width / 3 - layout->button_width / 2, y + chart_height, layout->button_width, layout->button_height, "Eksportuj do pliku");
    export_result->callback(_display_export_file_chooser);
    style_button(export_result);
}

void MovingAverageFilterScene::compose_interactive_section()
{
    this->window_size_slider = new Fl_Slider(layout->padding, layout->padding, parent_window->w() - 2 * layout->padding, layout->button_height, "Moving Average Window Size");
    this->window_size_slider->labelcolor(text_color);

    this->window_size_slider->type(FL_HOR_NICE_SLIDER);
    this->window_size_slider->value(this->moving_avg_window_size);
    this->window_size_slider->minimum(1);
    this->window_size_slider->maximum(maximum_size);
    this->window_size_slider->step(1);
    this->window_size_slider->callback(_on_user_changed_mov_avg_window_size);
    this->window_size_slider->color(window_color);
   
    setComparisionArea(0, layout->button_height + 2* layout->padding, parent_window->w(), parent_window->h() / 2);
}

void SincFilterScene::compose_interactive_section()
{
    this->window_size_slider = new Fl_Slider(layout->padding, layout->padding, parent_window->w() - 2 * layout->padding, layout->button_height, "Sinc Conv Window Size");
    this->window_size_slider->labelcolor(text_color);

    this->window_size_slider->type(FL_HOR_NICE_SLIDER);
    this->window_size_slider->value(this->moving_avg_window_size);
    this->window_size_slider->minimum(1);
    this->window_size_slider->maximum(maximum_size); 
    this->window_size_slider->step(1);
    this->window_size_slider->callback(_on_user_changed_sinc_window_size);
    this->window_size_slider->color(window_color);
    setComparisionArea(0, layout->button_height + 2 * layout->padding, parent_window->w(), parent_window->h() / 2);
}
