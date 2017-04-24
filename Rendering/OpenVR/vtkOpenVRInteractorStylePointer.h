/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkOpenVRInteractorStylePointer.h

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
* @class   vtkOpenVRInteractorStylePointer
* @brief   Interactions using a 3D ray
*
* vtkOpenVRInteractorStylePointer provides the same kind of interactions
* as the vtkInteractorStyle3D but uses the vtkOpenVRPropPicker to pick a
* vtkProp along a ray that is defined by the controller's position and
* orientation.
*
* @sa
* vtkInteractorStyle3D
*/

#ifndef vtkOpenVRInteractorStylePointer_h
#define vtkOpenVRInteractorStylePointer_h

#include "vtkRenderingOpenVRModule.h" // For export macro
#include "vtkInteractorStyle3D.h"

class vtkCallbackCommand;

class VTKRENDERINGOPENVR_EXPORT vtkOpenVRInteractorStylePointer : public vtkInteractorStyle3D
{
public:
  static vtkOpenVRInteractorStylePointer *New();
  vtkTypeMacro( vtkOpenVRInteractorStylePointer, vtkInteractorStyle3D );
  void PrintSelf( ostream& os, vtkIndent indent ) VTK_OVERRIDE;

protected:
  vtkOpenVRInteractorStylePointer();
  ~vtkOpenVRInteractorStylePointer() VTK_OVERRIDE;

  //Update ray callback
  vtkCallbackCommand* UpdateRayCallbackCommand;

  //Update position of the ray
  static void UpdateRay( vtkObject* object,
    unsigned long event,
    void* clientdata,
    void* calldata );

  // This method handles updating the prop based on changes in the devices
  // pose. We use rotate as the state to mean adjusting-the-actor-pose
  void Rotate() VTK_OVERRIDE;

  float GetLength()
  {
    return this->Length;
  }

private:
  vtkOpenVRInteractorStylePointer( const vtkOpenVRInteractorStylePointer& ) VTK_DELETE_FUNCTION;  // Not implemented.
  void operator=( const vtkOpenVRInteractorStylePointer& )VTK_DELETE_FUNCTION;  // Not implemented.

  //Ray length
  float Length;
};

#endif
