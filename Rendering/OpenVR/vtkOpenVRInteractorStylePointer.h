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
  vtkTypeMacro(vtkOpenVRInteractorStylePointer, vtkInteractorStyle3D);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

protected:
  vtkOpenVRInteractorStylePointer();
  ~vtkOpenVRInteractorStylePointer() VTK_OVERRIDE;

  //Update ray callback
  vtkCallbackCommand* UpdateCallbackCommand;

  //Update render state
  static void Update(vtkObject* object,
    unsigned long event,
    void* clientdata,
    void* calldata);

  //Update pointer
  void UpdateRay(vtkRenderer* ren, uint32_t controllerIndex);

  //Override base class to reset this->InteractionProp when picking buttons
  // are released.
  void OnLeftButtonUp();
  void OnRightButtonUp();

  //Override base class. Check the Dragable property before rotating.
  void Rotate();

  float GetLength()
  {
    return this->Length;
  }

  //Draw the ray or not
  bool ShowRay;

private:
  vtkOpenVRInteractorStylePointer(const vtkOpenVRInteractorStylePointer&) VTK_DELETE_FUNCTION;  // Not implemented.
  void operator=(const vtkOpenVRInteractorStylePointer&)VTK_DELETE_FUNCTION;  // Not implemented.

  //Ray length
  float Length;

};

#endif
