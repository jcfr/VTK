/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPointSetToPointSetFilter.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1998 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
#include "vtkPointSetToPointSetFilter.h"
#include "vtkPolyData.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"

// Description:
// Construct object.
vtkPointSetToPointSetFilter::vtkPointSetToPointSetFilter()
{
  this->PolyData = vtkPolyData::New();
  this->PolyData->SetSource(this);
  
  this->StructuredGrid = vtkStructuredGrid::New();
  this->StructuredGrid->SetSource(this);
  
  this->UnstructuredGrid = vtkUnstructuredGrid::New();
  this->UnstructuredGrid->SetSource(this);
  
  this->Output = NULL;
}

vtkPointSetToPointSetFilter::~vtkPointSetToPointSetFilter()
{
  this->PolyData->Delete();
  this->StructuredGrid->Delete();
  this->UnstructuredGrid->Delete();
  this->Output = NULL;
}

// Description:
// Specify the input data or filter.
void vtkPointSetToPointSetFilter::SetInput(vtkPointSet *input)
{
  if ( this->Input != input )
    {
    vtkDebugMacro(<<" setting Input to " << (void *)input);
    this->Input = input;
    this->Modified();

    if ( this->Input == NULL ) return;

    if ( ! strcmp(this->Input->GetDataType(),"vtkPolyData") )
      {
      this->Output = this->PolyData;
      }

    else if ( ! strcmp(this->Input->GetDataType(),"vtkStructuredGrid") )
      {
      this->Output = this->StructuredGrid;
      }

    else if ( ! strcmp(this->Input->GetDataType(),"vtkUnstructuredGrid") )
      {
      this->Output = this->UnstructuredGrid;
      }

    else
      {
      vtkErrorMacro(<<"Mismatch in data type");
      }
    }
}


// Description:
// Update input to this filter and the filter itself. Note that we are 
// overloading this method because the output is an abstract dataset type.
// This requires special treatment.
void vtkPointSetToPointSetFilter::Update()
{
  // make sure output has been created
  if ( !this->Output )
    {
    vtkErrorMacro(<< "No output has been created...need to set input");
    return;
    }

  // make sure input is available
  if ( !this->Input )
    {
    vtkErrorMacro(<< "No input...can't execute!");
    return;
    }

  // prevent chasing our tail
  if (this->Updating) return;

  this->Updating = 1;
  this->Input->Update();
  this->Updating = 0;

  if (this->Input->GetMTime() > this->ExecuteTime ||
  this->GetMTime() > this->ExecuteTime )
    {
    if ( this->Input->GetDataReleased() )
      {
      this->Input->ForceUpdate();
      }

    if ( this->StartMethod ) (*this->StartMethod)(this->StartMethodArg);
    // clear points and point data output 
    this->Output->CopyStructure(this->Input);
    this->Execute();
    this->ExecuteTime.Modified();
    this->SetDataReleased(0);
    if ( this->EndMethod ) (*this->EndMethod)(this->EndMethodArg);
    }

  if ( this->Input->ShouldIReleaseData() ) this->Input->ReleaseData();
}

  
// Description:
// Get the output of this filter. If output is NULL, then input hasn't been
// set, which is necessary for abstract filter objects.
vtkPointSet *vtkPointSetToPointSetFilter::GetOutput()
{
  if ( this->Output == NULL )
    {
    vtkErrorMacro(<<"Abstract filters require input to be set before output can be retrieved");
    }
  return (vtkPointSet *)this->Output;
}

// Description:
// Get the output as vtkPolyData. Performs run-time checking.
vtkPolyData *vtkPointSetToPointSetFilter::GetPolyDataOutput() 
{
  return this->PolyData;
}

// Description:
// Get the output as vtkStructuredGrid. Performs run-time checking.
vtkStructuredGrid *vtkPointSetToPointSetFilter::GetStructuredGridOutput()
{
  return this->StructuredGrid;
}

// Description:
// Get the output as vtkUnstructuredGrid. Performs run-time checking.
vtkUnstructuredGrid *vtkPointSetToPointSetFilter::GetUnstructuredGridOutput()
{
  return this->UnstructuredGrid;
}

