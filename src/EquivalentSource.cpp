/* 
 * File: EquivalentSource.cpp
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

#include "EquivalentSource.hpp"
#include <boost/math/special_functions/bessel.hpp>
#include <iostream>
#include "settings.hpp"
#include "error.hpp"

using namespace definitions;
using namespace settings;


EquivalentSource::EquivalentSource(): Wave() {
  wave_length = 1;
  reset();
}

EquivalentSource::EquivalentSource(EquivalentSource* es): Wave() {
  wave_length = es->wave_length;
  reset();
  pos = es->pos;
}

EquivalentSource::EquivalentSource(FLOAT wl): Wave() {
  wave_length = wl;
  reset();
}


EquivalentSource::~EquivalentSource() {}

void EquivalentSource::reset() {
  wave_number = 2*M_PI/wave_length;
  angular_freq = angular_vel(wave_number);
  pos = VEC2(0, 0);
  ampli = 1;
}


FLOAT EquivalentSource::height(FLOAT x, FLOAT y, FLOAT time) const{
  COMPLEX ampli_cur = ampli;
  FLOAT rx = x - pos(0);
  FLOAT ry = y - pos(1);
  FLOAT r  = sqrt(pow(rx, 2.0) + pow(ry, 2.0));
  FLOAT damp = damping(r, wave_number);
  FLOAT out = 0;
  if (damp > 0.02) {
    if (r != 0) {
      out = damp*real(addWaves(wave_number*r)*ampli_cur);
    }
  }
  return out;
}

FLOAT EquivalentSource::height(VEC2 p, FLOAT time) const{
  return height(p(0), p(1), time);
}

  
COMPLEX EquivalentSource::heightc(VEC2 p, FLOAT time) const {
  return heightc(p(0), p(1), time);
}


COMPLEX EquivalentSource::heightc(FLOAT x, FLOAT y, FLOAT time) const {
    COMPLEX ampli_cur = ampli;
  FLOAT rx = x - pos(0);
  FLOAT ry = y - pos(1);
  FLOAT r  = sqrt(pow(rx, 2.0) + pow(ry, 2.0));
  FLOAT damp = damping(r, wave_number);
  COMPLEX out(0, 0);
  if (damp > 0.02) {
    if (r != 0) {
      out = damp*addWaves(wave_number*r)*ampli_cur;
    }
  }
  return out;
  
}


COMPLEX EquivalentSource::heightc(VEC2 p) const{
  return heightc(p(0), p(1));
}

COMPLEX EquivalentSource::heightc(FLOAT x, FLOAT y) const {
  FLOAT rx = x - pos(0);
  FLOAT ry = y - pos(1);
  FLOAT r  = sqrt(pow(rx, 2.0) + pow(ry, 2.0));
  FLOAT damp = damping(r, wave_number);
  COMPLEX out(0, 0);
  if (damp > 0.02) {
    if (r != 0) {
      out = damp*addWaves(wave_number*r)*ampli;
    }
  }
  return out;
}



VEC2C EquivalentSource::gradHeightc(FLOAT x, FLOAT y, FLOAT time) const {
  COMPLEX ampli_cur = ampli;
  FLOAT rx = x - pos(0);
  FLOAT ry = y - pos(1);
  FLOAT r  = sqrt(pow(rx, 2.0) + pow(ry, 2.0));
  COMPLEX out_x(0, 0);
  COMPLEX out_y(0, 0);
  FLOAT damp = damping(r, wave_number);
  if (damp > 0.02) {
    if (r != 0) {
      FLOAT der_damp = 0;//exp(-damping*pow(wave_number, 2)*r);
      FLOAT cos_phi = rx/r;
      FLOAT sin_phi = ry/r;
      out_x = cos_phi*damp*(-i_/(FLOAT)4.0*wave_number*derHankel(wave_number*r)*ampli_cur) -
	sin_phi/r*der_damp*(-i_/(FLOAT)4.0*Hankel(wave_number*r)*ampli_cur);
      out_y = sin_phi*damp*(-i_/(FLOAT)4.0*wave_number*derHankel(wave_number*r)*ampli_cur) +
	cos_phi/r*der_damp*(-i_/(FLOAT)4.0*Hankel(wave_number*r)*ampli_cur);
    }
  }
  return VEC2C(out_x, out_y);
}

VEC2C EquivalentSource::gradHeightc(FLOAT x, FLOAT y) const {
  FLOAT rx = x - pos(0);
  FLOAT ry = y - pos(1);
  FLOAT r  = sqrt(pow(rx, 2.0) + pow(ry, 2.0));
  COMPLEX out_x(0, 0);
  COMPLEX out_y(0, 0);
  FLOAT damp = damping(r, wave_number);
  if (damp > 0.02) {
    if (r != 0) {
      FLOAT der_damp = 0;//damping(r, wave_number);//exp(-damping*pow(wave_number, 2)*r);
      FLOAT cos_phi = rx/r;
      FLOAT sin_phi = ry/r;
      out_x = cos_phi*damp* (-i_/(FLOAT)4.0*wave_number*derHankel(wave_number*r)) -
	sin_phi/r*der_damp*(-i_/(FLOAT)4.0*Hankel(wave_number*r));
      out_y = sin_phi*damp* (-i_/(FLOAT)4.0*wave_number*derHankel(wave_number*r)) +
	cos_phi/r*der_damp*(-i_/(FLOAT)4.0*Hankel(wave_number*r));
    }
  }
  return VEC2C(out_x, out_y);
}

MAT2C EquivalentSource::hessHeightc(FLOAT x, FLOAT y) const {
  FLOAT rx = x - pos(0);
  FLOAT ry = y - pos(1);
  FLOAT r  = sqrt(pow(rx, 2.0) + pow(ry, 2.0));
  MAT2C out;
  if (r != 0) {
    FLOAT cos_phi = rx/r;
    FLOAT sin_phi = ry/r;
    out(0,0) = cos_phi*cos_phi*(-i_/(FLOAT)4.0*wave_number*wave_number*derderHankel(wave_number*r));
    out(1,1) = sin_phi*sin_phi*(-i_/(FLOAT)4.0*wave_number*wave_number*derderHankel(wave_number*r));
    out(0,1) = cos_phi*sin_phi*(-i_/(FLOAT)4.0*wave_number*wave_number*derderHankel(wave_number*r));
    out(1,1) = sin_phi*cos_phi*(-i_/(FLOAT)4.0*wave_number*wave_number*derderHankel(wave_number*r));
  }
  return out;
}



VEC2C EquivalentSource::gradHeightc(VEC2 p, FLOAT time) const {
  return gradHeightc(p(0), p(1), time);
}


VEC2C EquivalentSource::gradHeightc(VEC2 p) const {
  return gradHeightc(p(0), p(1));
}


void EquivalentSource::setPos(VEC2 p) {
  pos = p;
}

void EquivalentSource::setPos(FLOAT x, FLOAT y) {
  pos = VEC2(x, y);
}

void EquivalentSource::setAmplitude(COMPLEX a) {
  ampli = a;
}

VEC2 EquivalentSource::getPos() const {
  return pos;
}

FLOAT EquivalentSource::getDistance(FLOAT x, FLOAT y) const {
  VEC2 v = pos - VEC2(x, y);
  return v.norm();
}

FLOAT EquivalentSource::getDistance(VEC2 p) const {
  VEC2 v = pos - p;
  return v.norm();
}

COMPLEX EquivalentSource::getAmpli() const {
  return ampli;
}

int EquivalentSource::getIndex() const {
  return 0;
}

FLOAT EquivalentSource::getWL() const {
    return wave_length;
}


FLOAT EquivalentSource::getDamping(VEC2 p) const{
    FLOAT rx = p[0] - pos(0);
    FLOAT ry = p[1] - pos(1);
    FLOAT r  = sqrt(pow(rx, 2.0) + pow(ry, 2.0));
    FLOAT damp = damping(r, wave_number);
    return damp;
}
