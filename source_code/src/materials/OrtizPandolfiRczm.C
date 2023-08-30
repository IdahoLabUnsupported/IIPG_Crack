//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "OrtizPandolfiRczm.h"
#include <iostream>
using namespace std;
registerMooseObject("TensorMechanicsApp", OrtizPandolfiRczm);

InputParameters
OrtizPandolfiRczm::validParams()
{
  InputParameters params = RigidBase::validParams();
  params.addClassDescription("Linear mixed extrinsic cohesive law of "
                             "Ortiz-Pandolfi model");
  params.addRequiredParam<Real>("maximum_effective_traction",
                                "The maximum effective traction the interface can sustain");
  params.addRequiredParam<Real>("maximum_effective_gap",
                                "The value of effective gap at which the effective traction goes down to zero");
  params.addParam<Real>("shear_weight",
                               0,
                                "The coeffiecient of shear contribution on softening");
  params.addCoupledVar(
      "damage_degree_init",
      0.0,
      "The initial damage degree.  This must be a real number or a constant monomial variable "
      "and must be less or equal one"
      "(not a linear lagrange or other type of variable)");
  return params;
}

OrtizPandolfiRczm::OrtizPandolfiRczm(const InputParameters & parameters)
  : RigidBase(parameters),
    _T_max(getParam<Real>("maximum_effective_traction")),
    _delta_max(getParam<Real>("maximum_effective_gap")),
    _shear_weight(getParam<Real>("shear_weight")),
    _dmg_init(coupledValue("damage_degree_init")),
    _stress(getMaterialProperty<RankTwoTensor>("stress")),
    _stress_neighbor(getNeighborMaterialProperty<RankTwoTensor>("stress")),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>("elasticity_tensor")),
    _elasticity_tensor_neighbor(getNeighborMaterialPropertyByName<RankFourTensor>("elasticity_tensor"))
{
}

Real
OrtizPandolfiRczm::compute_elastic_modulus()
{
 //c
 //c retrieve Young's modulus from elasticity tensor
//c and take the larger one from two sides.
 //c
 Real modulus_left(0.0);
 Real modulus_right(0.0);
 modulus_left = _elasticity_tensor[_qp](0,0,0,0);
 modulus_right = _elasticity_tensor_neighbor[_qp](0,0,0,0);
 if(modulus_left > modulus_right)
    return modulus_left;
 else
    return modulus_right;
}


void
OrtizPandolfiRczm::initQpStatefulProperties()
{
  _material_softening[_qp] = false;
 // cout << "dmg_ini = " << _dmg_init[0] << ".\n";
  _dmg_degree[_qp] = _dmg_init[0];
  if(_dmg_degree[_qp] > 0)
  {
    _material_softening[_qp] = true;
    _traction_on_interface[_qp] = 0.0;
  }
}

RealVectorValue
OrtizPandolfiRczm::compute_normal_plain()
{
  //c
  //c transfer point vector to plain vector
  //c
  RankTwoTensor ident = RankTwoTensor::Identity();
  RealVectorValue normal_plain = ident*_normals[_qp];
  return normal_plain;
}

Real
OrtizPandolfiRczm::compute_u_n()
{
  RealVectorValue jump_u = _displacement_jump_global[_qp];
  RealVectorValue normal_plain = OrtizPandolfiRczm::compute_normal_plain();
  Real u_n(0.0);
  for (unsigned int i = 0; i < 3; i++)
    u_n += jump_u(i)*normal_plain(i);
  return u_n;
}

Real
OrtizPandolfiRczm::compute_u_t()
{
  //c
  //c displacement component within interface plane
  //c
  RealVectorValue jump_u = _displacement_jump_global[_qp];
  Real u_n = OrtizPandolfiRczm::compute_u_n();
  RealVectorValue normal_plain = OrtizPandolfiRczm::compute_normal_plain();
  RealVectorValue u_t_vector = jump_u-u_n*normal_plain;
  Real u_t(0.0);
  for (unsigned int i = 0; i < 3; i++)
  {
    u_t += u_t_vector(i)*u_t_vector(i);
  }
  u_t = sqrt(u_t);
  return u_t;
}

Real
OrtizPandolfiRczm::computeTn()
{
  //
  // This computes the normal traction from bulk element surfaces NOT
  // from cohesive law
  //
    RealVectorValue traction = (_stress[_qp]+_stress_neighbor[_qp])*_normals[_qp]/2.0;
    Real T_n = traction*_normals[_qp];
 return T_n;
}

Real
OrtizPandolfiRczm::compute_equivalent_traction()
{
  //
  //The equivalent traction (NOT effective traction) computed from bulk element surfaces
  //(NOT from cohesive law). It will be used to check if interfaces start to soften.
  //
    RealVectorValue traction = (_stress[_qp]+_stress_neighbor[_qp])*_normals[_qp]/2.0;
    Real T_n = traction*_normals[_qp];
    RealVectorValue T_tangent_vector = traction-T_n*_normals[_qp];
    Real T_t = sqrt(T_tangent_vector*T_tangent_vector);
    Real T_eqpl(0.0);
    if(_shear_weight <= 0.0)
       T_eqpl = abs(T_n);
    else
       T_eqpl = sqrt(T_t*T_t/_shear_weight/_shear_weight+T_n*T_n);
    return T_eqpl;
}


Real
OrtizPandolfiRczm::computeEffectiveTraction()
{
  //
  //The effective traction computed from cohesive law using [u] and cohesive
  //material data
  //
  Real T_effective(0.0);
  Real delta_effective = OrtizPandolfiRczm::computeEffectiveGap();
  if(delta_effective > _delta_max)
     T_effective = 0.0;
  else
     T_effective = _T_max*(1.0-delta_effective/_delta_max);
 return T_effective;
}

Real
OrtizPandolfiRczm::computeEffectiveGap()
{
  Real u_n = OrtizPandolfiRczm::compute_u_n();
  Real u_t = OrtizPandolfiRczm::compute_u_t();
  Real delta_effective(0.0);
  delta_effective = sqrt(u_n*u_n+u_t*u_t*_shear_weight*_shear_weight);
 return delta_effective;
}

RealVectorValue
OrtizPandolfiRczm::computeEffectiveGapDerivative()
{
 RealVectorValue ducdjump(0.0);
 Real delta_effective = OrtizPandolfiRczm::computeEffectiveGap();
 if(delta_effective > 0)
 {
   Real u_t = OrtizPandolfiRczm::compute_u_t();
   Real u_n = OrtizPandolfiRczm::compute_u_n();
   RealVectorValue m = OrtizPandolfiRczm::computeTangentVector();
   ducdjump = (_shear_weight*_shear_weight*u_t*m+u_n*_normals[_qp])/delta_effective;
 }
 return ducdjump;
}

bool
OrtizPandolfiRczm::computeSoftening()
{
  bool softening_starts(false);
  if(_dmg_init[0]>0)
  {                     
    softening_starts = true; 
    return softening_starts;
  }
  Real Tn = OrtizPandolfiRczm::computeTn();
  if(_material_softening_old[_qp] )
     softening_starts = true;
  else
  {
   Real T_eqpl = OrtizPandolfiRczm::compute_equivalent_traction();
   if(T_eqpl > _T_max && Tn > 0.0)
   {
       softening_starts = true;
    }
  }
 return softening_starts;
}

Real
OrtizPandolfiRczm::computeDamage()
{
 Real dmg = _dmg_degree_old[_qp];
 if(dmg < _dmg_init[0]) dmg = _dmg_init[0];
 if(_material_softening_old[_qp])
 {
  Real delta_effective = OrtizPandolfiRczm::computeEffectiveGap();
  Real dmg_new = delta_effective/_delta_max+_dmg_init[0];
  if(dmg_new > _dmg_degree_old[_qp]) dmg = dmg_new;
   if(dmg > 1.0) dmg = 1.0;
  }
 return dmg;
}

RealVectorValue
OrtizPandolfiRczm::computeTangentVector()
{
  //c The tangent vector is computed from jump displacement vector and normal vector of
  //c the interface. Special handling for purely normal mode where the
  //c the tangent vector is undetermined.
  //c compute t_vector
  RealVectorValue u_t_vector,t_vector;
  RealVectorValue jump_u = _displacement_jump_global[_qp];
  Real u_n = OrtizPandolfiRczm::compute_u_n();
  Real u_t = OrtizPandolfiRczm::compute_u_t();
  RealVectorValue normal_plain = OrtizPandolfiRczm::compute_normal_plain();
  //c check if u_t=0
  if(u_t > 1.0e-16)
  {
    u_t_vector = jump_u - u_n*normal_plain;
    t_vector=u_t_vector/u_t;
  }
  else
  {
  RealTensorValue Q_mat = RigidBase::computeThreeOrthoVector(_normals[_qp]);
  t_vector(0) = Q_mat(0,0);
  t_vector(1) = Q_mat(1,0);
  t_vector(2) = Q_mat(2,0);
  }
 return t_vector;
}

RealVectorValue
OrtizPandolfiRczm::computeTraction()
{
  RealVectorValue interface_traction(0.0);
  RealVectorValue jump_u = _displacement_jump_global[_qp];
  Real u_n = OrtizPandolfiRczm::compute_u_n();
  Real T_effective = OrtizPandolfiRczm::computeEffectiveTraction();
  Real delta_effective = OrtizPandolfiRczm::computeEffectiveGap();
  if(delta_effective >= _delta_max)
    return interface_traction;
  if(delta_effective < 1.0e-15)
     delta_effective = 1.0e-15;
  interface_traction = T_effective/delta_effective*(_shear_weight*_shear_weight*jump_u+
                        (1.0-_shear_weight*_shear_weight)*u_n*_normals[_qp]);
  return interface_traction;
}


RealTensorValue
OrtizPandolfiRczm::computeTractionDerivatives()
{
  RealTensorValue tangent_modulus_on_interface;
  tangent_modulus_on_interface.zero();
  Real delta_effective = OrtizPandolfiRczm::computeEffectiveGap();
  if(delta_effective >= _delta_max)
  {
    return tangent_modulus_on_interface;
  }
   RealVectorValue ducdjump = OrtizPandolfiRczm::computeEffectiveGapDerivative();
   RealVectorValue jump_u = _displacement_jump_global[_qp];
   Real u_n = OrtizPandolfiRczm::compute_u_n();
   RankTwoTensor ident = RankTwoTensor::Identity();
   RealVectorValue normal_plain = OrtizPandolfiRczm::compute_normal_plain();
   if(delta_effective < 1.0e-15) delta_effective = 1.0e-15;
   for (unsigned int i = 0; i < 3; i++)
     for (unsigned int j = 0; j < 3; j++)
     {
       tangent_modulus_on_interface(i,j) = -_T_max/delta_effective/delta_effective*
                                           (_shear_weight*_shear_weight*jump_u(i)+
                                            (1.0-_shear_weight*_shear_weight)*u_n*normal_plain(i))*
                                            ducdjump(j);
       tangent_modulus_on_interface(i,j) += _T_max*(1.0/delta_effective-1.0/_delta_max)*
                                           (_shear_weight*_shear_weight*ident(i,j)+
                                            (1.0-_shear_weight*_shear_weight)*normal_plain(i)*normal_plain(j));
      }
   return tangent_modulus_on_interface;
}
