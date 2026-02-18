#include "et.hpp"

extern int et_runner();

int et()
{
   int err = et_runner();
   return err;
}