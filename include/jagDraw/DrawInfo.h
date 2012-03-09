/*************** <auto-copyright.pl BEGIN do not edit this line> **************
*
* jag3d is (C) Copyright 2011-2012 by Kenneth Mark Bryden and Paul Martz
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License version 2.1 as published by the Free Software Foundation.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*
*************** <auto-copyright.pl END do not edit this line> ***************/

#ifndef __JAGDRAW_DRAW_INFO_H__
#define __JAGDRAW_DRAW_INFO_H__ 1

#include <jagDraw/Export.h>
#include <jagDraw/ContextSupport.h>
#include <jagDraw/ShaderProgram.h>


namespace jagDraw {


/** \class DrawInfo DrawInfo.h <jagDraw/DrawInfo.h>
\brief
\details
*/
struct JAGDRAW_EXPORT DrawInfo
{
    DrawInfo();

    jagDrawContextID _id;

    /** \brief Pointer to shader program currently in use.
    \details Required by VertexArray to query the \c index
    parameter for a generic vertex attribute. */
    ShaderProgramPtr _shader;
};


// jagDraw
}


// __JAGDRAW_DRAW_INFO_H__
#endif
