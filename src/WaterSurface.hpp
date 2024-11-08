/* 
 * File: WaterSurface.hpp
 *
 * Copyright (C) 2019  Camille Schreck
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

#ifndef WATERSURFACE_HPP
#define WATERSURFACE_HPP

#include <list>

#include "definitions.hpp"
#include "Wave.hpp"
#include "Grid.hpp"
#include "ProjectedGrid.hpp"
#include "Sphere.hpp"

#include <thread>

class WaterSurface {
public:
  bool draw_sources;

  WaterSurface();
  ~WaterSurface();

  void clear();
  void reset();
  
  void setLists();
  void setAmpli();
  void setAmpli(FLOAT t);

  FLOAT height(int i, int j) const;
  void addEqSource(FLOAT x, FLOAT y, FLOAT wl = 0, COMPLEX ampli = COMPLEX(1, 0));

  void update();
  friend void updateSourcesAmplis(WaterSurface *ws);
  void updateHeight();

  void draw();

  void exportAmplitude(std::string file) const;
  void exportAmplitudeIm(std::string file) const;
  void exportAmplitudeRe(std::string file) const;
  void exportPhase(std::string file) const;

  void exportMitsuba(std::string file) const;
  void exportSurfaceTime(std::string file) const;
  void importSurfaceTime(std::string file);
  void importConfig(std::string file);

  void setImport(std::string file);
  void setExport(std::string file);
  void setImportConf(std::string file);
  void setStopTime(int end);
  void drawHeighField(std::string file);

// #ifdef PROJECTED_GRID
//   void setProjGrid(int i, int j, FLOAT x, FLOAT y);
//   VEC3 getPosProjGrid(int i, int j) const;
//   VEC3 getPosProjGrid(int i) const;

//   void setTargetLookAt(VEC2 target);
// #endif
  VEC3 getPosGrid(int i, int j) const;
  VEC3 getPosGrid(int i) const;

  FLOAT minWL() const;
  FLOAT maxWL() const;

private:
  FLOAT step_wl;
  FLOAT min_wl, max_wl;
  int nb_wl;
  
  Grid u;
  Grid pattern;
  
  std::vector<Grid> ampli_re;
  std::vector<Grid> ampli_im;

  int time;

  std::vector<std::list<Wave*> > waves;
  std::vector<FLOAT> wave_lenghts;

  bool import_;
  bool export_;
  bool load_conf;

  
  std::string import_file;
  std::string export_file;
  std::string conf_file;
  std::string data_file;

  int stop_time;
  
  //  ProjectedGrid proj_grid;
  std::thread t_solve;

  VEC2 target_lookat;

  Sphere sphere_source;
  Sphere sphere_bp;
  Sphere sphere_pp;
  Sphere sphere_pp2;

};



#endif
