// ============================================================================
// GhostParticles.cpp
// ============================================================================


#include <cstdlib>
#include <math.h>
#include <map>
#include <string>
#include "Precision.h"
#include "Constants.h"
#include "Debug.h"
#include "Sph.h"
#include "SphSimulation.h"
#include "SphParticle.h"
using namespace std;



// ============================================================================
// 
// ============================================================================
void SphSimulation::CheckBoundaries(void)
{
  int i;
  SphParticle *part;

  // x-boundary conditions
  for (i=0; i<sph->Nsph; i++) {
    part = &sph->sphdata[i];

    if (part->r[0] < simbox.boxmin[0])
      if (simbox.x_boundary_lhs == "periodic") part->r[0] += simbox.boxsize[0];
    if (part->r[0] > simbox.boxmax[0])
      if (simbox.x_boundary_rhs == "periodic") part->r[0] -= simbox.boxsize[0];


#if NDIM==2 || NDIM==3
    if (ndim >= 2 && part->r[1] < simbox.boxmin[1])
      if (simbox.y_boundary_lhs == "periodic") part->r[1] += simbox.boxsize[1];
    if (ndim >= 2 && part->r[1] > simbox.boxmax[1])
      if (simbox.y_boundary_rhs == "periodic") part->r[1] -= simbox.boxsize[1];
#endif

#if NDIM==3
    if (ndim == 3 && part->r[2] < simbox.boxmin[2])
      if (simbox.z_boundary_lhs == "periodic") part->r[2] += simbox.boxsize[2];
    if (ndim == 3 && part->r[2] > simbox.boxmax[2])
      if (simbox.z_boundary_rhs == "periodic") part->r[2] -= simbox.boxsize[2];
#endif

  }

  return;
}



// ============================================================================
// SphSimulation::SearchGhostParticles
// ============================================================================
void SphSimulation::SearchGhostParticles(void)
{
  int i;
  int k;
  const FLOAT ghost_range = 1.1;
  const FLOAT kernrange = sph->kernp->kernrange;
  SphParticle *sphdata = sph->sphdata;

  // Set all relevant particle counters
  sph->Nghost    = 0;
  sph->Nghostmax = sph->Nsphmax - sph->Nsph;
  sph->Ntot      = sph->Nsph;

  // If all boundaries are open, immediately return to main loop
  if (simbox.x_boundary_lhs == "open" && simbox.x_boundary_rhs == "open" &&
      simbox.y_boundary_lhs == "open" && simbox.y_boundary_rhs == "open" &&
      simbox.z_boundary_lhs == "open" && simbox.z_boundary_rhs == "open")
    return;

  debug2("[SphSimulation::SearchGhostParticles]");

  // Create ghost particles in x-dimension
  // --------------------------------------------------------------------------
  if ((simbox.x_boundary_lhs == "open" && 
       simbox.x_boundary_rhs == "open") == 0) {
    for (i=0; i<sph->Ntot; i++) {
      if (sphdata[i].r[0] < simbox.boxmin[0] + 
	  ghost_range*kernrange*sphdata[i].h) {
	if (simbox.x_boundary_lhs == "periodic")
	  CreateGhostParticle(i,0,sphdata[i].r[0] + simbox.boxsize[0],
			      sphdata[i].v[0],
			      sphdata[i].r[0] - simbox.boxmin[0]);
	if (simbox.x_boundary_lhs == "mirror")
	  CreateGhostParticle(i,0,2.0*simbox.boxmin[0] - 
			      sphdata[i].r[0],-sphdata[i].v[0],
			      sphdata[i].r[0] - simbox.boxmin[0]);
      }
      if (sphdata[i].r[0] > simbox.boxmax[0] - 
	  ghost_range*kernrange*sphdata[i].h) {
	if (simbox.x_boundary_rhs == "periodic")
	  CreateGhostParticle(i,0,sphdata[i].r[0] - simbox.boxsize[0],
			      sphdata[i].v[0],
			      simbox.boxmax[0] - sphdata[i].r[0]);
	if (simbox.x_boundary_rhs == "mirror")
	  CreateGhostParticle(i,0,2.0*simbox.boxmax[0] - 
			      sphdata[i].r[0],-sphdata[i].v[0],
			      simbox.boxmax[0] - sphdata[i].r[0]);
      }
    }
    sph->Ntot = sph->Nsph + sph->Nghost;
  }


  // Create ghost particles in y-dimension
  // --------------------------------------------------------------------------
#if NDIM==2 || NDIM==3
  if (ndim >= 2 && (simbox.y_boundary_lhs == "open" && 
		    simbox.y_boundary_rhs == "open") == 0) {
    for (i=0; i<sph->Ntot; i++) {
      if (sphdata[i].r[1] < simbox.boxmin[1] + 
	  ghost_range*kernrange*sphdata[i].h) {
	if (simbox.y_boundary_lhs == "periodic")
	  CreateGhostParticle(i,1,sphdata[i].r[1] + simbox.boxsize[1],
			      sphdata[i].v[1],
			      sphdata[i].r[1] - simbox.boxmin[1]);
	if (simbox.y_boundary_lhs == "mirror")
	  CreateGhostParticle(i,1,2.0*simbox.boxmin[1] - 
			      sphdata[i].r[1],-sphdata[i].v[1],
			      sphdata[i].r[1] - simbox.boxmin[1]);
      }
      if (sphdata[i].r[1] > simbox.boxmax[1] - 
	  ghost_range*kernrange*sphdata[i].h) {
	if (simbox.y_boundary_rhs == "periodic")
	  CreateGhostParticle(i,1,sphdata[i].r[1] - simbox.boxsize[1],
			      sphdata[i].v[1],
			      simbox.boxmax[1] - sphdata[i].r[1]);
	if (simbox.y_boundary_rhs == "mirror")
	  CreateGhostParticle(i,1,2.0*simbox.boxmax[1] - 
			      sphdata[i].r[1],-sphdata[i].v[1],
			      simbox.boxmax[1] - sphdata[i].r[1]);
      }
    }
    sph->Ntot = sph->Nsph + sph->Nghost;
  }
#endif


 // Create ghost particles in z-dimension
  // --------------------------------------------------------------------------
#if NDIM==3
  if (ndim == 3 && (simbox.z_boundary_lhs == "open" && 
		    simbox.z_boundary_rhs == "open") == 0) {
    for (i=0; i<sph->Ntot; i++) {
      if (sphdata[i].r[2] < simbox.boxmin[2] + 
	  ghost_range*kernrange*sphdata[i].h) {
	if (simbox.z_boundary_lhs == "periodic")
	  CreateGhostParticle(i,2,sphdata[i].r[2] + simbox.boxsize[2],
			      sphdata[i].v[2],
			      sphdata[i].r[2] - simbox.boxmin[2]);
	if (simbox.z_boundary_lhs == "mirror")
	  CreateGhostParticle(i,2,2.0*simbox.boxmin[2] - 
			      sphdata[i].r[2],-sphdata[i].v[2],
			      sphdata[i].r[2] - simbox.boxmin[2]);
      }
      if (sphdata[i].r[2] > simbox.boxmax[2] - 
	  ghost_range*kernrange*sphdata[i].h) {
	if (simbox.z_boundary_rhs == "periodic")
	  CreateGhostParticle(i,2,sphdata[i].r[2] - simbox.boxsize[2],
			      sphdata[i].v[2],
			      simbox.boxmax[2] - sphdata[i].r[2]);
	if (simbox.z_boundary_rhs == "mirror")
	  CreateGhostParticle(i,2,2.0*simbox.boxmax[2] - 
			      sphdata[i].r[2],-sphdata[i].v[2],
			      simbox.boxmax[2] - sphdata[i].r[2]);
      }
    }
    sph->Ntot = sph->Nsph + sph->Nghost;
  }
#endif

  // Quit here if we've run out of memory for ghosts
  if (sph->Ntot > sph->Nsphmax) {
    cout << "Not enough memory for ghost particles" << endl;
    exit(0);
  }

  return;
}



// ============================================================================
// SphSimulation::CreateGhostParticle
// ============================================================================
void SphSimulation::CreateGhostParticle(int i, int k, 
					FLOAT rk, FLOAT vk, FLOAT bdist)
{
  // Increase ghost counter and check there's enough space in memory
  if (sph->Nghost > sph->Nghostmax) {
    cout << "Not enough memory for new ghost" << endl;
    exit(0);
  }

  // If there's enough memory, create ghost particle in arrays
  sph->sphdata[sph->Nsph + sph->Nghost] = sph->sphdata[i];
  sph->sphdata[sph->Nsph + sph->Nghost].r[k] = rk;
  sph->sphdata[sph->Nsph + sph->Nghost].v[k] = vk;


  // Record id of original particle for later copying
  if (i >= sph->Nsph)
    sph->sphdata[sph->Nsph + sph->Nghost].iorig = sph->sphdata[i].iorig;
  else
    sph->sphdata[sph->Nsph + sph->Nghost].iorig = i;

  //cout << "Adding ghost particle : " << i << "   " << sph->Nghost << "    " 
  //     << sph->Nghostmax << "   " 
  //     << sph->sphdata[sph->Nsph + sph->Nghost].iorig << endl;

  sph->Nghost = sph->Nghost + 1;

  return;
}



// ============================================================================
// SphSimulation::CopyAccelerationsFromGhosts
// ============================================================================
void SphSimulation::CopyAccelerationFromGhosts(void)
{
  int i;
  int iorig;
  int j;
  int k;

  for (j=0; j<sph->Nghost; j++) {
	i = sph->Nsph + j;
	iorig = sph->sphdata[i].iorig;

	// Only look at active ghosts
	if (!sph->sphdata[iorig].active) continue;

	for (k=0; k<ndim; k++) sph->sphdata[iorig].a[k] += sph->sphdata[i].a[k];
	sph->sphdata[iorig].dudt += sph->sphdata[i].dudt;

  }

  return;
}