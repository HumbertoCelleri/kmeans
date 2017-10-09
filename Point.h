#ifndef POINT_H
#define POINT_H

#include <vector>
#include <math.h>

/*
* Point object with coord[]: coordinates vector components
*/

class Punto  {
public:
    vector<double> _coords;
    int _dim;
    int _zona;

    // Constructor
    Punto(){
    };

    /**
    * @param[in]: dim: point dimension
    */
    Punto(int dim){ 
        _coords.resize(_dim, 0);
        _dim = dim;
        _zona = 999; // Fuera de rango
        this->Fill();
        }

    /**
    * @param[in]: dim: point dimension
    * @param[in]: coords: vector<double> with coordinates
    */
    Punto(int dim, vector<double> coords){ 
        _coords.resize(_dim, 0);
        _dim = dim;
        _coords = coords;
        _zona = 999; // Fuera de rango
        }

    void Fill(){
        for(int i=0; i<_dim; i++){
            _coords[i] = rand() % 1; // Entre 0 y 1
        }
    }

    /** Compute distance from thisPoint to center.
    *
    */
    double distancia(Punto center){
        double distancia = 0;
            for (int i = 0; i < _dim; i++)
            {
                distancia += (_coords[i] - center._coords[i])*(_coords[i] - center._coords[i]);
            }
            distancia = sqrt(distancia);
        return distancia;
    }

     
    void pup(PUP::er &p){
        p | _coords;
        p | _dim;
        p | _zona;
    }

};

#endif
