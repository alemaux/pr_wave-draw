#include "wavedraw.hpp"
#include <vector>
#include<math.h>
#include "settings.hpp"


std::vector<VEC2> WaveDraw::positionSources(int nb_sources){
    //définition du cercle ou posiitonner les sources équivalentes
    VEC2 center = VEC2(settings::n_rows_ / 2, settings::n_cols_ /2);
    int rayonV = 0.45 * settings::n_rows_;
    int rayonH = 0.45 *settings::n_cols_;
    std::vector<VEC2> positions;

    for(double theta = 0; theta < 2 * M_PI; theta += 2*M_PI/nb_sources){
        double r = (cos(theta)/rayonH)*(cos(theta)/rayonH) + (sin(theta)/rayonV)*(sin(theta)/rayonV);
        positions.push_back(VEC2((static_cast<int>(center.x() + cos(theta)/r), static_cast<int>(center.y() + sin(theta)/r))));
    }
    return positions;
}

std::vector<COMPLEX> WaveDraw::invertMatrice(std::vector<EquivalentSource> sources, std::vector<VEC2> sample_points) {

}



