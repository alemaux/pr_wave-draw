/* 
 * File: EquivalentSource.hpp
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

#ifndef EQUIVALENTSOURCE_HPP
#define EQUIVALENTSOURCE_HPP

#include "Wave.hpp"
#include <vector>


// inline COMPLEX Hankel(FLOAT x);
// inline COMPLEX derHankel(FLOAT x);

class EquivalentSource: public Wave {

protected:
  FLOAT wave_length;
  FLOAT wave_number;
  FLOAT angular_freq;
  VEC2 pos;
  COMPLEX ampli;
  
public:
  EquivalentSource();
  EquivalentSource(EquivalentSource* es);
  EquivalentSource(FLOAT wl);
  ~EquivalentSource();

  void reset();  
  
  virtual FLOAT height(FLOAT x, FLOAT y, FLOAT time) const;
  virtual FLOAT height(VEC2 p, FLOAT time) const;
  virtual COMPLEX heightc(FLOAT x, FLOAT y, FLOAT time) const;
  virtual COMPLEX heightc(VEC2 p, FLOAT time) const;
  COMPLEX heightc(VEC2 p) const;
  COMPLEX heightc(FLOAT x, FLOAT y) const;
  

  virtual VEC2C gradHeightc(FLOAT x, FLOAT y, FLOAT time) const;
  virtual VEC2C gradHeightc(VEC2 p, FLOAT time) const;
  VEC2C gradHeightc(FLOAT x, FLOAT y) const;
  VEC2C gradHeightc(VEC2 p) const;
  MAT2C hessHeightc(FLOAT x, FLOAT y) const;
 
  void setPos(VEC2 p);
  void setPos(FLOAT x, FLOAT y);

  void setAmplitude(COMPLEX a);

  VEC2 getPos() const;
  FLOAT getDistance(FLOAT x, FLOAT y) const;
  FLOAT getDistance(VEC2 p) const;
  int getIndex() const;
  
  COMPLEX getAmpli() const;
};

#endif
