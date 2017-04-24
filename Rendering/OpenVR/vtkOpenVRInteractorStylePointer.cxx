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

  this->UpdateRayCallbackCommand = vtkCallbackCommand::New();
  this->UpdateRayCallbackCommand->SetClientData( this );
  this->UpdateRayCallbackCommand->SetCallback( vtkOpenVRInteractorStylePointer::UpdateRay );
  this->AddObserver( vtkCommand::RenderEvent,
    this->UpdateRayCallbackCommand,
    this->Priority );
}

//----------------------------------------------------------------------------
vtkOpenVRInteractorStylePointer::~vtkOpenVRInteractorStylePointer()
{
  this->UpdateRayCallbackCommand->Delete();
}

//----------------------------------------------------------------------------
void vtkOpenVRInteractorStylePointer::UpdateRay( vtkObject*,
  unsigned long,
  void* clientdata,
  void* )
{
  vtkOpenVRInteractorStylePointer* self =
    reinterpret_cast< vtkOpenVRInteractorStylePointer * >( clientdata );

  vtkOpenVRRenderWindowInteractor* iren =
    static_cast< vtkOpenVRRenderWindowInteractor* >( self->Interactor );
  if( !iren )
    {
    vtkErrorWithObjectMacro(self,
      "Unable to get vtkOpenVRRenderWindowInteractor");
    return;
    }

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
  vr::TrackedDeviceIndex_t controllerIndex = vr::k_unTrackedDeviceIndex_Hmd;

  for( uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1;
    unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++ )
    {
    if( pHMD->GetTrackedDeviceClass( unTrackedDevice ) ==
      vr::ETrackedDeviceClass::TrackedDeviceClass_Controller )
      {
      // Found a controller index
      controllerIndex = unTrackedDevice;
      // Check if the controller is on
      vr::ETrackedControllerRole role =
        pHMD->GetControllerRoleForTrackedDeviceIndex( controllerIndex );
      if( role != vr::ETrackedControllerRole::TrackedControllerRole_Invalid )
        {
        break;
        }
      }
    }

  // Compute controller position and world orientation
  double p0[3]; //Ray start point
  double wxyz[4];// Controller orientation
  double dummy_ppos[3];
  vr::TrackedDevicePose_t &tdPose = renWin->GetTrackedDevicePose( controllerIndex );
  iren->ConvertPoseToWorldCoordinates( ren, tdPose, p0, wxyz, dummy_ppos );

  //Compute ray length.
  double p1[3];
  vtkOpenVRPropPicker* picker = static_cast< vtkOpenVRPropPicker* >( self->InteractionPicker );
  picker->PickProp( p0, wxyz, ren, ren->GetViewProps() );//
  if( self->InteractionPicker->GetPath() )
    {
    self->InteractionPicker->GetPickPosition( p1 );
    self->Length = sqrt( vtkMath::Distance2BetweenPoints( p0, p1 ) );
    }
  else
    {
    self->Length = ren->GetActiveCamera()->GetClippingRange()[1];
    }

  ////Set ray length for vtkOpenVRRenderWindow::RenderModels()
  vtkOpenVRModel* controller = renWin->GetTrackedDeviceModel( controllerIndex );
  if( controller )
  {
    controller->SetShowRay( true );
    controller->SetRayLength( self->Length );
  }
}

//----------------------------------------------------------------------------
void vtkOpenVRInteractorStylePointer::Rotate()
{
  //WARNING: Overriding Rotate() to fix vtkInteractorStyle3D::Rotate() which
  //         isn't checking if the actor is dragable before rotating it.
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
