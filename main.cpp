#include <vector>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "charm++.h"
#include "main.decl.h"
#include "Point.h"
#include "Mean.h"

using namespace std;

/*readonly*/ CProxy_Main mainProxy;
/*readonly*/ CProxy_Main containerProxy;
/*readonly*/ int spatialDimension;
/*readonly*/ int groupDimension;
/*readonly*/ int chares, points, iter, LB_FREQ;

/** ESQUEMA DE RESOLUCION:
* - MAIN: 
*        1. Crea vectores de centros y el contenedor
*        2. Inicializa y corre (run)
*
* - CONTENEDOR:
*        1.
* 
*/

/* Main class */
class Main : public CBase_Main
{
public:
	int donestep = 0, doneContainer = 0;
	
	Main(CkArgMsg *m) {
		donestep = 0; doneContainer = 0;
		
		mainProxy = thisProxy;

		if (m->argc < 7) {
			ckout << "ERROR, usage " << m->argv[0] << " <chares> <data points per chare> <data dimensions> 
				<k> <iterations> <balance frequency>" << endl;
			CkExit();
		}

		// Variable assignation
		chares = atoi(m->argv[1]);
		points_per_chare = atoi(m->argv[2]);
		spatialDimension = atoi(m->argv[3]);
		groupDimension = atoi(m->argv[4]); // Cantidad de zonas
		max_iter = atoi(m->argv[5]);
		LB_FREQ = atoi(m->argv[6]);

		delete m;

		// ZONE CENterS
		vector<Mean> centers_position[groupDimension];

		for(int i=0; i<groupDimension; i++){
			centers_position[i] = Mean(spatialDimension); // Con 1 argumento lo lleno con random
		}

		vector<Mean> centers_sumatoria[groupDimension]; // Sumatoria de coordenadas de puntos
		//vector<int> centers_sumados[groupDimension]; // Almacena la cantidad ya sumada de puntos

		// Inicializamos
		vector<double> coord_nulas[spatialDimension] = 0;

		// Inicializamos en random los centros de las zonas.
		reboot_vars();

		//create the container and start the simulation by calling run()
		containerProxy = CProxy_Container::ckNew();
		containerProxy.run(centers_position);
	}

	/** Method to reinitite variables
	*
	*/
	void reboot_vars(){
		for(int i=0; i<groupDimension; i++){
			//cluster_position[i] = Mean(spatialDimension); // Sin ningun argumento lo lleno con random
			centers_sumatoria[i] = Mean(spatialDimension, coord_nulas); // Este tiene las coordenadas de la sumatoria de puntos
			// centers_sumados[i] = 0;		// AhoRa es un atributo de Mean
	}

	/** Method to update the zone centers.
	*
	*/
	void updateCenters(Punto point){
		// Hay que actualizar los centros!
		// Primero sumamos todas las componentes
		centers_sumatoria[point._zona].suma(point);

	}

	// Function after finishing one step
    void donestep(int iter) {

		// COMPUTE NEW CENTERS AND REBOOT
		for (int i = 0; i < groupDimension< i++){
			centers_position[i] = centers_sumatoria[i].get_mean();
		}

		reboot_vars();


    	CkPrintf("FINISH ITERATION %d\n", iter);
		// RUN NEW ITER
		containerProxy.run(centers_position); // Le paso un vector con los MEAN
		}

	// Function to finish execution
	void done() {
		doneContainer++;
		if(doneContainer >= chares){
			CkPrintf("EXIT!\n");
			CkExit();
		}
	}

};

class Container : public CBase_Container {
public:

	vector<Punto> points;
	vector<int> clustering;
	//clustering.resize(groupDimension, 0)
	
	int _total;
	int remoteContainers = 0;
	int iteration;

	Container(){
		iteration = 0;
		_total = points_per_chare;
		usesAtSync = true;

		// Last chare has 10 times the points
		if(thisIndex==(chares-1))
			_total = points_per_chare*10;
		
		points.resize(_total, 0);

		for(int i=0; i<_total; i++){
			points[i] = Punto(); // Sin ningun argumento lo lleno con random.
			//clustering[i] = 999; // Inicializamos en un valor de error.
		}
	}

	Container(CkMigrateMessage* m) {};

	// RUN
	void run(vector<Mean> _cluster_position) {

		ckout << thisIndex <<" Arranque el run y mis center coord son: " << _cluster_position._coords() << endl;

		// checking for termination
		if(iteration == max_iter) {
			mainProxy.done();
		} else {

			// updating positions of particles
			updatePoints(_cluster_position);
		}
	}

	// Function to update the clustering vector
	// Tenemos 3 vectores:
	// - points[N]: tiene las posiciones de los puntos.
	// - clustering[N]: tiene el indice de la zona mas cercana
	// - cluster_position[k]: tiene las posiciones de los centros 
	void updatePoints(vector<Punto> _cluster_position){

		double _distancia = 999;

		// compute distance to center and update  clustering
		for(int i=0;i<_total; i++){
			_distancia = 999;

			for (int j = 0; j < groupDimension; j++){
				// Chequeamos el center más cernano al punto
				if( puntos[i].distancia(_cluster_position[j]) < _distancia ) {
					puntos[i]._zona = j; // Actualizamos el centro mas cercano
					_distancia = puntos[i].distancia(_cluster_position[j]); // actualizamos la distancia a comparar
				}
				
			}
			ckout << thisIndex <<" La distancia es:" << _distancia << endl;
		}
		// 
		
		// checking if all messages have been received
		remoteContainers++;
		if(remoteContainers == chares)
			finish(); // Calculamos los nuevos centros aca adentro
	}

	// Internal function to finish an iteration
	void finish() {
		
		// updating variables
		iteration++;
		remoteContainers = 0;

		ckout << thisIndex <<" Termine con la iteracion:" << iteration << " , ahora voy a contribuir" << endl;

		// inform the group member on this PE about the number of
		// particles in this iteration for this chare.
		// particleGroupProxy.ckLocalBranch()->collectNumParticles(iteration, numberParticles);
		

		// OJO QUE ACA HAY UNO POR PROCESADOR!
		// Hau que contribuir a la cantidad de puntos tomados

		for (int j = 0; j < _total; j++)
		{
			contribute(2*sizeof(int)+spatialDimension*sizeof(double), &points[j], CkReduction::nop, CkCallback(CkReductionTarget(Main, updateCenters), mainProxy)); 
		}
		// Hay que contribuir en donde esta el nuevo center

		// checking for checkpointing and load balancing
		if(iteration % LB_FREQ == 0) {
			AtSync(); 
		} else {
			//CkPrintf("CELL %d %d %d\n",thisIndex.x,thisIndex.y,iteration);
			contribute(sizeof(int), &iteration, CkReduction::max_int, CkCallback(CkReductionTarget(Main, donestep), mainProxy)); 
		}
 
	}

	// PUP method
	void pup(PUP::er &p) {
		CBase_Container::pup(p);
		p| points;
	}

};

#include "main.def.h"
