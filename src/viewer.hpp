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

#include <QGLViewer/qglviewer.h>
#include <QGLViewer/manipulatedFrame.h>
#include "WaterSurface.hpp"
#include <fstream>

class Viewer : public QGLViewer {
protected:
  virtual void draw();
  virtual void animate();
  virtual void init();
  virtual QString helpString() const;
  void keyPressEvent(QKeyEvent *e);

private:
  WaterSurface _surface;
  
  Sphere sphere;

  qglviewer::ManipulatedFrame *light2;
  
  uint time_ = 0;
  uint stop_time = 1e4; // program stopped after <stop_time> frame
  bool running_;
  
  bool plot_;
  std::string export_file_data;
  std::string str_plot;
  std::ofstream stream_plot;

  // rendering option
  bool wireframe_;

public:
  Viewer() : wireframe_(false) {};
  ~Viewer();
  void treatArguments(int argc, char **argv);
};
