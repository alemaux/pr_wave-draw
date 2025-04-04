#include "wavedraw.hpp"
#include "settings.hpp"
#include "error.hpp"

#include <vector>
#include <math.h>
#include <Eigen/Dense>

std::vector<VEC2> WaveDraw::positionSources(int nb_sources){
    //définition du cercle ou posiitonner les sources équivalentes
    VEC2 center = VEC2(settings::n_rows_ / 2, settings::n_cols_ /2);
    int rayonV = 0.45 * settings::n_rows_;
    int rayonH = 0.45 *settings::n_cols_;
    std::vector<VEC2> positions;

    for(double k = 0; k < nb_sources; k++){
        const float theta = 2 * M_PI * k/nb_sources;
        //double r = pow(cos(theta)*rayonH, 2) + pow(sin(theta)*rayonV, 2);
        int x = round(center.x() + rayonH * cos(theta));
        int y = round(center.y() + rayonV * sin(theta));
        positions.push_back(VEC2(x * settings::cell_size_, y*settings::cell_size_));
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

void WaveDraw::setSinglePointtoHeight(VEC3 constr, VEC2 sourcePos, WaterSurface* ws){
    EquivalentSource eq = new EquivalentSource(settings::init_wl_);
    eq.setPos(sourcePos);
    VEC2 pos = VEC2(constr.x(), constr.y());

    COMPLEX actualHeight = eq.heightc(pos);
    COMPLEX newAmpli = constr.z() / actualHeight ;

    std::cout<<"Position : "<<pos<<std::endl;
    std::cout<<"Height : "<<actualHeight<<std::endl;
    std::cout<<"Nouvelle Ampli : "<<newAmpli<<std::endl;

    EquivalentSource* es;
    if(std::abs(actualHeight) > 1e-4) {
        es = (*ws).addSingleSource(sourcePos.x(), sourcePos.y(), settings::init_wl_, newAmpli);
    }
    else {
        std::cout<<"hauteur trop petite : "<<actualHeight<<std::endl;
        es = (*ws).addSingleSource(sourcePos.x(), sourcePos.y(), settings::init_wl_, COMPLEX(1,0));
    }
    std::cout<<"source height : "<<es->heightc(pos)<<std::endl;

//Ajout de la contrainte dans la liste
//    --> uniquement utile si la liste des contraintes n'est pas définie ailleurs
    (*ws).addConstPoint(constr);
}



void WaveDraw::setAmplisFromConstr(WaterSurface *ws){
    std::vector<VEC3> constraints = ws->getConstrPoints();
    ws->refreshHeight();

    //for(auto it = constraints.begin(); it!= constraints.end(); it++){
    //    std::cout<<"Constraints : "<<it->x()<<" : "<<it->y()<<" : "<<it->z()<<std::endl;
    //}

    int nbConst = constraints.size();
    assert(nbConst == 2);
    std::vector<VEC2> posSources = positionSources(nbConst);
    Eigen::Matrix2cf srcHeights(nbConst, nbConst);
    Eigen::Vector2cf constrHeights(nbConst);

    EquivalentSource eq = new EquivalentSource(settings::init_wl_);
    for(int is = 0; is<nbConst; is++){
        VEC2 pos = posSources[is];
        eq.setPos(pos.x() * settings::cell_size_, pos.y()*settings::cell_size_);
        int ic = 0;
        for(auto it = constraints.begin(); it != constraints.end(); ++ic, ++it){
            //srcHeights(ic, is) = ws->height(it->x() / settings::cell_size_, it->y()/settings::cell_size_);
            srcHeights(ic, is) = eq.heightc(it->x(), it->y());
            std::cout<<"contrainte : "<<ic<<", source "<<is<<" : "<<srcHeights(ic, is)<<std::endl;
        }
    }

    for(int i = 0; i<constraints.size(); i++){
        constrHeights(i) = constraints[i].z();
    }

    Eigen::Vector2cf x = srcHeights.lu().solve(constrHeights);

    COMPLEX DebugHeights[2] = {COMPLEX(0,0), COMPLEX(0,0)};

    for(int j = 0; j<nbConst; j++){
        std::cout<<std::endl;
        //std::cout<<"AMPLITUDE "<<j<<" : "<<x[j]<<std::endl;
        EquivalentSource *es = ws->addSingleSource(posSources[j].x(), posSources[j].y(), settings::init_wl_, x[j]);

        std::cout<<x[j]<<" : "<<srcHeights(0, j) * x[j]<<std::endl;
        std::cout<<"hauteur source "<<j<<" pos 1 : "<<es->heightc(constraints[0].x(), constraints[0].y())<<std::endl;
        std::cout<<x[j]<<" "<<srcHeights(1, j) * x[j]<<std::endl;
        std::cout<<"hauteur source "<<j<<" pos 2 : "<<es->heightc(constraints[1].x(), constraints[1].y())<<std::endl;

        DebugHeights[0] += (es->heightc(constraints[0].x(), constraints[0].y()));
        DebugHeights[1] += (es->heightc(constraints[1].x(), constraints[1].y()));
    }
    std::cout<<std::endl;
    std::cout<<"debug source height : pos1"<<DebugHeights[0]<<", pos2"<<DebugHeights[1]<<std::endl;


    ws->refreshHeight();
    std::cout<<"New height (surface-wise) : "<<ws->height(constraints[0].x()/ settings::cell_size_, constraints[0].y()/settings::cell_size_)<<std::endl;
    std::cout<<"New height (surface-wise) : "<<ws->height(constraints[1].x()/ settings::cell_size_, constraints[1].y()/settings::cell_size_)<<std::endl;

    std::cout<<"CALCUL : "<<srcHeights(0,0) * x[0] + srcHeights(0,1)*x[1]<<std::endl;
    std::cout<<"CALCUL : "<<srcHeights(1,0) * x[0] + srcHeights(1,1)*x[1]<<std::endl;

}

FLOAT WaveDraw::evaluateSolution(WaterSurface* ws){
    FLOAT error = 0;
    std::vector<VEC3> constraints = (*ws).getConstrPoints();
    for(VEC3 pos : constraints){
        FLOAT surfaceHeight = (*ws).height(pos.x() / settings::cell_size_,pos.y()/settings::cell_size_);
        FLOAT constrHeight = pos.z();
        std::cout<<"**********"<<std::endl;
        std::cout<<"Hauteur de la surface : "<<surfaceHeight<<std::endl;
        std::cout<<"Hauteur souhaitée : "<<constrHeight<<std::endl;
        std::cout<<"**********"<<std::endl;
        error += pow(surfaceHeight - constrHeight, 2);
    }
    std::cout<<"Erreur de la solution : " <<error<<std::endl;
    return error;
}

void WaveDraw::test(WaterSurface* ws){
    VEC2 center = VEC2(settings::n_rows_ * settings::cell_size_ /2, settings::n_cols_ * settings::cell_size_ /2);
    EquivalentSource *eq1 = ws->addSingleSource(0.9*center.x(), center.y(), settings::init_wl_, COMPLEX(1, 0));
    EquivalentSource *eq2 = ws->addSingleSource(1.1*center.x(), center.y(), settings::init_wl_, COMPLEX(1, 0));

    ws->refreshHeight();
    VEC2 testPoint = VEC2(1.2*center.x(), 1.2*center.y());
    COMPLEX h1 = eq1->heightc(testPoint);
    COMPLEX h2 = eq2->heightc(testPoint);
    std::cout<<"Somme des hauteurs : "<<h1 + h2<<std::endl;
    std::cout<<"hauteur de la surface : "<<ws->height(testPoint.x() / settings::cell_size_, testPoint.y() / settings::cell_size_)<<std::endl;
    COMPLEX amp1 = COMPLEX(2.0, 1.0);
    COMPLEX amp2 = COMPLEX(5.0, 5.0);
    eq1->setAmplitude(amp1);
    eq2->setAmplitude(amp2);
    ws->refreshHeight();
    std::cout<< eq1->heightc(testPoint) <<" = "<<amp1*h1<<std::endl;
    std::cout<< eq2->heightc(testPoint) <<" = "<<amp2*h2<<std::endl;
    std::cout<<"Somme des produits : "<<amp1*h1 + amp2*h2<<std::endl;
    std::cout<<"hauteur de la surface : "<<ws->height(testPoint.x() / settings::cell_size_, testPoint.y() / settings::cell_size_)<<std::endl;

    ws->reset();
    EquivalentSource *eq3 = ws->addSingleSource(0.9*center.x(), center.y(), settings::init_wl_, amp1);
    EquivalentSource *eq4 = ws->addSingleSource(1.1*center.x(), center.y(), settings::init_wl_, amp2);
    ws->refreshHeight();
    std::cout<< eq3->heightc(testPoint) <<" = "<<amp1*h1<<std::endl;
    std::cout<< eq4->heightc(testPoint) <<" = "<<amp2*h2<<std::endl;
}


