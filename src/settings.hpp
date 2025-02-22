/* 
 * File: setting.hpp
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

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

//#define INTERACTIVE_
//#define USE_CUDA
//#define PROJECTED_GRID // Note: only implemented with the cuda version
#define PLOT_RESULT //Note: only for one frequency at the time, without projective grid

#include "definitions.hpp"
#include <vector>
#include <string>

#define NTHREADS_ 22

namespace settings {

  extern bool doLoadTexture;
  extern FLOAT dt_; //second

  // Grid spec
  extern int n_rows_, n_cols_;
  extern FLOAT cell_size_; //meter
  extern FLOAT scale_; 

  extern FLOAT gravity_;
  extern FLOAT h_;
  
  extern FLOAT damping_;
  FLOAT damping(FLOAT r, FLOAT k);

  // dispersion relation
  FLOAT angular_vel(FLOAT k);
  FLOAT velocity(FLOAT k);

  extern FLOAT step_sampling_;

  extern FLOAT init_wl_;
  extern FLOAT height_ampli_;


  VEC2 grid2viewer(int i, int j);
  VEC2 gridObs2viewer(int i, int j);

  Vector2i world2grid(VEC2 p);
  
  VEC2 grid2world(Vector2i p);
  VEC2 grid2world(int i, int j);
    
  VEC2 world2viewer(FLOAT x, FLOAT y);
  VEC2 world2viewer(VEC2 p);

  VEC2 viewer2world(VEC2 p);
  VEC2 viewer2world(FLOAT x, FLOAT y);


  FLOAT interpolation(FLOAT t, int l, FLOAT dt);


COMPLEX Hankel(FLOAT x);
  COMPLEX derHankel(FLOAT x);
  COMPLEX derderHankel(FLOAT x);
  
  COMPLEX addWaves(FLOAT x0);
  void createTabs();

  
  extern std::vector<COMPLEX> hankel_tab;
  //profil buffer
  extern int nb_profil;
  extern FLOAT step_profil;

};

#endif
