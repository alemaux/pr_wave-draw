/* 
 * File: settings.cpp
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

#include "settings.hpp"
#include "error.hpp"
#include <boost/math/special_functions/bessel.hpp>

using namespace definitions;

namespace settings {

    bool doLoadTexture = true;
    FLOAT dt_ = 0.03;

    int n_rows_ = 150;
    int n_cols_ = 150;
    FLOAT cell_size_ = 0.2;

    FLOAT damping_ = 0.00;

    FLOAT gravity_ = 9.81;

    FLOAT step_sampling_ = 0.25;

    FLOAT init_wl_ = 1.2;
    FLOAT height_ampli_ = 1;

    FLOAT scale_ = 30;

    std::vector<COMPLEX> hankel_tab;
    //profil buffer
    int nb_profil = 100000;
    FLOAT step_profil = 0.025;

    FLOAT damping(FLOAT r, FLOAT k) {
        return exp(-damping_*k*k*r);
    }

    // dispersion relation
    FLOAT angular_vel(FLOAT k) {
        return sqrtf(gravity_*k + 0.074/1000*pow(k, 3));//*tanh(k*h_));
    }

    FLOAT velocity(FLOAT k) {
        return 0.5*angular_vel(k) / k;
    }

    VEC2 grid2viewer(int i, int j) {
        return VEC2(2*i/(FLOAT)n_rows_ - 1, 2*(FLOAT)j/(FLOAT)n_cols_ - 1);
    }


    Vector2i world2grid(VEC2 p) {
        return Vector2i(round(p(0)/cell_size_), round(p(1)/cell_size_));
    };

    VEC2 grid2world(Vector2i p) {
        return VEC2(p(0)*cell_size_, p(1)*cell_size_);
    };

    VEC2 grid2world(int i, int j) {
        return VEC2((FLOAT)i*cell_size_, (FLOAT)j*cell_size_);
    };


    VEC2 world2viewer(FLOAT x, FLOAT y) {
        return VEC2(2*x/cell_size_/n_rows_ - 1, 2*y/cell_size_/n_cols_ - 1);
    }

    VEC2 world2viewer(VEC2 p) {
        return VEC2(2*p(0)/cell_size_/n_rows_ - 1, 2*p(1)/cell_size_/n_cols_ - 1);
    }

    VEC2 viewer2world(VEC2 p) {
        return VEC2((p(0) + 1)*n_rows_*cell_size_/2.0, (p(1) + 1)*n_cols_*cell_size_/2.0);
    }

    VEC2 viewer2world(FLOAT x, FLOAT y) {
        return VEC2((x + 1)*n_rows_*cell_size_/2.0, (y + 1)*n_cols_*cell_size_/2.0);
    }


    FLOAT interpolation(FLOAT t, int l, FLOAT dt) {
        FLOAT tl = l*dt, tl_prev = (l-1)*dt, tl_next = (l+1)*dt;
        FLOAT out = 0;
        if (t < tl_prev || t > tl_next) {
            out = 0;
        }else if (t < tl) {
            out = (t - tl_prev)/dt;
        }else {
            out = (tl_next - t)/dt;
        }
        return out;
    }


    COMPLEX Hankel(FLOAT x) {
        // return boost::math::cyl_bessel_j<FLOAT, FLOAT>(0, x) -
        //   std::complex<FLOAT>(0, 1)*boost::math::cyl_neumann<FLOAT, FLOAT>(0, x);
        return sqrtf((FLOAT)2.0/((FLOAT)M_PI*x))*exp(i_*(x - (FLOAT)(M_PI/4.0)));
    }

    COMPLEX derHankel(FLOAT x) {
        // return 0.5f*(boost::math::cyl_bessel_j<FLOAT, FLOAT>(-1, x) -
        // 	       std::complex<FLOAT>(0, 1)*boost::math::cyl_neumann<FLOAT, FLOAT>(-1, x) -
        // 	       boost::math::cyl_bessel_j<FLOAT, FLOAT>(1, x) +
        // 	       std::complex<FLOAT>(0, 1)*boost::math::cyl_neumann<FLOAT, FLOAT>(1, x));
        return sqrtf((FLOAT)2.0/((FLOAT)M_PI*x))*(-1.0f/(2.0f*x) + i_)*exp(i_*(x - (FLOAT)(M_PI/4.0)));
    }

    COMPLEX derderHankel(FLOAT x) {
#ifdef DOUBLE_PRECISION
        return sqrtf((FLOAT)2.0/(FLOAT)M_PI)*(3.0f/4.0f*powf(x, -2.5) - i_*pow(x, -1.5) - powf(x, -0.5))*exp(i_*(x - (FLOAT)(M_PI/4.0)));
#else
        return sqrtf((FLOAT)2.0/(FLOAT)M_PI)*(3.0f/4.0f*powf(x, -2.5) - i_*powf(x, -1.5) - powf(x, -0.5))*exp(i_*(x - (FLOAT)(M_PI/4.0)));
#endif
    }


    COMPLEX addWaves(FLOAT x0) {
        // int ind = floor(x0/0.025);
        // FLOAT coef = x0/0.025 - ind;
        // if (ind >= 999999) {
        //   ind = 0;
        //   coef = 0;
        // }
        // COMPLEX out = (1-coef)*hankel_tab[ind] + coef*hankel_tab[ind+1];

        COMPLEX out = (-i_)/(FLOAT)4.0f*Hankel(x0);
        return out;
    }



    void createTabs() {
        hankel_tab = std::vector<COMPLEX>(nb_profil);
        hankel_tab[0] = 0;

        for (int i = 1; i < nb_profil; ++i) {
            FLOAT x0 = (FLOAT)i*step_profil;
            hankel_tab[i] = (-i_)/(FLOAT)4.0*Hankel(x0);
        }
    }


}
