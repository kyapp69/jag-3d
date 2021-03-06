/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 
 Copyright 2012-2014 by Ames Laboratory and Skew Matrix Software, LLC
 
 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:
 
    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
 
    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
 
    3. This notice may not be removed or altered from any source
    distribution.
 
 *************** <auto-copyright.rb END do not edit this line> ***************/

#include <jag/draw/DrawInfo.h>
#include <gmtl/gmtl.h>


namespace jag {
namespace draw {


DrawInfo::DrawInfo()
    : _startFrame( true ),
      _id( 0 ),
      _externalProgramID( 0 ),
      _elementBufferID( 0 ),
      _transform(),
      _controlFlags( 0 )
{
}


void DrawInfo::startFrame( const bool startFrame )
{
    _startFrame = startFrame;
}


// namespace jag::draw::
}
}
