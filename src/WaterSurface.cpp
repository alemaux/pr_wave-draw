/* 
 * File: WaterSurface.cpp
 *
 * Copyright (C) 2019-2024  Camille Schreck
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "WaterSurface.hpp"
#include "viewer.hpp"
#include "EquivalentSource.hpp"
#include "settings.hpp"
#include "ui_parameters.hpp"
#include "error.hpp"
#include "Times.hpp"
#include <fstream>
#include <boost/math/special_functions/bessel.hpp>


using namespace definitions;
using namespace settings;
using namespace ui_parameters;

WaterSurface::WaterSurface() {
//omp_set_num_threads(NTHREADS_);
  Eigen::setNbThreads(NTHREADS_);
  
  import_ = false;
  export_ = false;
  load_conf = false;
  draw_sources = false;

  stop_time = 1e4;

  data_file = "";
  sphere_source.create_array();
}

WaterSurface::~WaterSurface() {
  clear();
}

void WaterSurface::clear() {
  std::list<EquivalentSource*>::iterator it;
  std::vector<std::list<EquivalentSource*> >::iterator itwf;
  for (itwf= waves.begin(); itwf != waves.end(); ++itwf) {
    for (it = (*itwf).begin(); it != (*itwf).end(); ++it) {
      delete (*it);
    }
  }
  waves.clear();
}

void WaterSurface::reset() {
  clear();
    sourcesPos.clear();
  constraintsPos.clear();
  createTabs();
  sphere_source.setSize(0.5);
  sphere_source.setColor(0.8f, 0.2f, 1.0f);
  
  srand (std::time(NULL));
  
  step_wl = init_wl_;
  min_wl = 1;
  nb_wl = 1;
  max_wl = 1;

  setLists();
  
  if (load_conf) {
    importConfig(conf_file);
  }

  u = Grid(n_rows_, n_cols_, cell_size_);
  u.setColor(29.0/256.0,162.0/256.0,216.0/256.0);

  if(settings::doLoadTexture){
    pattern = Grid(n_rows_, n_cols_, cell_size_);
    pattern.setColor(0.5, 0.5, 0.5);
    pattern.loadTexture("test_texture2.png");
  }
  //INFO("Wavelength: "<<nb_wl<<" wavelenths between "<<min_wl<<" and "<<max_wl);
  
  time = 0;
  update();


#ifdef PLOT_RESULT
  std::stringstream ss;
  ss <<data_file<<"ampli.txt";
  std::string str(ss.str());
  std::stringstream ss4;
  ss4 <<data_file<<"phase.txt";
  std::string str4(ss4.str());
  std::stringstream ss5;
  ss5 <<data_file<<"ampli_dir.txt";
  std::string str5(ss5.str());
  std::stringstream ss6;
  ss6 <<data_file<<"analytic.txt";
  std::string str6(ss6.str());

  exportAmplitude(str);
  exportPhase(str4);
#endif

  INFO("DONE!   "<<nb_wl);
}

void WaterSurface::setLists() {
  std::list<Wave*>::iterator it;
  wave_lenghts = std::vector<FLOAT>();

  FLOAT wl = min_wl;
  nb_wl = 1;
  wave_lenghts.push_back(wl);
  while (wl < max_wl) {
    wl *= step_wl;
    wave_lenghts.push_back(wl);
    ++nb_wl;
  }
   
  
  waves = std::vector<std::list<EquivalentSource*> >(nb_wl);
  ampli_re = std::vector<Grid>(nb_wl);
  ampli_im = std::vector<Grid>(nb_wl);
  for (int i = 0; i < nb_wl; ++i) {
    ampli_re[i] = Grid(n_rows_, n_cols_, cell_size_);
    ampli_im[i] = Grid(n_rows_, n_cols_, cell_size_);
  }
}

void WaterSurface::setAmpli() {
    std::list<EquivalentSource*>::iterator it;

    for (int w = 0; w < nb_wl; ++w) {
      ampli_re[w].reset(0);
      ampli_im[w].reset(0);
      std::list<EquivalentSource*> waves_wl = waves[w];

        for (it = waves_wl.begin(); it != waves_wl.end(); ++it) {
#pragma omp parallel for
            for (int i = 0; i < n_rows_ - 1; i++) {
                FLOAT x = cell_size_*i;
                for (int j = 0; j < n_cols_ - 1; j++) {
                    FLOAT y = cell_size_*j;
                    ampli_re[w](i, j) += real((*it)->heightc(x, y, 0));
                    ampli_im[w](i, j) += imag((*it)->heightc(x, y, 0));
                }
            }
        }
      
    }
}

void WaterSurface::setAmpli(FLOAT t) {
    std::list<EquivalentSource*>::iterator it;
    for (int w = 0; w < nb_wl; ++w) {
        ampli_re[w].reset(0);
        ampli_im[w].reset(0);

        std::list<EquivalentSource*> waves_wl = waves[w];

        for (it = waves_wl.begin(); it != waves_wl.end(); ++it) {
            #pragma omp parallel for
            for (int i = 0; i < n_rows_ - 1; i++) {
                FLOAT x = cell_size_*i;
                for (int j = 0; j < n_cols_ - 1; j++) {
                    FLOAT y = cell_size_*j;
                    ampli_re[w](i, j) += real((*it)->heightc(x, y, t));
                    ampli_im[w](i, j) += imag((*it)->heightc(x, y, t));
                }
            }
        }
    }
}


FLOAT WaterSurface::height(int i, int j) const {
  return u(i, j);
}


void WaterSurface::addEqSource(FLOAT x, FLOAT y, FLOAT wl, COMPLEX ampli) {
  if (wl == 0) {
    wl = wave_lenghts[nb_wl-1];
  }
  for (int ind = 0; ind < nb_wl && wave_lenghts[ind]<=wl; ++ind) {
    EquivalentSource *w = new EquivalentSource(wave_lenghts[ind]);
    ((EquivalentSource*)w)->setPos(x, y);
    ((EquivalentSource*)w)->setAmplitude(ampli);
    waves[ind].push_back(w);
  }
  sourcesPos.push_back(VEC2(x, y));
}

void WaterSurface::addEqSource(EquivalentSource eq){
    WaterSurface::addEqSource(eq.getPos().x(), eq.getPos().y(), settings::init_wl_, 1);
}


void WaterSurface::update() {
    TR("TIME: "<<time);
  
    Times::TIMES->tick(Times::sum_up_time_);    
    updateHeight();
    Times::TIMES->tock(Times::sum_up_time_);   

  ++time;
  
  if (time > stop_time) {
    exit(0);
  }
}



void WaterSurface::updateHeight() {
  u.reset(0.0);
  FLOAT t = time*dt_;

  setAmpli(t);
  for (int f = 0; f < nb_wl; ++f) {
    FLOAT k =  2*M_PI/wave_lenghts[f];
    FLOAT omega = angular_vel(k);
#pragma omp parallel for
      for (int i = 0; i < n_rows_ - 1; i++) {
	for (int j = 0; j < n_cols_ - 1; j++) {
	  u(i, j) += real(COMPLEX(ampli_re[f](i, j), ampli_im[f](i, j))*exp(-omega*t*i_));
	}
      }
       
  }
}


void WaterSurface::draw() {
    u.draw();
    glPushMatrix();
    glTranslatef(30, 0, 0);
    pattern.draw();
    glPopMatrix();

    if (draw_sources) {
        sphere_source.setColor(0.8f, 0.2f, 1.0f);
        for (auto &it : sourcesPos) {
            VEC2 c = it;
            glPushMatrix();
            glTranslatef(c[0], c[1], 1.0f);
            sphere_source.draw();
            glPopMatrix();
        }
        sphere_source.setColor(0.0f, 1.0f, 0.0f);
        for(auto &it : constraintsPos){
            VEC3 c = it;
            glPushMatrix();
            glTranslatef(c[0], c[1], c[2]);
            sphere_source.draw();
            glPopMatrix();
        }
      }
}


void WaterSurface::exportAmplitude(std::string file) const {
  std::ofstream out_file;
  out_file.open(file);

  for (int i = 0; i < n_rows_; ++i) {
    for (int j = 0; j < n_cols_; ++j) {
      FLOAT a =	sqrt(pow(ampli_re[0](i, j), 2) +
		     pow(ampli_im[0](i, j), 2));
	a =20*log10(a/0.001);
      out_file<<i<<" "<<j<<" "<<a<<"\n";
    }
    out_file<<"\n";
  }
  out_file.close();
}

void WaterSurface::exportAmplitudeRe(std::string file) const {
  std::ofstream  out_file;
  out_file.open(file);

  for (int i = 0; i < n_rows_; ++i) {
    for (int j = 0; j < n_cols_; ++j) {
      FLOAT a = ampli_re[0](i, j);
      out_file<<i<<" "<<j<<" "<<a<<"\n";
    }
    out_file<<"\n";
  }
  out_file.close();
}

void WaterSurface::exportAmplitudeIm(std::string file) const {
  std::ofstream  out_file;
  out_file.open(file);

  for (int i = 0; i < n_rows_; ++i) {
    for (int j = 0; j < n_cols_; ++j) {
      FLOAT a = ampli_im[0](i, j);
      out_file<<i<<" "<<j<<" "<<a<<"\n";
    }
    out_file<<"\n";
  }
  out_file.close();
}

void WaterSurface::exportPhase(std::string file) const {
  std::ofstream  out_file;
  out_file.open(file);

  for (int i = 0; i < n_rows_; ++i) {
    for (int j = 0; j < n_cols_; ++j) {
      FLOAT a = sqrt(pow(ampli_re[0](i, j), 2) +  pow(ampli_im[0](i, j), 2));
      out_file<<i<<" "<<j<<" "<<acos(ampli_re[0](i, j)/a)<<"\n";
    }
    out_file<<"\n";
  }
  out_file.close();
}


void WaterSurface::exportSurfaceTime(std::string file) const {
  VERBOSE(1, "Exporting surface grid: "<<file);
  std::ofstream os(file.c_str());
  ERROR(os.good(), "Cannot open file "<<file, "");
  os<<u;
  os.close();
}
 
void WaterSurface::importSurfaceTime(std::string file) {
  VERBOSE(1, "Importing surface grid: "<<file);
  std::ifstream is(file.c_str());
  ERROR(is.good(), "Cannot open file "<<file, "");
  is>>u;
  is.close();
}

void WaterSurface::importConfig(std::string file) {
  std::ifstream is(file);
  ERROR(is.good(), "Cannot open file "<<file, "");
  std::string line;
  while (getline(is, line)) {
    if (line.substr(0,1) == "#") {
      //comment
    } else if (line.substr(0,9) == "<gravity>") {
      std::istringstream s(line.substr(9)); //Gravity value
      s >> gravity_ ;
    } else if (line.substr(0,9) == "<damping>") {
      std::istringstream s(line.substr(9)); //Amortissement
      s >> damping_ ;
    } else if (line.substr(0,7) == "<ampli>") {
      std::istringstream s(line.substr(7));
      s >> height_ampli_ ;
    } else if (line.substr(0,6) == "<grid>") { //GRID DEF 
      getline(is, line);
      while (line.substr(0,7) != "</grid>") {
	if (line.substr(0,6) == "<size>") {
	  std::istringstream s(line.substr(6));
	  s >> n_rows_ >> n_cols_;
	} else if (line.substr(0,11) == "<cell_size>") {
	  std::istringstream s(line.substr(11));
	  s >> cell_size_;
	} else {
      ERROR(false, "Invalid configuration file (grid)\""<<file, line);
	}
	scale_ = cell_size_ * n_rows_;
	getline(is, line);
      }
    } else if (line.substr(0,14) == "<wave_lenghts>") { //WAVE LENGHT RANGE 
      getline(is, line);
      while (line.substr(0,15) != "</wave_lenghts>") {
	if (line.substr(0,5) == "<min>") {
	  std::istringstream s(line.substr(5));
	  s >> min_wl;
	} else if (line.substr(0,5) == "<max>") {
	  std::istringstream s(line.substr(5));
	  s >> max_wl;
	} else if (line.substr(0,8) == "<number>") {
	  std::istringstream s(line.substr(8));
	  s >> nb_wl;
	  step_wl = (max_wl - min_wl)/(FLOAT)(nb_wl);
	} else if (line.substr(0,6) == "<step>") {
	  std::istringstream s(line.substr(6));
	  s >> step_wl;
	  nb_wl = (max_wl - min_wl)/(FLOAT)(step_wl);
	} else {
      ERROR(false, "Invalid configuration file (wave lenghts)\""<<file, line);
	}
	getline(is, line);
      }
      setLists();
	 	 
    } else if (line.substr(0,6) == "<wave>") { //def of a wave
      getline(is, line);
      FLOAT wl = 0;
      int ind = 0;
      FLOAT ampli = height_ampli_;
      while (line.substr(0,7) != "</wave>") {
	if (line.substr(0,13) == "<wave_lenght>") {
	  std::istringstream s(line.substr(11));
	  s >> wl;
	} else if (line.substr(0,11) == "<amplitude>") {
	  std::istringstream s(line.substr(11));
	  s >> ampli;
	} else if (line.substr(0,7) == "<ampli>") {
	  std::istringstream s(line.substr(7));
	  s >> ampli;
	     
	} else if (line.substr(0,8) == "<source>") {
	  getline(is, line);
	  VEC2 pos;
	  while (line.substr(0,9) != "</source>") {
	    if  (line.substr(0,5) == "<pos>") {
	      std::istringstream s(line.substr(6));
	      s >> pos(0) >> pos(1);
	    } else {
          ERROR(false, "Invalid configuration file (source wave)\""<<file, line);
	    }
	    getline(is, line);
	  }
	  addEqSource(pos(0), pos(1), wl, ampli);
	} else if (line.substr(0,12) == "<source_all>") {
	  getline(is, line);
	  VEC2 pos;
	  while (line.substr(0,13) != "</source_all>") {
	    if  (line.substr(0,5) == "<pos>") {
	      std::istringstream s(line.substr(6));
	      s >> pos(0) >> pos(1);
	    } else {
          ERROR(false, "Invalid configuration file (spurce all wave)\""<<file, line);
	    }
	    getline(is, line);
	  }
	  for (int i = 0; i < nb_wl; ++i) {
	    EquivalentSource *w = new EquivalentSource(wave_lenghts[i]);
	    w->setPos(pos);
	    w->setAmplitude(ampli);
	    waves[i].push_back(w);
	  }
	} else {
      ERROR(false, "Invalid configuration file (wave)\""<<file, line);
	}
	getline(is, line);
      }

     } else  if (line.substr(0,9) == "<pattern>") { //define patterns
       getline(is, line);
       std::string name;
       int nc = 200;
       int nr = 200;
       FLOAT cs = 0.15;
       FLOAT x = 15, y = 15;
       FLOAT radius = 15;
       FLOAT iv = 1;
       FLOAT ampli = 0;
       uint m = 0;
       while (line.substr(0,10) != "</pattern>") {
     	if  (line.substr(0,6) == "<file>") {
     	  file = line.substr(7);
     	} else if (line.substr(0,6) == "<size>") {
     	  std::istringstream s(line.substr(6));
     	  s >> nr >> nc;
     	} else if (line.substr(0,11) == "<cell_size>") {
     	  std::istringstream s(line.substr(11));
     	  s >> cs;
	} else if  (line.substr(0,5) == "<pos>") {
	  std::istringstream s(line.substr(5));
	  s >> x >> y;
	} else if (line.substr(0,8) == "<radius>") {
	  std::istringstream s(line.substr(8));
	  s >> radius;
	} else if (line.substr(0,7) == "<ampli>") {
	  std::istringstream s(line.substr(7));
	  s >> ampli;
	} else if (line.substr(0,6) == "<init>") {
	  std::istringstream s(line.substr(6));
	  s >> iv;
	} else if (line.substr(0,8) == "<method>") {
	  std::istringstream s(line.substr(8));
	  s >> m;
	  INFO("Metode  "<<m);
	} else {
          ERROR(false, "Invalid configuration file (pattern)\""<<file, line);
     	}
     	getline(is, line);
       }
    }
    else if (line.substr(0,14) == "<load_texture>") {
        settings::doLoadTexture = !settings::doLoadTexture;
    }
    else {
        ERROR(false, "Invalid configuration file \""<<file, line);
    }
  }
  is.close();
}



void WaterSurface::setImport(std::string file) {
  import_ = true;
  import_file = file;
}

void WaterSurface::setExport(std::string file) {
  export_ = true;
  export_file = file;
}

void WaterSurface::setImportConf(std::string file) {
  load_conf = true;
  conf_file = file;
}

void WaterSurface::setStopTime(int end) {
  stop_time = end;
}

void WaterSurface::drawHeighField(std::string file) {
  std::ofstream  out_file;
  out_file.open(file);
  INFO("Exporting "<<file);
  out_file<<0<<" "<<0<<" "<<0.25*height_ampli_<<"\n";
  for (int i = 0; i < n_rows_; ++i) {
    for (int j = 0; j < n_cols_; ++j) {
      FLOAT a = u(i, j);
      if (a < -0.25*height_ampli_) {
	a = -0.25*height_ampli_;
      }
      if (a > 0.25*height_ampli_) {
	a = 0.25*height_ampli_;
      }
      if (i != 0 && j != 0) {
	out_file<<i<<" "<<j<<" "<<a<<"\n";
      }
    }
    out_file<<"\n";
  }
  out_file.close();
}

VEC3 WaterSurface::getPosGrid(int i, int j) const {
  VEC2 pv = grid2viewer(i, j);
  return VEC3(pv(0), pv(1), u(i, j));
}

VEC3 WaterSurface::getPosGrid(int i) const {
  VEC2 pv = grid2viewer(i/n_cols_, i - (int)(i/n_cols_)*n_cols_);
  return VEC3(pv(0), pv(1), u(i/n_cols_, i - (int)(i/n_cols_)*n_cols_));
}

FLOAT WaterSurface::minWL() const {
    return wave_lenghts[0];
}
FLOAT WaterSurface::maxWL() const {
    return wave_lenghts[nb_wl-1];
}

std::list<EquivalentSource*> WaterSurface::getSourceList() {
    return waves[0];
}

void WaterSurface::addConstPoint(VEC3 pos){
    constraintsPos.push_back(pos);
}
