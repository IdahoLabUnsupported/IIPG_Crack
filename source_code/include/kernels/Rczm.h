//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DGSolid.h"

//c This kernel implements crack propagation governed by rigid/extrinsic cohesive law (Rczm)
//c through applying Discontinous Galerkin (DG) method: before softening,
//c solve a continuous problems via a standard DG method but apply ridid cohesive law after
//c material softens (failure).
class Rczm : public DGSolid
{
public:
  static InputParameters validParams();
  Rczm(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type);
  virtual Real computeQpJacobian(Moose::DGJacobianType type);
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar);
  //c
  //c define traction and tangent modulus variables on interfaces
  //c
  const MaterialProperty<RealVectorValue> & _u_jump;
  const MaterialProperty<RealVectorValue> & _traction_on_interface;
  const MaterialProperty<RealTensorValue> & _material_tangent_modulus_on_interface;
  //c
  //c material softening flag
  //c
  const MaterialProperty<bool> & _material_softening;
  const MaterialProperty<bool> & _material_softening_old;
  const MaterialProperty<Real> & _dmg_degree_old;
  const bool _tied_contact;
};
