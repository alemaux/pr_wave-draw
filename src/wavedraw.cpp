#include "wavedraw.hpp"
#include "settings.hpp"
#include "error.hpp"

#include <vector>
#include<math.h>


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

//std::vector<COMPLEX> WaveDraw::invertMatrice(std::vector<EquivalentSource> sources, std::vector<VEC2> sample_points) {
//    int n = sources.size(), p = sample_points.size();
//    //MAT2C matrice()
//}
//
//std::vector<FLOAT> static WaveDraw::getEquationFromPoint(VEC2 pos, std::vector<EquivalentSource> sources){
//    std::vector<FLOAT> coefs(sources.size());
//    for(EquivalentSource source : sources){
//        coefs.push_back(source.height(pos, 0f));
//    }
//    return coefs;
//}

void WaveDraw::setSinglePointtoHeight(VEC2 pos, float height, EquivalentSource* source){
    float actualHeight = (*source).height(pos, 0);
    source->setAmplitude(height / actualHeight);
}
