/*****************************************************************
 * Must be used with Expat compiled for UTF-8 output.
 */

#include <iostream>

#include "../expat_justparse_interface.h"

using namespace std;

int main(int argc, char *argv[])
{
  char c;
  string buf;
  while (!cin.eof())
  {
    cin.get(c);
    buf += c;
  }
  
  cout<<escape_xml(buf);
  
  return 0;
}
