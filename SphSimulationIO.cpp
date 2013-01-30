// ============================================================================
// SphSimulationIO.cpp
// ============================================================================


#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstring>
#include "SphSimulation.h"
#include "Parameters.h"
#include "Debug.h"
using namespace std;



// ============================================================================
// SphSimulation::ReadSnapshotFile
// ============================================================================
bool SphSimulation::ReadSnapshotFile(string filename, string fileform)
{
  debug1("[SphSimulation::ReadSnapshotFile]");

  if (fileform == "ascii")
    return ReadColumnSnapshotFile(filename);
  else {
    cout << "Unrecognised file format" << endl;
    return false;
  }
}



// ============================================================================
// SphSimulation::WriteSnapshotFile
// ============================================================================
bool SphSimulation::WriteSnapshotFile(string filename, string fileform)
{
  debug1("[SphSimulation::WriteSnapshotFile]");

  if (fileform == "column")
    return WriteColumnSnapshotFile(filename);
  else {
    cout << "Unrecognised file format" << endl;
    return false;
  }
}




// ============================================================================
// SphSimulation::ReadColumnSnapshotFile
// ============================================================================
bool SphSimulation::ReadColumnSnapshotFile(string filename)
{
  int i;
  int ndimaux;
  int Npart;
  int Nstar;
  ifstream infile;

  debug1("[SphSimulation::ReadColumnSnapshotFile]");

  // Open file and read header information
  infile.open(filename.c_str());
  infile >> Npart;
  infile >> Nstar;
  infile >> ndimaux;

  // Check dimensionality matches if using fixed dimensions
#if defined(FIXED_DIMENSIONS)
  if (ndimaux != ndim) {
    cout << "Incorrect no. of dimensions in file : " 
	 << ndimaux << "  [ndim : " << ndim << "]" << endl;
    return false;
  }
#else
  ndim = ndimaux;
#endif

  sph->Nsph = Npart;
  sph->AllocateMemory(sph->Nsph);
  i = 0;

  // Read in data depending on dimensionality
  // --------------------------------------------------------------------------
  while (infile.good()) {
    if (ndim == 1) infile >> sph->sphdata[i].r[0] >> sph->sphdata[i].v[0] 
			  >> sph->sphdata[i].m >> sph->sphdata[i].h
			  >> sph->sphdata[i].rho >> sph->sphdata[i].u;
    else if (ndim == 2) infile >> sph->sphdata[i].r[0] >> sph->sphdata[i].r[1] 
			       >> sph->sphdata[i].v[0] >> sph->sphdata[i].v[1] 
			       >> sph->sphdata[i].m >> sph->sphdata[i].h
			       >> sph->sphdata[i].rho >> sph->sphdata[i].u;
    else if (ndim == 3) infile >> sph->sphdata[i].r[0] >> sph->sphdata[i].r[1] 
			       >> sph->sphdata[i].r[2] >> sph->sphdata[i].v[0] 
			       >> sph->sphdata[i].v[1] >> sph->sphdata[i].v[2] 
			       >> sph->sphdata[i].m >> sph->sphdata[i].h
			       >> sph->sphdata[i].rho >> sph->sphdata[i].u;
    i++;
  }

  infile.close();

  return true;
}



// ============================================================================
// SphSimulation::WriteColumnSnapshotFile
// ============================================================================
bool SphSimulation::WriteColumnSnapshotFile(string filename)
{
  int i;
  int ndimaux;
  int Npart;
  int Nstar;
  ofstream outfile;

  debug1("[SphSimulation::WriteColumnSnapshotFile]");

  cout << "Writing current data to snapshot file : " << filename << endl;

  // Open file and read header information
  outfile.open(filename.c_str());
  outfile << sph->Nsph << endl;
  outfile << 0 << endl;
  outfile << ndim << endl;

  // Write data
  // --------------------------------------------------------------------------
  for (i=0; i<sph->Nsph; i++) {
    if (ndim == 1) outfile << sph->sphdata[i].r[0] << "   " 
			   << sph->sphdata[i].v[0] << "   "
			   << sph->sphdata[i].m << "   " 
			   << sph->sphdata[i].h << "   "
			   << sph->sphdata[i].rho << "   " 
			   << sph->sphdata[i].u
			   << endl;
    else if (ndim == 2) outfile << sph->sphdata[i].r[0] << "   " 
				<< sph->sphdata[i].r[1] << "   " 
				<< sph->sphdata[i].v[0] << "   " 
				<< sph->sphdata[i].v[1] << "   "
				<< sph->sphdata[i].m << "   "
				<< sph->sphdata[i].h << "   "
				<< sph->sphdata[i].rho << "   "
				<< sph->sphdata[i].u
				<< endl;
    else if (ndim == 3) outfile << sph->sphdata[i].r[0] << "   "
				<< sph->sphdata[i].r[1] << "   " 
				<< sph->sphdata[i].r[2] << "   "
				<< sph->sphdata[i].v[0] << "   " 
				<< sph->sphdata[i].v[1] << "   " 
				<< sph->sphdata[i].v[2] << "   "
				<< sph->sphdata[i].m << "   "
				<< sph->sphdata[i].h << "   "
				<< sph->sphdata[i].rho << "   "
				<< sph->sphdata[i].u
				<< endl;
  }

  outfile.close();

  return true;
}