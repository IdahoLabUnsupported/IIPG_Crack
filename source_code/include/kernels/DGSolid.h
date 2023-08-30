//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "InterfaceKernel.h"
#include "JvarMapInterface.h"
//c
//c An interface shared by two elements may tie two elements together, open, close,
//c slide, and form an aperture within which fuilds may flow due to material failure.
//c This kernel implements DG method for interface behaviors before interfaces fail
//c More precisely, this kernel works on broken mesh but for continuous solutions.

class DGSolid: public JvarMapKernelInterface<InterfaceKernel>
{
public:
  static InputParameters validParams();
  DGSolid(const InputParameters & parameters);
protected:
  //c
  //c  stabilized parameter (/lambda)
  //c
  Real _stabilized_para;
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  RealVectorValue computeAverageTraction();
  RealVectorValue computeDisplacementJump();
  Real computeShearModulus();
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar) override;
  //c
  //c the displacement component this kernel is operating on (0=x, 1=y, 2 =z)
  //c
   //const unsigned int _component;
   unsigned int _component;
  //c
  //c number of displacement components
  //
  const unsigned int _ndisp;
  //c the coupled displacement and neighbor displacement values
  //c
  const std::vector<const VariableValue *> _disp;
  const std::vector<const VariableValue *> _disp_neighbor;
  std::vector<unsigned int> _disp_var;
  //c
  //c define interface stress and tangent modulus on the left & right side ("neighbor");
  //c
  const MaterialProperty<RankTwoTensor> & _stress;
  const MaterialProperty<RankTwoTensor> & _stress_neighbor;
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
  const MaterialProperty<RankFourTensor> & _elasticity_tensor_neighbor;
  const MaterialProperty<RankFourTensor> & _tangent_modulus;
  const MaterialProperty<RankFourTensor> & _tangent_modulus_neighbor;
};
