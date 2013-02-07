// ============================================================================
// Sph.h
// ============================================================================


#ifndef _SPH_H_
#define _SPH_H_


#include <string>
#include "Constants.h"
#include "Dimensions.h"
#include "SphParticle.h"
#include "SphKernel.h"
#include "Parameters.h"
#include "EOS.h"
using namespace std;


// ============================================================================
// Class Sph
// Main parent Sph class.  Different SPH implementations 
// (e.g. grad-h SPH, Saitoh & Makino 2012) are derived from this class.
// Each implementation requires defining its own version of each function 
// (e.g. ComputeH for its own method of computing smoothing lengths).
// ============================================================================
class Sph
{
 public:

  // SPH functions for computing SPH sums with neighbouring particles 
  // (fully coded in each separate SPH implementation, and not in Sph.cpp)
  // --------------------------------------------------------------------------
  virtual int ComputeH(int,int,SphParticle *,Parameters &) = 0;
  virtual void ComputeSphProperties(int,int,SphParticle *,Parameters &) = 0;
  virtual void ComputeHydroForces(int,int,SphParticle *,Parameters &) = 0;
  virtual void ComputeGravForces(int,int,SphParticle *) = 0;
  virtual void ComputeMeanhZeta(int,int,int *) = 0;

  // SPH array memory allocation functions
  // --------------------------------------------------------------------------
  void AllocateMemory(int);
  void DeallocateMemory(void);
  void SphBoundingBox(float *, float *, int);
  void InitialSmoothingLengthGuess(void);

  // SPH particle counters and main particle data array
  // --------------------------------------------------------------------------
  bool allocated;                       // Is SPH memory allocated?
  int Nsph;                             // No. of SPH particles in simulation
  int Nghost;                           // No. of ghost SPH particles
  int Ntot;                             // No. of real + ghost particles
  int Nsphmax;                          // Max. no. of SPH particles in array
  int Nghostmax;                        // ..
  struct SphParticle *sphdata;          // Main SPH particle data array

  SphKernel *kern;                      // SPH kernel 
  EOS *eos;                             // Equation-of-state

  float alpha_visc;                     // alpha artificial viscosity parameter
  float beta_visc;                      // beta artificial viscosity parameter
  float h_fac;
  float h_converge;
  string avisc;
  string acond;
  int hydro_forces;
  int self_gravity;

#if !defined(FIXED_DIMENSIONS)
  int ndim;
  int vdim;
  int bdim;
  float invndim;
#endif

};



// ============================================================================
// Class GradhSph
// Class definition for conservative 'grad-h' SPH simulations (as derived 
// from the parent Sph class).  Full code for each of these class functions 
// written in 'GradhSph.cpp'.
// ============================================================================
class GradhSph: public Sph
{
 public:

  GradhSph(int,int,int);
  ~GradhSph();

  int ComputeH(int,int,SphParticle *,Parameters &);
  void ComputeSphProperties(int,int,SphParticle *,Parameters &);
  void ComputeHydroForces(int,int,SphParticle *,Parameters &);
  void ComputeGravForces(int,int,SphParticle *);
  void ComputeMeanhZeta(int,int,int *);

};


#endif
