#ifndef WAVEDRAW_HPP
#define WAVEDRAW_HPP

#include "definitions.hpp"
#include "settings.hpp"
#include "EquivalentSource.hpp"


class WaveDraw
{
public:
    std::vector<VEC2> positionSources(int nb_sources);
    void setAmplitudeFromPoint(EquivalentSource *es, VEC2 point, FLOAT desired_height);
private:

};

#endif // WAVEDRAW_HPP
