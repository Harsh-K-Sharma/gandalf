// ============================================================================
// InlineFuncs.h
// Contains definitions of any useful small utility functions that can be 
// inlined to improve readability/performance of the code.
// ============================================================================


#ifndef _INLINE_FUNCS_H_
#define _INLINE_FUNCS_H_


#include <string>
#include "Precision.h"
#include "Constants.h"
using namespace std;


// ============================================================================
// DotProduct
// Calculates the dot product between two vectors, v1 and v2, 
// of given length 'ndim'
// ============================================================================
template <typename T>
static inline T DotProduct(T *v1, T *v2, int ndim)
{
  if (ndim == 1)
    return v1[0]*v2[0];
  else if (ndim == 2)
    return v1[0]*v2[0] + v1[1]*v2[1];
  else if (ndim == 3)
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}



// ============================================================================
// PrintArray
// Print values of a given array to standard output
// ============================================================================
template <typename T>
static inline void PrintArray(string message, int Tsize, T *array)
{
  cout << message;
  for (int i=0; i<Tsize; i++) cout << array[i] << "  ";
  cout << endl;
  return;
}


#endif