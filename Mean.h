#ifndef MEAN_H
#define MEAN_H

#include <vector>
#include <math.h>

/*
* Point that acts as sum container of the points coordinates
*/

class Mean : public Punto  {
public:

    int nbr_points_in_zone; // Variable to count number of points in zone

    Mean(){
        nbr_points_in_zone = 0;
    }

    Mean(int dim) : Punto(dim){
        nbr_points_in_zone = 0;
    }

    Mean(int dim, vector<double> coords) : Punto(dim, coords){
        nbr_points_in_zone = 0;
    }

    /** Function to add coordinate to coordinate a new point
    *
    */
    void suma(Punto seg_punto){
        
        nbr_points_in_zone += 1;

        Punto result();

        for (int i = 0; i < _dim; i++)
        {
            _coords[i] = (_coords[i] + seg_punto._coords[i]);
        }

        // Chequeamos que sea de la misma zona
        if (_zona == seg_punto._zona){
            result._zona = seg_punto._zona;    
        } else {
            CkPrintf("Estas sumando dos puntos de distintas zonas!!\n");
            CkExit();
        }
    }

    /** Function to get mean after iteration
    *
    */
    Mean get_mean(){
        
        vector<double> _new_coords;
        _new_coords.resize(_dim, 0);

        for (int i = 0; i <_dim; i++){
            _new_coords[i] = _coords[i]/nbr_points_in_zone;
        }

        Mean result(_dim, _new_coords);

        return result;
    }

    void pup(PUP::er &p){
        p | _coords;
        p | _dim;
        p | _zona;
        p | _nbr_points_in_zone;
        
    }


};

#endif
