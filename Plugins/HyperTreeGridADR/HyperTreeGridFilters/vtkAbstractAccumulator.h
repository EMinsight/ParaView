// SPDX-FileCopyrightText: Copyright (c) Sandia Corporation, Kitware Inc.
// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

/**
 * @class   vtkAbstractAccumulator
 * @brief   accumulates input data given a certain pattern
 *
 * Given input values, this class accumulates data following a certain pattern.
 * This class is typically used for array measurement inside of
 * vtkAbstractArrayMeasurement. It allows to compute estimates
 * over the array while adding values into it on the fly,
 * with the ability of computing the wanted measure in O(1)
 * at any state of the accumulator.
 *
 * Unless specified otherwise, adding a value is constant in time, as well as merging
 * accumulators of the same type.
 *
 */

#ifndef vtkAbstractAccumulator_h
#define vtkAbstractAccumulator_h

#include "vtkFiltersHyperTreeGridADRModule.h" // For export macro
#include "vtkObject.h"

#include <functional> // for std::function

class vtkDataArray;
class vtkDoubleArray;

class VTKFILTERSHYPERTREEGRIDADR_EXPORT vtkAbstractAccumulator : public vtkObject
{
public:
  vtkAbstractTypeMacro(vtkAbstractAccumulator, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  static vtkAbstractAccumulator* New();

  ///@{
  /**
   * Methods for adding data to the accumulator.
   */
  virtual void Add(vtkDataArray* data, vtkDoubleArray* weights = nullptr);
  virtual void Add(const double* data, vtkIdType numberOfElements = 1, double weight = 1.0);
  virtual void Add(vtkAbstractAccumulator* accumulator) = 0;
  virtual void Add(double value, double weight) = 0;
  ///@}

  /**
   * Returns true if the parameters of accumulator is the same as the ones of this
   */
  virtual bool HasSameParameters(vtkAbstractAccumulator* accumulator) const = 0;
  ///@}

  /**
   * Accessor on the accumulated value.
   */
  virtual double GetValue() const = 0;

  virtual void DeepCopy(vtkObject*) {}
  virtual void ShallowCopy(vtkObject*) {}
  virtual void Initialize() {}

protected:
  ///@{
  /**
   * Default constructors and destructors
   */
  vtkAbstractAccumulator();
  ~vtkAbstractAccumulator() override = default;
  ///@}

  /**
   * Lambda expression converting vectors to scalars. Default function is regular L2 norm.
   */
  std::function<double(const double*, vtkIdType)> ConvertVectorToScalar;

private:
  vtkAbstractAccumulator(const vtkAbstractAccumulator&) = delete;
  void operator=(const vtkAbstractAccumulator&) = delete;
};

#endif
