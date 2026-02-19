#include "et.hpp"

extern int et_runner(float);

int et(float v)
{
   int err = et_runner(v);
   return err;
}