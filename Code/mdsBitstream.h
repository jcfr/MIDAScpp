/*=========================================================================

  Copyright (c) Kitware Inc. All rights reserved.
  See Copyright.txt or http://www.Kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _mdsBitstream_h_
#define _mdsBitstream_h_

#include "mdsObject.h"
#include <vector>

namespace mdo{
  class Bitstream;
}

namespace mds{

/** This class represent a bitstream on the MIDAS server. */
class Bitstream : public Object
{
public:

  Bitstream();
  ~Bitstream();

  // Fill the item given the id
  bool Fetch();
  bool Commit();
  void SetObject(mdo::Object* object);

protected:

  mdo::Bitstream*   m_Bitstream;
};

} //end namespace

#endif //_mdsBitstream_h_
