// ============================================================================
// SimUnits.h
// Contains class definitions for all unit scaling classes.
// ============================================================================


#ifndef _SIM_UNITS_H_
#define _SIM_UNITS_H_


#include <iostream>
#include <map>
#include <string>
#include "Parameters.h"
using namespace std;


// ============================================================================
// Class SimUnit
// Main parent class for each individual unit class.
// ============================================================================
class SimUnit
{
 public:

  SimUnit();

  virtual double SIUnit(string) = 0;
  virtual string LatexLabel(string) = 0;
  double OutputScale(string);

  double inscale;                           // Input scaling factor
  double inSI;                              // Input SI scaling factor
  double outcgs;                            // Output cgs scaling factor
  double outscale;                          // Output scaling factor
  double outSI;                             // Output SI scaling factor
  string inunit;                            // Input unit string
  string outunit;                           // Output unit string

};



// ============================================================================
// Class LengthUnit
// ============================================================================
class LengthUnit: public SimUnit
{
 public:
  LengthUnit() : SimUnit() {};
  double SIUnit(string);
  string LatexLabel(string);

};



// ============================================================================
// Class MassUnit
// ============================================================================
class MassUnit: public SimUnit
{
 public:
  MassUnit() : SimUnit() {};
  double SIUnit(string);
  string LatexLabel(string);
 
};



// ============================================================================
// Class TimeUnit
// ============================================================================
class TimeUnit: public SimUnit
{
 public:
  TimeUnit() : SimUnit() {};
  double SIUnit(string);
  string LatexLabel(string);

};



// ============================================================================
// Class VelocityUnit
// ============================================================================
class VelocityUnit: public SimUnit
{
 public:
  VelocityUnit() : SimUnit() {};
  double SIUnit(string);
  string LatexLabel(string);

};



// ============================================================================
// Class AccelerationUnit
// ============================================================================
class AccelerationUnit: public SimUnit
{
 public:
  AccelerationUnit() : SimUnit() {};
  double SIUnit(string);
  string LatexLabel(string);

};




// ============================================================================
// Class DensityUnit
// ============================================================================
class DensityUnit: public SimUnit
{
 public:
  DensityUnit() : SimUnit() {};
  double SIUnit(string);
  string LatexLabel(string);

};


/*
// ============================================================================
// Class ColumnDensityUnit
// ============================================================================
class ColumnDensityUnit: public SimUnit
{
 public:

  double SIUnit(string);
  string LatexLabel(string);

};



// ============================================================================
// Class PressureUnit
// ============================================================================
class PressureUnit: public SimUnit
{
 public:

  double SIUnit(string);
  string LatexLabel(string);

};



// ============================================================================
// Class ForceUnit
// ============================================================================
class ForceUnit: public SimUnit
{
 public:

  double SIUnit(string);
  string LatexLabel(string);

};
*/


// ============================================================================
// Class EnergyUnit
// ============================================================================
class EnergyUnit: public SimUnit
{
 public:
  EnergyUnit() : SimUnit() {};
  double SIUnit(string);
  string LatexLabel(string);

};



// ============================================================================
// Class MomentumUnit
// ============================================================================
class MomentumUnit: public SimUnit
{
 public:
  MomentumUnit() : SimUnit() {};
  double SIUnit(string);
  string LatexLabel(string);

};




// ============================================================================
// Class ColumnDensityUnit
// ============================================================================
class AngularMomentumUnit: public SimUnit
{
 public:
  AngularMomentumUnit() : SimUnit() {};
  double SIUnit(string);
  string LatexLabel(string);

};



// ============================================================================
// Class AngularVelocityUnit
// ============================================================================
class AngularVelocityUnit: public SimUnit
{
 public:
  AngularVelocityUnit() : SimUnit() {};
  double SIUnit(string);
  string LatexLabel(string);

};


/*
// ============================================================================
// Class MassAccretionRateUnit
// ============================================================================
class MassAccretionRateUnit: public SimUnit
{
 public:

  double SIUnit(string);
  string LatexLabel(string);

};



// ============================================================================
// Class LuminosityUnit
// ============================================================================
class LuminosityUnit: public SimUnit
{
 public:

  double SIUnit(string);
  string LatexLabel(string);

};



// ============================================================================
// Class OpacityUnit
// ============================================================================
class OpacityUnit: public SimUnit
{
 public:

  double SIUnit(string);
  string LatexLabel(string);

};



// ============================================================================
// Class MagneticFieldUnit
// ============================================================================
class MagneticFieldUnit: public SimUnit
{
 public:

  double SIUnit(string);
  string LatexLabel(string);

};



// ============================================================================
// Class ChargeUnit
// ============================================================================
class ChargeUnit: public SimUnit
{
 public:

  double SIUnit(string);
  string LatexLabel(string);

};



// ============================================================================
// Class CurrentDensityUnit
// ============================================================================
class CurrentDensityUnit: public SimUnit
{
 public:

  double SIUnit(string);
  string LatexLabel(string);

};
*/


// ============================================================================
// Class SpecificEnergyUnit
// ============================================================================
class SpecificEnergyUnit: public SimUnit
{
 public:
  SpecificEnergyUnit() : SimUnit() {};
  double SIUnit(string);
  string LatexLabel(string);

};



// ============================================================================
// Class SpecificEnergyRateUnit
// ============================================================================
class SpecificEnergyRateUnit: public SimUnit
{
 public:
  SpecificEnergyRateUnit() : SimUnit() {};
  double SIUnit(string);
  string LatexLabel(string);

};



// ============================================================================
// Class TemperatureUnit
// ============================================================================
class TemperatureUnit: public SimUnit
{
 public:
  TemperatureUnit() : SimUnit() {};
  double SIUnit(string);
  string LatexLabel(string);

};





// ============================================================================
// Class SimUnits
// Main simulation scaling class containing an instance of each unit.
// ============================================================================
class SimUnits
{
 public:

  SimUnits();
  ~SimUnits();

  void SetupUnits(Parameters &);

  bool ReadInputUnits;                      // Have input units been read-in
                                            // from the snapshot file?


  // Instances of all unit classes
  // --------------------------------------------------------------------------
  LengthUnit r;
  MassUnit m;
  TimeUnit t;
  VelocityUnit v;
  AccelerationUnit a;
  DensityUnit rho;
  //ColumnDensityUnit sigma;
  //PressureUnit press;
  //ForceUnit f;
  EnergyUnit E;
  MomentumUnit mom;
  AngularMomentumUnit angmom;
  AngularVelocityUnit angvel;
  //MassAccretionRateUnit dmdt;
  //LuminosityUnit L;
  //OpacityUnit kappa;
  //MagneticFieldUnit B;
  //ChargeUnit Q;
  //CurrentDensityUnit Jcur;
  SpecificEnergyUnit u;
  SpecificEnergyRateUnit dudt;
  TemperatureUnit temp;

};


#endif




