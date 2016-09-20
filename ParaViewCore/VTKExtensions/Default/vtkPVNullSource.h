/*=========================================================================

  Program:   ParaView
  Module:    vtkPVNullSource.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVNullSource - source for NULL data.
// .SECTION Description
// This is a source for null data. Although this actually produces a
// vtkPolyLine paraview blocks all data information from this source resulting
// in it being treated as a null source.

#ifndef vtkPVNullSource_h
#define vtkPVNullSource_h

#include "vtkLineSource.h"
#include "vtkPVVTKExtensionsDefaultModule.h" //needed for exports

class VTKPVVTKEXTENSIONSDEFAULT_EXPORT vtkPVNullSource : public vtkLineSource
{
public:
  static vtkPVNullSource* New();
  vtkTypeMacro(vtkPVNullSource, vtkLineSource);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkPVNullSource();
  ~vtkPVNullSource();

private:
  vtkPVNullSource(const vtkPVNullSource&) VTK_DELETE_FUNCTION;
  void operator=(const vtkPVNullSource&) VTK_DELETE_FUNCTION;

};

#endif

