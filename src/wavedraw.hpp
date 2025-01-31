#ifndef WAVEDRAW_HPP
#define WAVEDRAW_HPP

#include "definitions.hpp"
#include "settings.hpp"
#include "EquivalentSource.hpp"


class WaveDraw
{
public:
    std::vector<VEC2> positionSources(int nb_sources);
    static void setSinglePointtoHeight(VEC2 pos, float height, EquivalentSource* source);
private:

};

#endif // WAVEDRAW_HPP
