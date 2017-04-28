/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkOpenVRInteractorStylePointer.cxx

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOpenVRInteractorStylePointer.h"

#include "vtkObjectFactory.h"
#include "vtkOpenVRPropPicker.h"

#include "vtkCallbackCommand.h"
#include "vtkOpenVRRenderWindow.h"
#include "vtkOpenVRRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkOpenVRModel.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkOpenVRInteractorStylePointer );

//----------------------------------------------------------------------------
vtkOpenVRInteractorStylePointer::vtkOpenVRInteractorStylePointer()
{
  this->InteractionPicker = vtkOpenVRPropPicker::New();

  this->UpdateCallbackCommand = vtkCallbackCommand::New();
  this->UpdateCallbackCommand->SetClientData(this);
  this->UpdateCallbackCommand->SetCallback(this->Update);
  this->AddObserver(vtkCommand::RenderEvent,
    this->UpdateCallbackCommand,
    this->Priority);

  this->ShowRay = true;
}

//----------------------------------------------------------------------------
vtkOpenVRInteractorStylePointer::~vtkOpenVRInteractorStylePointer()
{
  this->UpdateCallbackCommand->Delete();
}

//----------------------------------------------------------------------------
void vtkOpenVRInteractorStylePointer::Update(vtkObject*,
  unsigned long,
  void* clientdata,
  void*)
{
  vtkOpenVRInteractorStylePointer* self =
    reinterpret_cast< vtkOpenVRInteractorStylePointer * >( clientdata );

  vtkRenderer* ren = self->GetCurrentRenderer();
  if( !ren )
    {
    vtkErrorWithObjectMacro(self,
      "Unable to get vtkRenderer. Need to SetCurrentRenderer()");
    return;
    }

  vtkOpenVRRenderWindow* renWin =
    static_cast< vtkOpenVRRenderWindow* >( ren->GetRenderWindow() );
  if( !renWin )
    {
    vtkErrorWithObjectMacro(self, "Unable to get vtkOpenVRRenderWindow");
    return;
    }

  vr::IVRSystem *pHMD = renWin->GetHMD();
  if( !pHMD )
    {
    vtkErrorWithObjectMacro(self, "Unable to get HMD ");
    return;
    }

  // Compute the active controller index
  vr::TrackedDeviceIndex_t RightControllerIndex = vr::k_unTrackedDeviceIndex_Hmd;

  for( uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1;
    unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++ )
    {
    if( pHMD->GetTrackedDeviceClass( unTrackedDevice ) ==
      vr::ETrackedDeviceClass::TrackedDeviceClass_Controller )
      {
      // Found a controller index
      // Check if the controller is on
      vr::ETrackedControllerRole role =
        pHMD->GetControllerRoleForTrackedDeviceIndex( unTrackedDevice );
      if( role == vr::ETrackedControllerRole::TrackedControllerRole_RightHand )
        {
        renWin->GetTrackedDeviceModel( unTrackedDevice )->SetShowRay( self->ShowRay );
        RightControllerIndex = unTrackedDevice;
        }
      else if( role == vr::ETrackedControllerRole::TrackedControllerRole_LeftHand )
        {
        renWin->GetTrackedDeviceModel( unTrackedDevice )->SetShowRay( false );
        }
      }
    }
  self->UpdateRay( ren, RightControllerIndex );
}

//----------------------------------------------------------------------------
void vtkOpenVRInteractorStylePointer::UpdateRay(vtkRenderer* ren,
  uint32_t controllerIndex)
{
  if( !this->ShowRay )
    {
    return;
    }

  vtkOpenVRRenderWindowInteractor* iren =
    static_cast< vtkOpenVRRenderWindowInteractor* >( this->Interactor );
  if( !iren )
    {
    vtkErrorWithObjectMacro( this,
      "Unable to get vtkOpenVRRenderWindowInteractor" );
    return;
    }

  vtkOpenVRRenderWindow* renWin =
    static_cast< vtkOpenVRRenderWindow* >( ren->GetRenderWindow() );
  if( !renWin )
    {
    vtkErrorWithObjectMacro( this, "Unable to get vtkOpenVRRenderWindow" );
    return;
    }

  if( controllerIndex == vr::k_unTrackedDeviceIndex_Hmd )
    {
    return;
    }
  // Compute controller position and world orientation
  double p0[3]; //Ray start point
  double wxyz[4];// Controller orientation
  double dummy_ppos[3];
  vr::TrackedDevicePose_t &tdPose = renWin->GetTrackedDevicePose( controllerIndex );
  iren->ConvertPoseToWorldCoordinates( ren, tdPose, p0, wxyz, dummy_ppos );

  //Compute ray length.
  double p1[3];
  vtkOpenVRPropPicker* picker =
    static_cast< vtkOpenVRPropPicker* >( this->InteractionPicker );
  picker->PickProp( p0, wxyz, ren, ren->GetViewProps() );
  //If something is picked, set the length accordingly
  if( this->InteractionPicker->GetPath() )
    {
    this->InteractionPicker->GetPickPosition( p1 );
    this->Length = sqrt( vtkMath::Distance2BetweenPoints( p0, p1 ) );
    }
  //Keep the same length if the ray moves faster than the vtkProp
  else if( this->InteractionProp != NULL )
    {
    double* p = this->InteractionProp->GetPosition();
    this->Length = sqrt( vtkMath::Distance2BetweenPoints( p0, p ) );
    }
  //Otherwise set the length to its max
  else
    {
    this->Length = ren->GetActiveCamera()->GetClippingRange()[1];
    }

  ////Set ray length for vtkOpenVRRenderWindow::RenderModels()
  vtkOpenVRModel* controller = renWin->GetTrackedDeviceModel( controllerIndex );
  if( controller )
    {
    controller->SetRayLength( this->Length );
    }
}



//----------------------------------------------------------------------------
void vtkOpenVRInteractorStylePointer::OnRightButtonUp()
{
  Superclass::OnRightButtonUp();
  //Nothing picked, reset InteractionProp
  this->InteractionProp = NULL;
}

//----------------------------------------------------------------------------
void vtkOpenVRInteractorStylePointer::OnLeftButtonUp()
{
  Superclass::OnLeftButtonUp();
  //Nothing picked, reset InteractionProp
  this->InteractionProp = NULL;
}

//----------------------------------------------------------------------------
void vtkOpenVRInteractorStylePointer::Rotate()
{
  //WARNING: Overriding Rotate() to fix vtkInteractorStyle3D::Rotate() which
  //         isn't checking if the prop is dragable before rotating it.
  if( this->InteractionProp == NULL || !this->InteractionProp->GetDragable() )
    {
    return;
    }
  this->Superclass::Rotate();
}

//----------------------------------------------------------------------------
void vtkOpenVRInteractorStylePointer::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}
