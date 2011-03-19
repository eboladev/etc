#include <iostream.h>
#include "thrower.h"

int main(int argc, char** argv)
{
  Thrower MyThrower;

  try
  {
    MyThrower.ThrowIt();

    cout << "No exception this time.\n";

  }
  catch(char* ex)
  {
    cout << "Exception: " << ex << "\n" << flush;
  }
  catch (const char* ex)
  {
    cout << "Exception: " << ex << "\n" << flush;
  }
  catch (...)
  {
    cout << "Exception of unknown type" << endl << flush;
  }

  return 0;
}
