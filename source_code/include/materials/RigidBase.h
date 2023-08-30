//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceMaterial.h"
//c
//c This is the base Material class for implementing a rigid traction separation material model.
//c
class RigidBase: public InterfaceMaterial
{
public:
  static InputParameters validParams();
  RigidBase(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  //c normal to the interface
  const MooseArray<Point> & _normals;

  //c number of displacement components
  const unsigned int _ndisp;

  //c the coupled displacement and neighbor displacement values
  //c
  const std::vector<const VariableValue *> _disp;
  const std::vector<const VariableValue *> _disp_neighbor;
  //c

  //c method returning the traction in the interface coordinate system.
  virtual RealVectorValue computeTraction() = 0;

  //c method returning the traction derivitaves wrt local displacement jump.
  virtual RealTensorValue computeTractionDerivatives() = 0;

  virtual RealTensorValue computeThreeOrthoVector(RealVectorValue n_vector);
  //c
  //c the displacement jump in global
  //c
  MaterialProperty<RealVectorValue> & _displacement_jump_global;
  //c
  //c the value of the traction in global
  //c
  MaterialProperty<RealVectorValue> & _traction_on_interface;
  //c
  //c
  //c
  MaterialProperty<RealTensorValue> & _material_tangent_modulus_on_interface;
  //c
  //c method returning the softening flag.
  //c
  virtual bool computeSoftening() = 0;
  //c
  //c method returning material damage
  //c
  virtual Real computeDamage() = 0;
  //c
  //c material softening flag
  //c
  MaterialProperty<bool> & _material_softening;
  const MaterialProperty<bool> & _material_softening_old;
  //c
  //c material damage
  //c
  MaterialProperty<Real> & _dmg_degree;
  const MaterialProperty<Real> & _dmg_degree_old;
};
