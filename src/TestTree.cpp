//=================================================================================================
//  TestTree.cpp
//  ..
//
//  This file is part of GANDALF :
//  Graphical Astrophysics code for N-body Dynamics And Lagrangian Fluids
//  https://github.com/gandalfcode/gandalf
//  Contact : gandalfcode@gmail.com
//
//  Copyright (C) 2013  D. A. Hubber, G. Rosotti
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
//=================================================================================================


#include <math.h>
#include "Constants.h"
#include "Parameters.h"
#include "RandomNumber.h"
#include "Sph.h"
#include "SphNeighbourSearch.h"
#include "gtest/gtest.h"


//=================================================================================================
//  class TreeTest
//=================================================================================================
class TreeTest : public testing::Test
{
public:

  void SetUp(void);
  void TearDown(void);

  static const int Npart = 1000;
  static const unsigned long rseed = 1000;
  static const int Nleafmax = 6;
  static const FLOAT thetamaxsqd = 0.1;
  static const FLOAT kernrange = 2.0;
  static const FLOAT macerror = 0.0001;
  string gravity_mac = "geometric";
  string multipole = "monopole";

  SphParticle<3> *partdata;
  Parameters params;
  KDTree<3,SphParticle> *kdtree;
  RandomNumber *randnumb;

};



//=================================================================================================
//  TreeTest::Setup
//=================================================================================================
void TreeTest::SetUp(void)
{
  // Create all objects for test
  partdata = new SphParticle<3>[Npart];
  randnumb = new XorshiftRand(rseed);
  kdtree = new KDTree<3,SphParticle>(Nleafmax, thetamaxsqd, kernrange,
                                     macerror, gravity_mac, multipole);

  // Create some simple particle configuration
  for (int i=0; i<Npart; i++) {
    for (int k=0; k<3; k++) partdata[i].r[k] = 1.0 - 2.0*randnumb->floatrand();
    partdata[i].m = 1.0 / (FLOAT) Npart;
    partdata[i].h = 0.1;
  }

  // Now build the tree using the particle configuration
  kdtree->Ntot = Npart;
  kdtree->Ntotmaxold = 0;
  kdtree->Ntotmax = Npart;
  kdtree->BuildTree(Npart,Npart,partdata,0.0);
  kdtree->StockTree(kdtree->kdcell[0],partdata);

  return;
}



//=================================================================================================
//  TreeTest::TearDown
//=================================================================================================
void TreeTest::TearDown(void)
{
  // Delete all objects created for test
  delete kdtree;
  delete randnumb;
  delete[] partdata;

  return;
}



//=================================================================================================
//  StructureTest
//=================================================================================================
TEST_F(TreeTest, StructureTest)
{
  int c;                            // Cell counter
  int cc;                           // Aux. cell counter
  int i;                            // Particle counter
  int j;                            // Aux. particle counter
  int l;                            // Tree level
  int Nactivecount=0;               // Counter for total no. of active ptcls
  int Ncount=0;                     // Total particle counter
  int *ccount;                      // Array for counting cells
  int *lcount;                      // Array for counting ptcls on each level
  int *pcount;                      // Array for counting particles in tree

  ccount = new int[kdtree->Ncellmax];
  pcount = new int[Npart];
  lcount = new int[kdtree->ltot+1];
  for (i=0; i<Npart; i++) pcount[i] = 0;
  for (c=0; c<kdtree->Ncellmax; c++) ccount[c] = 0;
  for (l=0; l<kdtree->ltot; l++) lcount[l] = 0;
  Ncount = 0;
  Nactivecount = 0;

  // Count how many times we enter a cell in a full tree walk
  c = 0;
  while (c < kdtree->Ncell) {
    ccount[c]++;
    if (kdtree->kdcell[c].c1 != -1) c = kdtree->kdcell[c].c1;
    else c = kdtree->kdcell[c].cnext;
  }

  // Now check we enter all cells once and once only
  for (c=0; c<kdtree->Ncell; c++) {
    ASSERT_EQ(ccount[c],1);
  }

  delete[] lcount;
  delete[] pcount;
  delete[] ccount;

}



//=================================================================================================
//  ComTest
//=================================================================================================
TEST_F(TreeTest, ComTest)
{
  FLOAT mtot = 0.0;
  FLOAT rcom[3],vcom[3];

  for (int k=0; k<3; k++) rcom[k] = 0.0;
  for (int k=0; k<3; k++) vcom[k] = 0.0;

  for (int i=0; i<Npart; i++) {
    for (int k=0; k<3; k++) rcom[k] += partdata[i].m*partdata[i].r[k];
    for (int k=0; k<3; k++) vcom[k] += partdata[i].m*partdata[i].v[k];
    mtot += partdata[i].m;
  }
  for (int k=0; k<3; k++) rcom[k] /= mtot;
  for (int k=0; k<3; k++) vcom[k] /= mtot;

  EXPECT_FLOAT_EQ(mtot,kdtree->kdcell[0].m);
  EXPECT_FLOAT_EQ(rcom[0],kdtree->kdcell[0].r[0]);
  EXPECT_FLOAT_EQ(rcom[1],kdtree->kdcell[0].r[1]);
  EXPECT_FLOAT_EQ(rcom[2],kdtree->kdcell[0].r[2]);
  EXPECT_FLOAT_EQ(vcom[0],kdtree->kdcell[0].v[0]);
  EXPECT_FLOAT_EQ(vcom[1],kdtree->kdcell[0].v[1]);
  EXPECT_FLOAT_EQ(vcom[2],kdtree->kdcell[0].v[2]);
}



//=================================================================================================
//  GatherTest
//=================================================================================================
TEST_F(TreeTest, GatherTest)
{
  int i;
  int j;
  int k;
  int Nneib;
  int Nneibtree;
  int *neiblist;
  FLOAT dr[3];
  FLOAT drsqd;
  FLOAT hrange;

  neiblist = new int[Npart];

  for (i=0; i<Npart; i++) {
    Nneib = 0;
    hrange = 2.0*partdata[i].h;
    for (j=0; j<Npart; j++) {
      for (k=0; k<3; k++) dr[k] = partdata[i].r[k] - partdata[j].r[k];
      drsqd = dr[0]*dr[0] + dr[1]*dr[1] + dr[2]*dr[2];
      if (drsqd < hrange*hrange) Nneib++;
    }
    Nneibtree = kdtree->ComputeGatherNeighbourList(partdata[i].r,hrange,Npart,neiblist,partdata);

    ASSERT_EQ(Nneib,Nneibtree);
  }

}