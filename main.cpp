#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Chart.H>
#include <FL/Fl_Choice.H>
#include <math.h>


#include "signals.h"
#include <ctime>
#include <cstdlib>

#include "scenes.h"
#include <iostream>

#include "Style.h"
#include "App.h"
// main

Fl_Window* G_win = 0;
App* global_app = 0;

int main() {
    //srand(time(0));
    Fl_Font();
    G_win = new Fl_Window(1024, 1024, "oscillator");
    G_win->color(window_color);
    global_app = new App(G_win);
 
    
    G_win->resizable(G_win);
    G_win->show();
    
    global_app->start();
    
    return(Fl::run());

  
}
