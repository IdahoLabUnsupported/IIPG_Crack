//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Assembly.h"
#include "RigidBase.h"
InputParameters
RigidBase::validParams()
{
  InputParameters params = InterfaceMaterial::validParams();
  params.addClassDescription("Base class for rigid cohesive zone mateirla models");
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
  return params;
}

RigidBase::RigidBase(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _normals(_assembly.normals()),
    _ndisp(coupledComponents("displacements")),
    _disp(coupledValues("displacements")),
    _disp_neighbor(coupledNeighborValues("displacements")),
    _displacement_jump_global(declareProperty<RealVectorValue>("displacement_jump_global")),
    _traction_on_interface(declarePropertyByName<RealVectorValue>("traction_on_interface")),
    _material_tangent_modulus_on_interface(
        declarePropertyByName<RealTensorValue>("material_tangent_modulus_on_interface")),
    _material_softening(declareProperty<bool>("material_softening")),
    _material_softening_old(getMaterialPropertyOld<bool>("material_softening")),
    _dmg_degree(declareProperty<Real>("dmg_degree")),
    _dmg_degree_old(getMaterialPropertyOld<Real>("dmg_degree"))
{
}

void
RigidBase::computeQpProperties()
{
  //c
  //c computing the displacement jump
  //c
  for (unsigned int i = 0; i < _ndisp; i++)
    _displacement_jump_global[_qp](i) = (*_disp_neighbor[i])[_qp] - (*_disp[i])[_qp];
  //c
  //c if softening/failure
  //c
  _material_softening[_qp] = computeSoftening();
  _dmg_degree[_qp] = computeDamage();
  _traction_on_interface[_qp] = computeTraction();
  _material_tangent_modulus_on_interface[_qp] = computeTractionDerivatives();
}

RealTensorValue
RigidBase::computeThreeOrthoVector(RealVectorValue n_vector)
{
  //c computing three orthoganl coordinate vectors. The Q-matrix formed
  //c by these three vectors also serve a rotation matrix from the global to thelocal.
  //c The last column in the Q matrix stores input n_vector (must be normalized)
  RealVectorValue n_vector_x(0.0);
  RealVectorValue n_vector_y(0.0);
  Real sum_power = n_vector(0)*n_vector(0)+n_vector(1)*n_vector(1);
  if(sum_power < 1.e-8)
  {
    n_vector_x(0) = 1.0;
    n_vector_x(1) = 0.0;
    n_vector_x(2) = 0.0;
    n_vector_y(0) = 0.0;
    n_vector_y(1) = 1.0;
    n_vector_y(2) = 0.0;
  }
  else
  {
   n_vector_x(0) = -n_vector(1);
   n_vector_x(1) = n_vector(0);
   n_vector_x = n_vector_x/sqrt(sum_power);
   n_vector_y(0) = -n_vector(0)*n_vector(2);
   n_vector_y(1) = -n_vector(1)*n_vector(2);
   n_vector_y(2) = sum_power;
   n_vector_y = n_vector_y/sqrt(sum_power);
  }
  RealTensorValue Q_mat;
  for (unsigned int i = 0; i < _ndisp; i++)
  {
   Q_mat(i,0) = n_vector_x(i);
   Q_mat(i,1) = n_vector_y(i);
   Q_mat(i,2) = n_vector(i);
  }
  return Q_mat;
}
