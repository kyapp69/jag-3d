/*
 -------------------------------------------------------------------------------
 | Producer - Copyright (C) 2001-2007  Don Burns                               |
 |                                                                             |
 | This library is free software; you can redistribute it and/or modify        |
 | it under the terms of the GNU Lesser General Public License as published    |
 | by the Free Software Foundation; either version 3 of the License, or        |
 | (at your option) any later version.                                         |
 |                                                                             |
 | This library is distributed in the hope that it will be useful, but         |
 | WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY  |
 | or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public     |
 | License for more details.                                                   |
 |                                                                             |
 | You should have received a copy of the GNU Lesser General Public License    |
 | along with this software; if not, write to the Free Software Foundation,    |
 | Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.               |
 |                                                                             |
 -------------------------------------------------------------------------------
 */
#ifndef PRODUCER_TRACKBALL_DEF
#define PRODUCER_TRACKBALL_DEF 1

#include <Chaskii/Export.h>
#include <Chaskii/Math/matrix.h>
#include <Chaskii/Math/vec.h>
#include <Chaskii/WindowSystem/EventResponder.h>
#include <map>



class Trackball: public iiWS::EventResponder
{
    public :
        //-------------------------------------------------------------------------
        // Constructor
            Trackball( void );

        // Destructor
            ~Trackball( void ) {}


        //-------------------------------------------------------------------------
        // Update Mode - Enumerants, specifying update taken by the trackball.  
        //     UpdateRotation - rotation about a fixed axis
        //     UpdatePan      - movements up and down relative to and parallel to
        //                      the screen
        //     UpdateDistance - Displacement toward and away from viewer
        //     

        enum UpdateMode { 
             UpdateNone,
             UpdateRotation,
             UpdatePan,
             UpdateDistance
        };

        //-------------------------------------------------------------------------
        // mapButtonState()
        // getMappedButtonState()
        //          state  - Bitwise or of push buttons
        //          mode   - One of UpdateMode's
        //
        // mapButtonState() maps a button or combination of button presses to an 
        // update type.
        // getMappedButtonState() returns the UpdateMode associated with a button
        // state.
        
        
        void mapButtonState( unsigned int state, UpdateMode mode )
        {
            _buttonMap[state] = mode;
        }

        UpdateMode getMappedButtonState( unsigned int state ) const
        {
            std::map<unsigned int,UpdateMode>::const_iterator p;
            p = _buttonMap.find( state );
            if( p != _buttonMap.end() )
                return p->second;
            else
                return UpdateNone;
        }

        //-------------------------------------------------------------------------
        // Orientation - Enumerants referring to default orientation of the 
        //               identity.  
        //      Y_UP   - Orientation referring to and relative to the screen:
        //               X right, Y UP, and Z toward the eye.
        //      Z_UP   - X right, Z UP, Y away from the eye.
        //      
        //  setOrientation()
        //  getOrientation()
        //
        //      mode - One of Y_UP, or Z_UP. 
        //      
        // sets/gets the orientation of the identity.
        //
        enum Orientation {
            Y_UP,
            Z_UP
        };

        void  setOrientation( Orientation mode );
        Orientation getOrientation() const { return _orientation; }

        void setComputeOrientation( bool flag ) { _computeOrientation = flag; }
        bool getComputeOrientation() { return _computeOrientation; }

        //-------------------------------------------------------------------------
        // Transform Order - Enumerants referring to the order in which rotation/
        //                   translation operations should occur.  
        //   RotateTranslate - Operation applies a rotation matrix to a translation 
        //                     matrix.  This is the style of the Inventor trackball,
        //                     for example
        //   TranslateRotate - Operation applies a translation matrix to a rotation
        //                     matrix.  This is the style of the Performer trackball,
        //                     for example.
        //
        //  setTransformOrder()
        //  getTransformOrder()
        //       mode        - One of RotateTranslate or TranslateRotate
        //
        //  set/get Transform order.

        enum TransformOrder {
            RotateTranslate,
            TranslateRotate
        };

        void setTransformOrder( TransformOrder mode ) { _transform_order = mode; }
        TransformOrder getTransformOrder() const { return _transform_order; }

        //-------------------------------------------------------------------------
        // RotationalMode - Enumerants referring to the method used for doing 
        //                  rotations.  
        //    FixedAxis   - Refers to applying rotations directly around the 
        //                  upward axis, or the axis perpendicular to the screen.
        //                  Performer trackball functions like this, for example.
        //    Spherical   - Refers to applying rotations in a spherical manner.
        //                  Inventor trackball functions like this, for example.
        //
        // setRotationalMode()
        // getRotationalMode()  - set/get the Rotational mode.
        //
        //    mode         - One of FixedAxis or Spherical
        // 
        enum RotationalMode {
            FixedAxis,
            Spherical
        };

        void setRotationalMode( RotationalMode mode ) { _rotational_mode = mode; }
        RotationalMode getRotationalMode() const { return _rotational_mode; }

        //-------------------------------------------------------------------------
        // ThrowMode  - Enumerants referring to modes used to control which operations
        //              may be allowed to continuously update.
        //   ThrowRotation          - Allows continuous update of Rotation operation.
        //   ThrowPan               - Allows continuous update of Pan operation.
        //   ThrowPanRotation       - Allows continuous update of Rotation and Pan 
        //                            operations.
        //   ThrowDistance          - Allows continuous update of Distancing operation.
        //   ThrowRotationDistance, - Allows continuous update of Rotation and 
        //                            Distancing operations.
        //   ThrowPanDistance       - Allows continuous update of Pan and Distancing
        //                           operations
        //   ThrowRotationPanDistance - Allows continuous update of all three of
        //                            Rotation, Pan and Distancing operations.
        //
        //  setThrowMode()
        //  getThrowMode()          - set/get Throw mode
        //  addThrowMode()
        //  removeThrowMode()       - Adds and removes a single attribute of the
        //                            ThrowMode.  For example, if current ThrowMode
        //                            is ThrowRotationPanDistance and 
        //                                 removeThrowMode( ThrowDistance ); 
        //                            is called, resulting ThrowMode will be
        //                            ThrowModePanRotation
        //
        //    mode  - One of the operations specfied in ThrowMode enumerant.
        //
        enum ThrowMode {
            ThrowNone,
            ThrowRotation,
            ThrowPan,
            ThrowPanRotation,
            ThrowDistance,
            ThrowRotationDistance,
            ThrowPanDistance,
            ThrowRotationPanDistance
        };

        void setThrowMode( ThrowMode throw_mode ) { _throw_mode = throw_mode; }
        ThrowMode getThrowMode() const{ return _throw_mode; }
        void addThrowMode( ThrowMode throw_mode )
        {
            unsigned int t0 = static_cast<unsigned int>(_throw_mode);
            unsigned int t1 = static_cast<unsigned int>(throw_mode);
            _throw_mode = static_cast<ThrowMode>(t0|t1);
        }
        void removeThrowMode( ThrowMode throw_mode )
        {
            unsigned int t0 = static_cast<unsigned int>(_throw_mode);
            unsigned int t1 = static_cast<unsigned int>(throw_mode);
            _throw_mode = static_cast<ThrowMode>(t0&(~t1));
        }

        void setThrowThreshold( float threshold ) { _throwThreshold = threshold; }

        //-------------------------------------------------------------------------
        //  OperationalMode - Enumerants specifying a configuration of button 
        //                    mappings, orientation,transform order, rotational mode
        //                    throw mode, and auto scale that correspond to some 
        //                    familiar behavior.
        //    DefaultOperationalMode - The default for this trackball
        //    InventorLike           - Behavior similar to the Inventor trackball
        //    PerformerLike          - Behavior similar to the Performer trackball
        //
        //  setOperationalMode()
        //  getOperationalMode()  - sets/gets the operational mode.  May be changed on 
        //                          the fly.
        //     mode               - one of DefaultOperationalMode, InventorLike or 
        //                          PerformerLike
        enum OperationalMode {
            DefaultOperationalMode,
            InventorLike,
            PerformerLike
        };

        void setOperationalMode( OperationalMode );
        OperationalMode getOperationalMode() const { return _operational_mode; }

        //-------------------------------------------------------------------------
        //  setAutoScale()
        //  getAutoScale()  - set/get auto scale.  Auto scale causes the scaling of 
        //                    translations and distance displacements based on the
        //                    distance from the viewing center
        //     flag - boolean 
        //     
        //  setScale()
        //  getScale()      - set/get scaling.  Ignored when auto scale is enabled.
        //                    Provides a method of controlling the scale if auto 
        //                    scaling is off.
        //
        //     scale - floating point number specifying the scale
        //
        //  setPanFOV()
        //  getPanFOV()      - set/get the panning FOV.  Used to compute the distance
        //                     a Pan operation should translate when auto scale is on.
        //                     This should correspond to the viewing angle of the 3D
        //                     viewing frustum.
        
        void  setAutoScale( bool flag ){ _auto_scale = flag; }
        bool  getAutoScale( void ) const { return _auto_scale; }

        void  setScale( float scale );
        float getScale( void ) const { return _scale; }

        void setMinimumScale( float minScale );
        float getMinimumScale( void ) { return _minimum_scale; }

        void  setRotScale( float scale ) { _rscale = scale; }
        float getRotScale( void ) const { return _rscale; }

        void  setPanFOV( float panFov ) { _pan_fov = panFov; }
        float getPanFOV( void ) const { return _pan_fov; }


        //-------------------------------------------------------------------------
        //  setReference() - Record the current state of the transform matrix as
        //                   a reference state.  When reset() is called, the 
        //                   transform matrix will return to the reference state.
        void setReference( void );

        //-------------------------------------------------------------------------
        // setDistance()
        // getDistance()
        // resetDistance()
        // distanceHasChanged() - set/get the distance to the viewing center of the
        //                        scene.  resetDistance() returns the distance to the
        //                        reference distance. distanceHasChanged() returns
        //                        a boolean which can be used for outside operations
        //                        based on whether the viewing distance has changed.
        //                           
        void setMinimumDistance( float dist );
        void setDistance( float dist, bool do_update=true );
        void resetDistance(bool do_update=true );
        float getDistance() const { return _distance; }
        bool distanceHasChanged() const { return _distance_has_changed; }


        //-------------------------------------------------------------------------
        // setTranslation()
        // getTranslation() - set/get the current translation transformation matrix
        // setRotation()
        // getRotation()   - set/get the current rotation transformation matrix
        //     Matrix  - matrix used to set or returned from a get.
        void setTranslation( const iiMath::matrix4f tmat ) { T = tmat; }
        const iiMath::matrix4f & getTranslation() const { return T; }

        void setRotation( const iiMath::matrix4f rmat ) { R = rmat; }
        const iiMath::matrix4f & getRotation() const { return R; }

        void setScaleMatrix( const iiMath::matrix4f smat ) { S = smat; }
        const iiMath::matrix4f & getScaleMatrix(void) const { return S; }

        void setMatrix( const iiMath::matrix4f mat );

        //-------------------------------------------------------------------------
        // translate()  - translate the transform matrix from its current state
        //   x, y, z    - translation parameters
        // rotate()     - rotate the transform matrix from its current state
        //   rad        - rotation angle in radians
        //   x, y, z    -  Rotation vector (rotate around x,y,z)

        void translate( float x, float y, float z, bool do_update=true ); 
        void rotate( float rad, float x, float y, float z, bool do_update=true); 
        void scale( float sx, float sy, float sz, bool do_update=true );

        //-------------------------------------------------------------------------
        //  input()     - Updates the Trackball with new input, and/or for continuous
        //                matrix updates.
        //   mx, my     - "Normalized" values.  For example, an application receiving
        //                mouse events on a window that is 800x800 pixels in size,
        //                should normalize the mouse positions such that the range
        //                will be -1.0 at the left most side of the window and 1.0 at 
        //                the right most side of the window for mx.
        //   mbutton    - button state.  A bit representation of pressed/released
        //                buttons.
        //
        virtual void input( float mx, float my, unsigned int mbutton, bool do_update=true );

        //-------------------------------------------------------------------------
        // update()  - Updates the transform matrix.  Called internally from 
        //             input(), translate() and rotate(), unless do_update=false.
        // restart() - Restarts the initial x an y reference values.  For example,
        //             if a button has been newly pressed and before motion events
        //             begin to change.  Used internally, for the most part.
        // reset()   - Returns the transform matrix to its reference state.
        void update( void );
        void restart( float mx, float my );
        void reset( bool do_update=true );
        
        void enableRotation() { _rotation = true; }
        void disableRotation() { _rotation = false; }
        bool isRotationEnabled() { return _rotation; }

        void enablePanning() { _panning = true; }
        void disablePanning() { _panning = false; }
        bool isPanningEnabled() { return _panning; }

        void enableDistancing() { _distancing = true; }
        void disableDistancing() { _distancing = false; }
        bool isDistancingEnabled() { return _distancing; }

        void enableAllTransforms() { _rotation = _panning = _distancing = true; }


        //-------------------------------------------------------------------------
        // getMatrix() - Returns the transform matrix as an Matrix.  Note that
        //               this matrix is not valid until update() has been called.
        iiMath::matrix4f &getMatrix( void );

        const iiMath::matrix4f &getMatrix( void ) const;


        //-------------------------------------------------------------------------
        // EventResponder virtual routines
        virtual bool normalizeMouseCoordinates() { return true; }
        virtual bool sendPassiveMouseMotion() { return false; }
        virtual bool sendKeyRepeats() { return false; }
        virtual bool buttonPress( unsigned int mbutton, float mx, float my ) ;
        virtual bool buttonRelease( unsigned int mbutton, float mx, float my );
        virtual bool mouseMotion( float mx, float my) ;
        virtual bool keyPress( iiWS::KeyCharacter key, bool isRepeat ) ;
        virtual void frame( unsigned int );

    protected :
        //-------------------------------------------------------------------------

        iiMath::matrix4f S;         // Scale matrix
        iiMath::matrix4f O;         // Orientation Matrix
        iiMath::matrix4f R;         // Rotation Matrix 
        iiMath::matrix4f Rr;        // Reference Rotation Matrix 
        iiMath::matrix4f T;         // Translation Matrix
        iiMath::matrix4f Tr;        // Reference Translation :Matrix
        iiMath::matrix4f defTX;     // Transform matrix
        iiMath::matrix4f &TX;        // Transform matrix

        float _distance;
        float _min_distance;
        bool _min_distance_is_set;
        float _lastx, _lasty;
        float _dx, _dy;
        float _distance_ref;

        Orientation _orientation;
        unsigned int _mbutton;
        bool  _auto_scale;
        bool  _minimum_scale_is_set;
        float _minimum_scale;
        float _scale;
        float _rscale;
        float _pan_fov;

        UpdateMode _update_mode;
        std::map <unsigned int, UpdateMode>_buttonMap;
        ThrowMode  _throw_mode;
        float _throwThreshold;
        TransformOrder _transform_order;
        RotationalMode _rotational_mode;

        OperationalMode _operational_mode;
        bool _distance_has_changed;

        bool _rotation;
        bool _panning;
        bool _distancing;
        bool _computeOrientation;
        void  updateScale();

        // From EventResponder
        float _mx, _my;
        unsigned int _button;
};


#endif
