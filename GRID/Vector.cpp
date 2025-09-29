#include "Vector.h"
#include <cmath>

// add Vectors u & v together
Vector add(Vector u, Vector v)
{
  Vector result;
  result.x = u.x + v.x;
  result.y = u.y + v.y;
  return result;
}

// subtract v from u
Vector sub(Vector u, Vector v)
{
  Vector result;
  result.x = u.x - v.x;
  result.y = u.y - v.y;
  return result;
}

// calculate dot product of vectors
double dot(Vector u, Vector v)
{
  double result = (u.x * v.x + u.y * v.y);
  return result;
}

// calculate length of vector
double length(Vector u)
{
   return sqrt(dot(u, u));
}

// multiply by scalar
Vector multiply(Vector u, double s)
{
  Vector result;
  result.x = u.x * s;
  result.y = u.y * s;
  return result;
}

// zero the vector
void zero(Vector* u)
{
  u->x = 0;
  u->y = 0;
}
