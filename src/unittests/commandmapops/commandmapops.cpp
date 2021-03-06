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

#include <jag/draw/CommandMap.h>
#include <jag/draw/Program.h>
#include <jag/draw/VertexArrayObject.h>

#include <iostream>


using jag::draw::Command;
using jag::draw::CommandMap;
using jag::draw::Program;
using jag::draw::ProgramPtr;
using jag::draw::VertexArrayObject;
using jag::draw::VertexArrayObjectPtr;


bool test()
{
    {
        CommandMap cmA, cmB;
        ProgramPtr progA( ProgramPtr( new Program() ) );
        progA->setUserDataName( "progA" );
        cmA.insert( progA );
        if( !cmA.contains( Command::Program_t ) )
        {
            std::cerr << "cmA.contains( Command::Program_t ) is false." << std::endl;
            return( false );
        }
        ProgramPtr progB( ProgramPtr( new Program() ) );
        progB->setUserDataName( "progB" );
        cmB.insert( progB );
        if( !cmB.contains( Command::Program_t ) )
        {
            std::cerr << "cmB.contains( Command::Program_t ) is false." << std::endl;
            return( false );
        }

        CommandMap cmResult( cmA + cmB );
        if( !cmResult.contains( Command::Program_t ) )
        {
            std::cerr << "cmResult.contains( Command::Program_t ) is false." << std::endl;
            return( false );
        }
        if( cmResult[ Command::Program_t ]->getUserDataName() != "progB" )
        {
            std::cerr << "cmResult[ Command::Program_t ]->getUserDataName() != \"progB\"." << std::endl;
            return( false );
        }
    }

    {
        CommandMap cmA, cmB;
        VertexArrayObjectPtr vaoA( VertexArrayObjectPtr( new VertexArrayObject() ) );
        vaoA->setUserDataName( "vaoA" );
        cmA.insert( vaoA );
        if( !cmA.contains( Command::VertexArrayObject_t ) )
        {
            std::cerr << "cmA.contains( Command::VertexArrayObject_t ) is false." << std::endl;
            return( false );
        }
        VertexArrayObjectPtr vaoB( VertexArrayObjectPtr( new VertexArrayObject() ) );
        vaoB->setUserDataName( "vaoB" );
        cmB.insert( vaoB );
        if( !cmB.contains( Command::VertexArrayObject_t ) )
        {
            std::cerr << "cmB.contains( Command::VertexArrayObject_t ) is false." << std::endl;
            return( false );
        }

        CommandMap cmResult( cmA + cmB );
        if( !cmResult.contains( Command::VertexArrayObject_t ) )
        {
            std::cerr << "cmResult.contains( Command::VertexArrayObject_t ) is false." << std::endl;
            return( false );
        }
        if( cmResult[ Command::VertexArrayObject_t ]->getUserDataName() != "vaoB" )
        {
            std::cerr << "cmResult[ Command::VertexArrayObject_t ]->getUserDataName() != \"vaoB\"." << std::endl;
            return( false );
        }
    }

    {
        CommandMap cmA, cmB, cmC, current, delta;

        ProgramPtr progA( ProgramPtr( new Program() ) );
        progA->setUserDataName( "progA" );
        cmA.insert( progA );

        VertexArrayObjectPtr vaoA( VertexArrayObjectPtr( new VertexArrayObject() ) );
        vaoA->setUserDataName( "vaoA" );
        cmB.insert( vaoA );
        VertexArrayObjectPtr vaoB( VertexArrayObjectPtr( new VertexArrayObject() ) );
        vaoB->setUserDataName( "vaoB" );
        cmC.insert( vaoB );

        delta = current << cmA;
        if( !delta.contains( Command::Program_t ) )
        {
            std::cerr << "delta.contains( Command::Program_t ) is false." << std::endl;
            return( false );
        }
        if( delta[ Command::Program_t ]->getUserDataName() != "progA" )
        {
            std::cerr << "delta[ Command::Program_t ]->getUserDataName() != \"progA\"." << std::endl;
            return( false );
        }
        if( delta.contains( Command::VertexArrayObject_t ) )
        {
            std::cerr << "delta should not contain VertexArrayObject_t." << std::endl;
            return( false );
        }

        delta = current << cmB;
        if( delta.contains( Command::Program_t ) )
        {
            std::cerr << "delta should not contain Program_t." << std::endl;
            return( false );
        }
        if( !delta.contains( Command::VertexArrayObject_t ) )
        {
            std::cerr << "delta.contains( Command::VertexArrayObject_t ) is false." << std::endl;
            return( false );
        }
        if( delta[ Command::VertexArrayObject_t ]->getUserDataName() != "vaoA" )
        {
            std::cerr << "delta[ Command::VertexArrayObject_t ]->getUserDataName() != \"vaoA\"." << std::endl;
            return( false );
        }

        delta = current << cmC;
        if( delta.contains( Command::Program_t ) )
        {
            std::cerr << "delta should not contain Program_t." << std::endl;
            return( false );
        }
        if( !delta.contains( Command::VertexArrayObject_t ) )
        {
            std::cerr << "delta.contains( Command::VertexArrayObject_t ) is false." << std::endl;
            return( false );
        }
        if( delta[ Command::VertexArrayObject_t ]->getUserDataName() != "vaoB" )
        {
            std::cerr << "delta[ Command::VertexArrayObject_t ]->getUserDataName() != \"vaoB\"." << std::endl;
            return( false );
        }

        delta = current << cmA;
        if( delta.contains( Command::Program_t ) )
        {
            std::cerr << "delta should not contain Program_t." << std::endl;
            return( false );
        }
        if( delta.contains( Command::VertexArrayObject_t ) )
        {
            std::cerr << "delta should not contain VertexArrayObject_t." << std::endl;
            return( false );
        }
    }

    return( true );
}

int main( int argc, char** argv )
{
    std::cout << "This is a CTest regression test. To launch under Visual Studio, build the" << std::endl;
    std::cout << "RUN_TESTS target. Under Linux, enter 'make test' at a shell prompt." << std::endl;


    std::cout << std::endl;
    if( test() )
    {
        std::cout << "Test passed." << std::endl;
        return( 0 );
    }
    else
    {
        std::cout << "Test failed." << std::endl;
        return( 1 );
    }
}
