//=============================================================================
//  NbodySystemTree.cpp
//  Contains all function definitions for constructing N-body nearest-neighbour
//  tree for creating systems and sub-systems.
//
//  This file is part of GANDALF :
//  Graphical Astrophysics code for N-body Dynamics and Lagrangian Fluids
//  https://github.com/gandalfcode/gandalf
//  Contact : gandalfcode@gmail.com
//
//  Copyright (C) 2013  D. A. Hubber, G Rosotti
//
//  GANDALF is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  GANDALF is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License (http://www.gnu.org/licenses) for more details.
//=============================================================================


#include <math.h>
#include <string>
#include "Precision.h"
#include "Constants.h"
#include "NbodyParticle.h"
#include "StarParticle.h"
#include "SystemParticle.h"
#include "NbodySystemTree.h"
#include "MergeList.h"
#include "InlineFuncs.h"
#include "Debug.h"
using namespace std;



//=============================================================================
//  NbodySystemTree::NbodySystemTree()
/// NbodySystemTree constructor
//=============================================================================
template <int ndim>
NbodySystemTree<ndim>::NbodySystemTree()
{
  allocated_tree = false;
  Nnode = 0;
  Nnodemax = 0;
  Nbinary = 0;
  Nbinarymax = 0;
}



//=============================================================================
//  NbodySystemTree::~NbodySystemTree()
/// NbodySystemTree destructor
//=============================================================================
template <int ndim>
NbodySystemTree<ndim>::~NbodySystemTree()
{
  if (allocated_tree) DeallocateMemory();
}



//=============================================================================
//  NbodySystemTree::AllocateMemory
/// Allocate nearest-neighbour tree memory
//=============================================================================
template <int ndim>
void NbodySystemTree<ndim>::AllocateMemory(int N)
{
  debug2("[NbodySystemTree::AllocateMemory]");

  if (2*N > Nnodemax || !allocated_tree) {
    if (allocated_tree) DeallocateMemory();
    Nnodemax = 2*N;
    Nbinarymax = N;
    NNtree = new NNTreeCell<ndim>[Nnodemax];
    binary = new BinaryStar<ndim>[Nbinarymax];
  }

  return;
}



//=============================================================================
//  NbodySystemTree::DeallocateMemory
/// Deallocate nearest-neighbour tree memory
//=============================================================================
template <int ndim>
void NbodySystemTree<ndim>::DeallocateMemory(void)
{
  debug2("[NbodySystemTree::DeallocateMemory]");

  if (allocated_tree) {
    delete[] NNtree;
  }
  allocated_tree = false;

  return;
}



//=============================================================================
//  NbodySystemTree::CreateNbodySystemTree
/// Creates a nearest neighbour tree based on the positions of all stars
/// contained in the nbody object that is passed.
//=============================================================================
template <int ndim>
void NbodySystemTree<ndim>::CreateNbodySystemTree
(Nbody<ndim> *nbody)                ///< [in] Nbody object containing stars
{
  int i,ii,j,jj;                    // Node ids and counters
  int k;                            // Dimension counter
  int Nfreenode;                    // No. of free (i.e. unattached) nodes
  int *nodelist;                    // List of unattached nodes
  DOUBLE dr[ndim];                  // Relative position vector
  DOUBLE drsqd;                     // Distance squared

  debug2("[NbodySystemTree::CreateNbodySystemTree]");

  Nnode = 0;
  Nfreenode = 0;

  // Allocate memory for tree
  AllocateMemory(nbody->Nstar);
  nodelist = new int[Nnodemax];

  // Initialise all node variables before adding stars
  for (i=0; i<Nnodemax; i++) {
    NNtree[i].ichild1 = -1;
    NNtree[i].ichild2 = -1;
    NNtree[i].iparent = -1;
    NNtree[i].inearest = -1;
    NNtree[i].radius = 0.0;
    NNtree[i].rsqdnearest = big_number_dp;
    NNtree[i].gpe = 0.0;
    NNtree[i].gpe_internal = 0.0;
    NNtree[i].Ncomp = 0;
    NNtree[i].Nstar = 0;
    NNtree[i].Nchildlist = 0;
  }

  // Add one star to each lead node, recording the position and id
  for (i=0; i<nbody->Nstar; i++) {
    nodelist[i] = i;
    NNtree[i].Ncomp = 1;
    NNtree[i].Nstar = 1;
    for (k=0; k<ndim; k++) NNtree[i].r[k] = nbody->stardata[i].r[k];
    Nnode++;
    Nfreenode++;
#if defined(VERIFY_ALL)
    cout << "Setting leaf nodes of tree : " << i << endl;
#endif
  }


  // Process all remaining unconnected nodes to find new set of mutually
  // nearest neighbours for next phase of tree construction
  // ==========================================================================
  while (Nnode < Nnodemax) {

    // Construct list of remaining nodes
    Nfreenode = 0;
    for (i=0; i<Nnode; i++) {
      if (NNtree[i].iparent == -1) nodelist[Nfreenode++] = i;
    }

    // If we have only one remaining unconnected node (i.e. the root) exit loop
    if (Nfreenode == 1) break;

    // Identify the nearest neighbouring node for each node
    // ------------------------------------------------------------------------
    for (ii=0; ii<Nfreenode; ii++) {
      i = nodelist[ii];
      NNtree[i].rsqdnearest = big_number;

      for (jj=0; jj<Nfreenode; jj++) {
        j = nodelist[jj];
        if (i == j) continue;

        for (k=0; k<ndim; k++) dr[k] = NNtree[i].r[k] - NNtree[j].r[k];
        drsqd = DotProduct(dr,dr,ndim);
        if (drsqd < NNtree[i].rsqdnearest) {
          NNtree[i].rsqdnearest = drsqd;
          NNtree[i].inearest = j;
        }
      }
    }
    // ------------------------------------------------------------------------


    // Now identify all mutually nearest neighbours to create a new generation
    // of nodes
    // ------------------------------------------------------------------------
    for (ii=0; ii<Nfreenode-1; ii++) {
      i = nodelist[ii];

      for (jj=ii+1; jj<Nfreenode; jj++) {
        j = nodelist[jj];

        // If each node is the others nearest neighbour, then create a new
        // parent node with the two original nodes as child nodes
        if (NNtree[i].inearest == j && NNtree[j].inearest == i) {
          NNtree[i].iparent = Nnode;
          NNtree[j].iparent = Nnode;
          NNtree[Nnode].ichild1 = i;
          NNtree[Nnode].ichild2 = j;
          for (k=0; k<ndim; k++)
            NNtree[Nnode].r[k] = 0.5*(NNtree[i].r[k] + NNtree[j].r[k]);
          for (k=0; k<ndim; k++) dr[k] = NNtree[Nnode].r[k] - NNtree[i].r[k];
          drsqd = DotProduct(dr,dr,ndim);
          NNtree[Nnode].radius = sqrt(drsqd);
          NNtree[Nnode].Nstar = NNtree[i].Nstar + NNtree[j].Nstar;
          NNtree[Nnode].Ncomp = NNtree[i].Ncomp + NNtree[j].Ncomp;
          Nnode++;
#if defined(VERIFY_ALL)
	  cout << "Adding new node to tree : " << Nnode - 1 << "   " 
	       << i << "    " << j << endl;
#endif
        }

      }

    }
    // ------------------------------------------------------------------------

  };
  // ==========================================================================

  // Free all locally allocated memory
  delete[] nodelist;

#if defined(VERIFY_ALL)
  cout << "Nnode : " << Nnode << endl;
#endif

  return;
}



//=============================================================================
//  NbodySystemTree::BuildSubSystems
/// Calculate the properties of all nearest-neighbour tree nodes, starting from
/// single-star child-cells and then working up through the parent nodes and
/// finally the root node.
//=============================================================================
template <int ndim>
void NbodySystemTree<ndim>::BuildSubSystems
(Nbody<ndim> *nbody)               ///< [inout] Nbody object containing stars
{
  int c,c1,c2;                     // Cell counter
  int i,ii,j,jj;                   // Star and system particle counters
  int k;                           // Dimension counter
  int Nsystem;                     // No. of systems
  DOUBLE dr[ndim];                 // Relative position vector
  DOUBLE drsqd;                    // Distance squared
  DOUBLE dv[ndim];                 // Relative velocity vector
  DOUBLE invdrmag;                 // 1 / drmag
  DOUBLE ketot = 0.0;              // Kinetic energy
  DOUBLE vmean;                    // Average speed of stars in sub-cluster
  NbodyParticle<ndim> *si;         // Pointer to star/system i
  NbodyParticle<ndim> *sj;         // Pointer to star/system j

  debug2("[NbodySystemTree::BuildSubSystems]");

  // Set all counters
  nbody->Nnbody = 0;
  nbody->Nsystem = 0;
  Nsystem = 0;
  Nbinary = 0;

  // Loop through all nodes of tree and compute all physical quantities
  // ==========================================================================
  for (c=0; c<Nnode; c++) {

#if defined(VERIFY_ALL)
    cout << "Stocking node : " << c << "    Ncomp : " 
	 << NNtree[c].Ncomp << endl;
#endif

    // If node contains one star, set all properties equal to star values
    // ------------------------------------------------------------------------
    if (NNtree[c].Ncomp == 1) {
      i = c;
      NNtree[c].m = nbody->stardata[i].m;
      NNtree[c].h = nbody->stardata[i].h;
      for (k=0; k<ndim; k++) NNtree[c].r[k] = nbody->stardata[i].r[k];
      for (k=0; k<ndim; k++) NNtree[c].v[k] = nbody->stardata[i].v[k];
      for (k=0; k<ndim; k++) NNtree[c].a[k] = nbody->stardata[i].a[k];
      for (k=0; k<ndim; k++) NNtree[c].adot[k] = nbody->stardata[i].adot[k];
      for (k=0; k<ndim; k++) NNtree[c].a2dot[k] = nbody->stardata[i].a2dot[k];
      for (k=0; k<ndim; k++) NNtree[c].a3dot[k] = nbody->stardata[i].a3dot[k];
      NNtree[c].gpe = 0.5*nbody->stardata[i].m*nbody->stardata[i].gpot;
      NNtree[c].gpe_internal = 0.0;
      NNtree[c].tcross = big_number;
      NNtree[c].Nchildlist = 1;
      NNtree[c].childlist[0] = &nbody->stardata[i];

#if defined(VERIFY_ALL)
      cout << "Stocking single star data : " << i << "    " << NNtree[c].r[0] 
	   << "    " << NNtree[c].r[1] << "    " << NNtree[c].gpe << endl;
#endif

    }

    // Else, add together both child node properties
    // ------------------------------------------------------------------------
    else {

      c1 = NNtree[c].ichild1;
      c2 = NNtree[c].ichild2;
      NNtree[c].Ncomp = NNtree[c1].Ncomp + NNtree[c2].Ncomp;
      NNtree[c].m = NNtree[c1].m + NNtree[c2].m;
      NNtree[c].h = max(NNtree[c1].h,NNtree[c2].h);
      NNtree[c].gpe = NNtree[c1].gpe + NNtree[c2].gpe; 
      for (k=0; k<ndim; k++) {
        NNtree[c].r[k] = (NNtree[c1].m*NNtree[c1].r[k] +
			  NNtree[c2].m*NNtree[c2].r[k])/NNtree[c].m;
        NNtree[c].v[k] = (NNtree[c1].m*NNtree[c1].v[k] +
			  NNtree[c2].m*NNtree[c2].v[k])/NNtree[c].m;
        NNtree[c].a[k] = (NNtree[c1].m*NNtree[c1].a[k] +
			  NNtree[c2].m*NNtree[c2].a[k])/NNtree[c].m;
        NNtree[c].adot[k] = (NNtree[c1].m*NNtree[c1].adot[k] +
			  NNtree[c2].m*NNtree[c2].adot[k])/NNtree[c].m;
        NNtree[c].a2dot[k] = (NNtree[c1].m*NNtree[c1].a2dot[k] +
			  NNtree[c2].m*NNtree[c2].a2dot[k])/NNtree[c].m;
        NNtree[c].a3dot[k] = (NNtree[c1].m*NNtree[c1].a3dot[k] +
			  NNtree[c2].m*NNtree[c2].a3dot[k])/NNtree[c].m;
      }
      NNtree[c].gpe_internal = 0.0;
      ketot = 0.0;

#if defined(VERIFY_ALL)
      cout << "Stocking system data : " << c << "    " << NNtree[c].r[0] 
	   << "    " << NNtree[c].r[1] << endl;
      cout << "Child energies : " << NNtree[c].gpe << "    " 
	   << NNtree[c1].gpe << "   " << NNtree[c2].gpe
	   << "     " << NNtree[c].gpe_internal << "     " 
	   << NNtree[c1].gpe_internal << "    " 
	   << NNtree[c2].gpe_internal << endl;
#endif


      // If node contains within maximum allowed number of components, then 
      // check if node is a new system or not
      // ----------------------------------------------------------------------
      if (NNtree[c].Ncomp <= Ncompmax) {

        // Merge lists of stars/systems in child nodes
        NNtree[c].Nchildlist = NNtree[c1].Nchildlist + NNtree[c2].Nchildlist;
        for (i=0; i<NNtree[c1].Nchildlist; i++)
          NNtree[c].childlist[i] = NNtree[c1].childlist[i];
        for (i=0; i<NNtree[c2].Nchildlist; i++)
          NNtree[c].childlist[i+NNtree[c1].Nchildlist] =
            NNtree[c2].childlist[i];

#if defined(VERIFY_ALL)
	    cout << "No. of child systems for " << c << "   :   "
	      << NNtree[c].Nchildlist << endl;
#endif
	
	    // Compute internal kinetic energy
        ketot = 0.0;
        for (i=0; i<NNtree[c].Nchildlist; i++) {
          si = NNtree[c].childlist[i];
          for (k=0; k<ndim; k++) dv[k] = si->v[k] - NNtree[c].v[k];
          ketot += 0.5*si->m*DotProduct(dv,dv,ndim);
        }
        vmean = sqrt(2.0*ketot/NNtree[c].m);
	
        // Compute internal gravitational potential energy
        for (i=0; i<NNtree[c].Nchildlist-1; i++) {
          si = NNtree[c].childlist[i];
          for (j=i+1; j<NNtree[c].Nchildlist; j++) {
            sj = NNtree[c].childlist[j];
            for (k=0; k<ndim; k++) dr[k] = sj->r[k] - si->r[k];
            drsqd = DotProduct(dr,dr,ndim);
            invdrmag = 1.0 / (sqrt(drsqd) + small_number_dp);
            NNtree[c].gpe_internal += si->m*sj->m*invdrmag;
          }
        }

        // Then, estimate the crossing time as tcross = Rgrav / vdisp
        // where Rgrav = sqrt(m^2/G/Egrav).  Should give similar
        // timescale to binary period in case of bound binary.
        NNtree[c].tcross =
          sqrt(NNtree[c].m*NNtree[c].m/NNtree[c].gpe_internal)/vmean;
	
#if defined(VERIFY_ALL)
        cout << "gpe : " << NNtree[c].gpe << "    gpe_internal : "
	      << NNtree[c].gpe_internal << "    ketot : " << ketot << endl;
        if (NNtree[c].gpe < NNtree[c].gpe_internal) {
          cout << "Grav. energy problem" << endl;
          exit(0);
        }
        cout << "Checking system criteria : "
          << fabs(NNtree[c].gpe - NNtree[c].gpe_internal)/NNtree[c].gpe
          << "    " << gpefrac << "    " << NNtree[c].Ncomp << "     "
          << Ncompmax << endl;
#endif


        // Now check energies and decide if we should create a new sub-system
        // object.  If yes, create new system in main arrays
        // --------------------------------------------------------------------
        if (fabs(NNtree[c].gpe - NNtree[c].gpe_internal)
            < gpefrac*NNtree[c].gpe) {
	  
          // Copy centre-of-mass properties of new sub-system
          nbody->system[Nsystem].inode = c;
          nbody->system[Nsystem].dt_internal = NNtree[c].tcross;
          nbody->system[Nsystem].m = NNtree[c].m;
          nbody->system[Nsystem].h = NNtree[c].h;
          for (k=0; k<ndim; k++) {
            nbody->system[Nsystem].r[k] = NNtree[c].r[k];
            nbody->system[Nsystem].v[k] = NNtree[c].v[k];
            nbody->system[Nsystem].a[k] = NNtree[c].a[k];
            nbody->system[Nsystem].adot[k] = NNtree[c].adot[k];
            nbody->system[Nsystem].a2dot[k] = NNtree[c].a2dot[k];
            nbody->system[Nsystem].a3dot[k] = NNtree[c].a3dot[k];
            nbody->system[Nsystem].r0[k] = NNtree[c].r[k];
            nbody->system[Nsystem].v0[k] = NNtree[c].v[k];
            nbody->system[Nsystem].a0[k] = NNtree[c].a[k];
            nbody->system[Nsystem].adot0[k] = NNtree[c].adot[k];
          }

          // Compute and store binary properties if bound
          if (NNtree[c].Ncomp == 2) {
            for (k=0; k<ndim; k++) dv[k] = si->v[k] - sj->v[k];
            for (k=0; k<ndim; k++) dr[k] = sj->r[k] - si->r[k];
            drsqd = DotProduct(dr,dr,ndim);
            invdrmag = 1.0 / (sqrt(drsqd) + small_number_dp);
            binary[Nbinary].ichild1 = c1;
            binary[Nbinary].ichild2 = c2;
            binary[Nbinary].m = NNtree[c].m;
            for (k=0; k<ndim; k++) binary[Nbinary].r[k] = NNtree[c].r[k];
            for (k=0; k<ndim; k++) binary[Nbinary].v[k] = NNtree[c].v[k];
            binary[Nbinary].binen =
              0.5*DotProduct(dv,dv,ndim) - NNtree[c].m*invdrmag;
            binary[Nbinary].sma = -0.5*NNtree[c].m/binary[Nbinary].binen;
            binary[Nbinary].period = 0.0;
            binary[Nbinary].ecc = 0.0;
            if (si->m > sj->m) binary[Nbinary].q = sj->m/si->m;
            else binary[Nbinary].q = si->m/sj->m;
            Nbinary++;
          }
	  
          // Copy list of contained N-body particles (either systems or stars)
          // to system object, and also total no. of stars/components
          nbody->system[Nsystem].Ncomp = NNtree[c].Ncomp;
          nbody->system[Nsystem].Nchildren = 0;
          nbody->system[Nsystem].Npert = 0;
          for (i=0; i<NNtree[c].Nchildlist; i++)
            nbody->system[Nsystem].children[nbody->system[Nsystem].Nchildren++]
              = NNtree[c].childlist[i];

#if defined(VERIFY_ALL)
          cout << "Adding binary system : " << c << "    " << c1 << "    "
            << c2 << "     " << nbody->system[Nsystem].Ncomp << endl;
          cout << "System no. : " << Nsystem << endl;
          cout << "r : " << NNtree[c].r[0] << "    " << NNtree[c].r[1] << endl;
#endif

          // Finally, clear list and append newly created system particle.
          // Also, zero internal energy to allow detection of hierarchical
          // systems.
          NNtree[c].Ncomp = 1;
          NNtree[c].Nchildlist = 1;
          NNtree[c].childlist[0] = &(nbody->system[Nsystem]);
          NNtree[c].gpe = NNtree[c].gpe - NNtree[c].gpe_internal;
          NNtree[c].gpe_internal = 0.0;
          Nsystem++;

        }

      }

      // If lists contain more than the maximum allowed number of components,
      // then flush list of systems to main arrays
      // ----------------------------------------------------------------------
      else {

        // Merge lists of stars/systems in child nodes
        for (i=0; i<NNtree[c1].Nchildlist; i++)
          nbody->nbodydata[nbody->Nnbody++] = NNtree[c1].childlist[i];
        for (i=0; i<NNtree[c2].Nchildlist; i++)
          nbody->nbodydata[nbody->Nnbody++] = NNtree[c2].childlist[i];
        NNtree[c].Nchildlist = 0;
        NNtree[c1].Nchildlist = 0;
        NNtree[c2].Nchildlist = 0;

      }
      // ----------------------------------------------------------------------

    }
    // ------------------------------------------------------------------------

  }
  // ==========================================================================

  
  // For root cell, push all remaining systems (or single sub-systems) to list
  c = Nnode - 1;
  for (i=0; i<NNtree[c].Nchildlist; i++)
    nbody->nbodydata[nbody->Nnbody++] = NNtree[c].childlist[i];
  nbody->Nsystem = Nsystem;

#if defined(VERIFY_ALL)
  cout << "No. of remaining systems in root node " << c << "  :  " 
       << NNtree[c].Nchildlist << endl;
  cout << "List all main N-body particles : " << nbody->Nnbody << endl;
  for (i=0; i<nbody->Nnbody; i++) {
    cout << "i : " << i << "    r : " << nbody->nbodydata[i]->r[0] 
	 << "    " << nbody->nbodydata[i]->r[1] << "    Ncomp : " 
	 << nbody->nbodydata[i]->Ncomp << endl;
  }
  cout << "List all main system particles : " << nbody->Nsystem << endl;
  for (i=0; i<nbody->Nsystem; i++) {
    cout << "i : " << i << "    r : " << nbody->system[i].r[0] 
	 << "    " << nbody->system[i].r[1] << "     Ncomp : " 
	 << nbody->system[i].Ncomp << "   Nchildren : " 
	 << nbody->system[i].Nchildren << endl;
  }
#endif

  return;
}



//=============================================================================
//  NbodySystemTree::FindPerturberLists
/// Walk the N-body tree top-down and compute all perturber lists
//=============================================================================
template <int ndim>
void NbodySystemTree<ndim>::FindPerturberLists
(Nbody<ndim> *nbody)                ///< [in] Nbody object containing stars
{
  int c;                            // Node id
  int caux;                         // ..
  int cparent;                      // id of parent node
  int i;                            // Particle id
  int k;                            // Dimension counter
  int Nstack;                       // No. of unprocessed nodes on stack
  int s;                            // System counter
  int *cellstack;                   // ids of unprocessed nodes
  SystemParticle<ndim> *s1;         // ..

  debug2("[NbodySystemTree::FindPerturberLists]");


  cellstack = new int[nbody->Nstar];


  // Loop over all systems to find nearest perturbers
  // --------------------------------------------------------------------------
  for (s=0; s<nbody->Nsystem; s++) {
    s1 = &(nbody->system[s]);
    s1->Npert = 0;

    // Find node on NN-tree corresponding to system particle
    c = s1->inode;

    // If system contains all N-body particles, no perturbers exist
    if (c == Nnode - 1) continue;

    cout << "Finding perturbers for system " << s << endl;

    // Add 'sister' cell to stack
    Nstack = 0;
    cparent = NNtree[c].iparent;
    cellstack[Nstack++] = NNtree[c].inearest;
    

    // Now walk through the stack finding any perturbers
    // ------------------------------------------------------------------------
    do {
      Nstack--;
      caux = cellstack[Nstack];

      // If node is star/system, add to perturber list.
      // Else, open cell and add child cells to stack.
      if (NNtree[caux].Ncomp == 1 && caux != Nnode - 1) {
        s1->perturber[s1->Npert++] =  NNtree[caux].childlist[0];
        cout << "Added perturber for system " << s << "  c : " << caux << endl;
      }
      else {
        cellstack[Nstack++] = NNtree[caux].ichild1;
        cellstack[Nstack++] = NNtree[caux].ichild2;
      }

    } while (Nstack > 0);
    // ------------------------------------------------------------------------
    

  }
  // --------------------------------------------------------------------------

  delete[] cellstack;
  

  return;
}



//=============================================================================
//  NbodySystemTree::RestockTreeNodes
/// ..
//=============================================================================
template <int ndim>
void NbodySystemTree<ndim>::RestockTreeNodes
(Nbody<ndim> *nbody)                ///< [in] Nbody object containing stars
{
  debug2("[NbodySystemTree::RestockTreeNodes]");

  return;
}



template class NbodySystemTree<1>;
template class NbodySystemTree<2>;
template class NbodySystemTree<3>;
