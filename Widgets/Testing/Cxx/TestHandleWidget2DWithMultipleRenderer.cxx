/*=========================================================================

  Program:   Visualization Toolkit
  Module:    TestHandleWidget2D.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
//
// This example tests the vtkHandleWidget with a 2D representation

// First include the required header files for the VTK classes we are using.
#include "vtkHandleWidget.h"
#include "vtkPointHandleRepresentation2D.h"
#include "vtkCursor2D.h"
#include "vtkCoordinate.h"
#include "vtkDiskSource.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkActor2D.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkCommand.h"
#include "vtkInteractorEventRecorder.h"
#include "vtkRegressionTestImage.h"
#include "vtkDebugLeaks.h"
#include "vtkSmartPointer.h"
#include "vtkPolyDataMapper.h"
#include "vtkSphereSource.h"
#include "vtkProperty2D.h"
#include "vtkInteractorStyleTrackballCamera.h"

// Convenient macro
#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

#define RENDERER_COUNT 6

// This does the actual work: updates the probe.
// Callback for the interaction
class vtkHandleCallback : public vtkCommand
{
public:
  static vtkHandleCallback *New()
    { return new vtkHandleCallback; }
  virtual void Execute(vtkObject *caller, unsigned long, void*)
    {
      vtkRenderWindowInteractor *interactor =
        reinterpret_cast<vtkRenderWindowInteractor*>(caller);
      std::string key = interactor->GetKeySym();
      bool update = false;
      if (key.compare("Right") == 0)
        {
        this->CurrentRenderer++;
        if(this->CurrentRenderer == RENDERER_COUNT) { this->CurrentRenderer = 0; }
        update = true;
        }
      else if (key.compare("Left") == 0)
        {
        this->CurrentRenderer--;
        if(this->CurrentRenderer == -1) { this->CurrentRenderer = RENDERER_COUNT - 1; }
        update = true;
        }

      if (update)
        {
        std::cout << "Move widget to renderer:" << this->CurrentRenderer << std::endl;
        this->Widget->SetEnabled(0);
        this->Widget->SetDefaultRenderer(0);
        this->Widget->SetCurrentRenderer(this->Renderers[this->CurrentRenderer]);
        this->Widget->SetEnabled(1);
        this->RenderWindow->Render();
        }
    }
  vtkHandleCallback():Widget(0), CurrentRenderer(0), RenderWindow(0)
    { for(int i=0; i < RENDERER_COUNT; ++i){this->Renderers[i] = 0;} }
  vtkRenderer* Renderers[RENDERER_COUNT];
  vtkHandleWidget* Widget;
  int CurrentRenderer;
  vtkRenderWindow * RenderWindow;
};

int TestHandleWidget2DWithMultipleRenderer( int argc, char *argv[] )
{
  // Create the RenderWindow, Renderer
  VTK_CREATE(vtkRenderWindow, renWin);
  renWin->SetSize(600, 500);

  // Create interactor
  VTK_CREATE(vtkRenderWindowInteractor, iren);
  VTK_CREATE(vtkInteractorStyleTrackballCamera, istyle);
  iren->SetInteractorStyle(istyle);
  iren->SetRenderWindow(renWin);

  //
  // Let's now create 6 renderers, the viewport of each
  // one of these renderer will be set according to the layout
  // described below:
  //
  // |--------|--------|--------|
  // |  ren4  |  ren5  |  ren6  |
  // |        |        |        |
  // |--------|--------|--------|
  // |  ren1  |  ren2  |  ren3  |
  // |        |        |        |
  // |--------|--------|--------|
  //
  // Each rendere will have a size of 200*250
  //


  double viewportWidth = 1.0 / 3;
  double viewportHeight = 0.5;

  VTK_CREATE(vtkRenderer, ren1);
  VTK_CREATE(vtkRenderer, ren2);
  VTK_CREATE(vtkRenderer, ren3);
  VTK_CREATE(vtkRenderer, ren4);
  VTK_CREATE(vtkRenderer, ren5);
  VTK_CREATE(vtkRenderer, ren6);
  vtkRenderer* renderers[6] = {ren1, ren2, ren3, ren4, ren5, ren6 };

  // Add renderer
  for(int i=0; i < RENDERER_COUNT; ++i)
    {
    renWin->AddRenderer(renderers[i]);
    renderers[i]->GetActiveCamera(); // Make sure each renderer has an active camera
    }

  ren1->SetBackground(0.94, 0, 0.58); // pink
  double xMin = 0.0, yMin = 0.0;
  ren1->SetViewport( xMin, yMin, (xMin + viewportWidth), (yMin + viewportHeight));

  ren2->SetBackground(0.94, 0, 0.10); // red
  xMin = viewportWidth, yMin = 0.0;
  ren2->SetViewport( xMin, yMin, (xMin + viewportWidth), (yMin + viewportHeight));

  ren3->SetBackground(0.58, 0, 0.94); // purple
  xMin = viewportWidth * 2, yMin = 0.0;
  ren3->SetViewport( xMin, yMin, (xMin + viewportWidth), (yMin + viewportHeight));

  ren4->SetBackground(0.10, 0.94, 0); // green 2
  xMin = 0.0, yMin = viewportHeight;
  ren4->SetViewport( xMin, yMin, (xMin + viewportWidth), (yMin + viewportHeight));

  ren5->SetBackground(0, 0.58, 0.94); // light blue
  xMin = viewportWidth, yMin = viewportHeight;
  ren5->SetViewport( xMin, yMin, (xMin + viewportWidth), (yMin + viewportHeight));

  ren6->SetBackground(0, 0.10, 0.94); // dark blue
  xMin = viewportWidth * 2, yMin = viewportHeight;
  ren6->SetViewport( xMin, yMin, (xMin + viewportWidth), (yMin + viewportHeight));

  //
  // Create a sphere
  //
  VTK_CREATE(vtkSphereSource, sphere);
  sphere->SetRadius(0.25);
  VTK_CREATE(vtkPolyDataMapper, sphereMapper);
  sphereMapper->SetInputConnection(sphere->GetOutputPort());
  VTK_CREATE(vtkActor, sphereActor);
  sphereActor->SetMapper(sphereMapper);

  //
  // Add sphere to all renderers
  //
  for(int i=0; i < RENDERER_COUNT; ++i)
    {
    renderers[i]->AddActor(sphereActor);
    }

  //
  // Create a vtkHandleWidget and associated it with "ren1"
  //
  VTK_CREATE(vtkPointHandleRepresentation2D, handle1);
  handle1->GetProperty()->SetColor(0,0,0);
  handle1->SetHandleSize(10);
  VTK_CREATE(vtkHandleWidget, handleWidget);
  handleWidget->CreateDefaultRepresentation();
  handleWidget->SetRepresentation(handle1);
  handleWidget->SetInteractor(iren);
  handleWidget->SetCurrentRenderer(ren1);
  handleWidget->SetEnabled(1);

  // Catch KeyPress event
  VTK_CREATE(vtkHandleCallback, callback);
  iren->AddObserver(vtkCommand::KeyPressEvent, callback);
  callback->Widget = handleWidget;
  callback->CurrentRenderer = 0;
  callback->RenderWindow = renWin;
  for(int i=0; i < RENDERER_COUNT; ++i) { callback->Renderers[i] = renderers[i]; }

  //
  // Render
  //
  iren->Initialize();
  renWin->Render();
  handleWidget->On();

  int retVal = vtkRegressionTestImage( renWin );
  if ( retVal == vtkRegressionTester::DO_INTERACTOR)
    {
    iren->Start();
    }

  return !retVal;
}
