/****************************************************************************

 Copyright (C) 2002-2014 Gilles Debunne. All rights reserved.

 This file is part of the QGLViewer library version 2.7.2.

 http://www.libqglviewer.com - contact@libqglviewer.com

 This file may be used under the terms of the GNU General Public License 
 versions 2.0 or 3.0 as published by the Free Software Foundation and
 appearing in the LICENSE file included in the packaging of this file.
 In addition, as a special exception, Gilles Debunne gives you certain 
 additional rights, described in the file GPL_EXCEPTION in this package.

 libQGLViewer uses dual licensing. Commercial/proprietary software must
 purchase a libQGLViewer Commercial License.

 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

*****************************************************************************/
#include "Grid.hpp"
#include "viewer.hpp"
#include "ui_parameters.hpp"
#include "error.hpp"

#include "plotter.hpp"

#include <QCursor>
#include <QKeyEvent>
#include <QMap>
#include <QMenu>
#include <QMouseEvent>

using namespace std;

Viewer::~Viewer() {
  if (plot_ && stream_plot.is_open()) {
      stream_plot<<"set term pop; set out;";
      stream_plot.close();
    }
  
  _surface.clear();
}

void help_parse() {
  std::cout<<"\n     *** WAVE: Help ***\n"<<std::endl;
  std::cout<<"Synopsis: \n     .\\main <options>\n\nOptions:"<<std::endl;
  std::cout<<"     -l, -load <file>: load configuration file"<<std::endl;
  std::cout<<"     -stop <t>: stop animation and exit at time t"<<std::endl;
  std::cout<<"     -h, -help: print help\n"<<std::endl;
  exit(0);
}

void Viewer::treatArguments(int argc, char **argv) {
  running_ = false;
  plot_ = false;
  for (int i = 1;  i < argc; ++i) {
    std::string s(argv[i]); //transforme argv[i] en string standard
    if (s == "-l" || s == "-load") {
      if (argc < i + 2) {
        std::cerr<<"\nERROR: wrong number of arguments\n"<<std::endl;
        help_parse();
      }
      std::cout<<"Loading configuration file:"<<" "<<argv[i+1]<<std::endl;
      _surface.setImportConf(argv[i+1]);
      ++i;
    } else if (s == "-p" || s == "-plot") {
      if (argc < i + 2) {
        std::cerr<<"\nERROR: wrong number of arguments\n"<<std::endl;
        help_parse();
      }
      std::cout<<"plotting in "<<" "<<argv[i+1]<<std::endl;
      export_file_data = argv[i+1];
      plot_ = true;
      ++i;
    } else if (s == "-stop") {
      if (argc < i + 2) {
        std::cerr<<"\nERROR: wrong number of arguments\n"<<std::endl;
        help_parse();
      }
      std::cout<<"Stop at t = "<<argv[i+1]<<std::endl;
      // _surface.setStopTime(atoi(argv[i+1]));
      stop_time = atoi(argv[i+1]);
      ++i;
    } else if (s == "-r" || s == "-run") {
      running_ = true;
    } else if (s == "-h" || s == "-help") {
      std::cout<<"help"<<std::endl;
      help_parse();
    }else if(s == "-ld" || s=="-load-default"){
        _surface.setImportConf("./conf/default_static.conf");
    }
    else {
      std::cerr<<"\nERROR: Unknown option\n"<<std::endl;
      help_parse();
    }
  }
}

void Viewer::animate() {
  try {
    if (time_ > stop_time) {
      std::exit(0);
    }
  _surface.update();

  if (plot_) {
  uint n = time_;
  std::string s0 = "";
  if (n < 10) {
    s0 = "000";
  } else if (n < 100) {
    s0 = "00";
  } else if (n < 1000) {
    s0 = "0";
  }
  
  std::stringstream ss_dat;
  ss_dat <<export_file_data<<s0<<n<<".dat";
  std::string str_dat(ss_dat.str());
  _surface.drawHeighField(str_dat);
  stream_plot<<"set output \""<<export_file_data<<"_2d_"<<s0<<n<<".png\"\n";
  stream_plot<<"splot '"<<str_dat<<"' with pm3d\n";
  }
  ++time_;
   } catch (std::exception& e) {
    std::cerr << "Exception catched : " << e.what() << std::endl;
    _surface.clear();
    throw;
  }
}
  
void Viewer::draw() {
 
float pos[4] = {1.0, 1.0, 1.0, 0.0};
  // Directionnal light
  glLightfv(GL_LIGHT0, GL_POSITION, pos);

   // Point light
   qglviewer::Vec pos2 = light2->position();
   pos[0] = float(pos2.x);
   pos[1] = float(pos2.y);
   pos[2] = float(pos2.z);
   glLightfv(GL_LIGHT2, GL_POSITION, pos);
   drawLight(GL_LIGHT2);
  
  _surface.draw();
}


void Viewer::init() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
        // Light0 is the default ambient light
    glEnable(GL_LIGHT0);
        // Light2 is a classical directionnal light
    glEnable(GL_LIGHT2);
    const GLfloat light_ambient2[4] = {0.2f, 0.2f, 0.2f, 1.0};
    const GLfloat light_diffuse2[4] = {0.2f, 0.2f, 0.2, 1.0};
    const GLfloat light_specular2[4] = {0.2, 0.2, 0.2, 1.0};
    glLightfv(GL_LIGHT2, GL_AMBIENT, light_ambient2);
    glLightfv(GL_LIGHT2, GL_SPECULAR, light_specular2);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse2);
    light2 = new qglviewer::ManipulatedFrame();
    light2->setPosition(0.0, 0.0, 1);
    _surface.reset();

    // Restore previous viewer state.
    restoreStateFromFile();

    // Add custom key description (see keyPressEvent).
    setKeyDescription(Qt::Key_W, "Toggles wire frame display");
    setKeyDescription(Qt::Key_S, "Toogles sources display");
    setSceneRadius(30);
    srand(time(NULL));
    sphere.create_array();
    sphere.setSize(0.1);
    sphere.setColor(0.9f, 0.2f, 0.1f);
    // Opens help window
    // help();
    glDisable(GL_CULL_FACE);
    if (plot_) {
        std::stringstream ss_plot;
        ss_plot <<export_file_data<<"_plot.txt";
        str_plot = std::string(ss_plot.str());
        if (stream_plot.is_open()) {
            stream_plot.close();
        }
        INFO("Writing "<<str_plot);
        stream_plot.open(str_plot);
        stream_plot<<"set view map\n";
        stream_plot<<"unset key\n";
        stream_plot<<"unset tics\n";
        stream_plot<<"unset border\n";
        stream_plot<<"unset colorbox\n";
        stream_plot<<"set terminal png size 600, 600\n";
    }
    if (running_) {
        startAnimation();
    }
}


void Viewer::keyPressEvent(QKeyEvent *e) {
  // Get event modifiers key
  const Qt::KeyboardModifiers modifiers = e->modifiers();

  // A simple switch on e->key() is not sufficient if we want to take state key
  // into account. With a switch, it would have been impossible to separate 'F'
  // from 'CTRL+F'. That's why we use imbricated if...else and a "handled"
  // boolean.
  bool handled = false;
  if ((e->key() == Qt::Key_W) && (modifiers == Qt::NoButton)) {
    wireframe_ = !wireframe_;
    if (wireframe_) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    handled = true;
    update();
  } else if ((e->key() == Qt::Key_Backspace) && (modifiers == Qt::NoButton)) {
    _surface.reset();
    handled = true;
    update();
  } else if ((e->key() == Qt::Key_S) && (modifiers == Qt::NoButton)) {
    _surface.draw_sources = !_surface.draw_sources;
    handled = true;
    update();
  } else if ((modifiers == Qt::NoButton) && (e -> key() == Qt::Key_Agrave)){ //Centre la scène avec à
    camera()->setOrientation(qglviewer::Quaternion());
    camera()->showEntireScene();
    camera()->setPosition(qglviewer::Vec(30, 15, camera()->position().z / 2));
    handled = true;
    update();
  } else if ((modifiers == Qt::NoButton) && (e -> key() == Qt::Key_P)){ //affiche la position de la camera avec P
    qglviewer::Vec vec = camera()->position();
    std::cout << "x: " << vec.x << ", y: " << vec.y << ", z: " << vec.z << std::endl;
    handled = true;
  } else if ((e->key() == Qt::Key_X) && (modifiers == Qt::CTRL)){ //CTRL+X : ajoute une source au centre
    qglviewer::Vec center = qglviewer::Vec(settings::n_rows_ * settings::cell_size_ /2, settings::n_cols_ * settings::cell_size_ /2, 0);
    _surface.addEqSource(center.x, center.y, 0, 1);
    std::cout<<center.x<<" "<<center.y<<std::endl;
    handled = true;
    update();
  }else if ((e->key() == Qt::Key_H) && (modifiers == Qt::CTRL)){
      Plotter::exportHeightMap("C:/msys64/home/alois/Waves-main/plots/map.png", &_surface, settings::n_rows_, settings::n_cols_);
    handled = true;
  }
  if (!handled)
    QGLViewer::keyPressEvent(e);
}

QString Viewer::helpString() const {
  QString text("<h2>S i m p l e V i e w e r</h2>");
  text += "Use the mouse to move the camera around the object. ";
  text += "You can respectively revolve around, zoom and translate with the "
          "three mouse buttons. ";
  text += "Left and middle buttons pressed together rotate around the camera "
          "view direction axis<br><br>";
  text += "Pressing <b>Alt</b> and one of the function keys "
          "(<b>F1</b>..<b>F12</b>) defines a camera keyFrame. ";
  text += "Simply press the function key again to restore it. Several "
          "keyFrames define a ";
  text += "camera path. Paths are saved when you quit the application and "
          "restored at next start.<br><br>";
  text +=
      "Press <b>F</b> to display the frame rate, <b>A</b> for the world axis, ";
  text += "<b>Alt+Return</b> for full screen mode and <b>Control+S</b> to save "
          "a snapshot. ";
  text += "See the <b>Keyboard</b> tab in this window for a complete shortcut "
          "list.<br><br>";
  text += "Double clicks automates single click actions: A left button double "
          "click aligns the closer axis with the camera (if close enough). ";
  text += "A middle button double click fits the zoom of the camera and the "
          "right button re-centers the scene.<br><br>";
  text += "A left button double click while holding right button pressed "
          "defines the camera <i>Revolve Around Point</i>. ";
  text += "See the <b>Mouse</b> tab and the documentation web pages for "
          "details.<br><br>";
  text += "Press <b>Escape</b> to exit the viewer.";
  return text;
}
