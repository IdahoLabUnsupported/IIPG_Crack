//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RigidBase.h"

/**
 * Implement a rigid/extrinsic version of a linear mixed CZM model
 * cohesive material model: Mode-I dominated model
 **/
class OrtizPandolfiRczm : public RigidBase
{
public:
  static InputParameters validParams();
  OrtizPandolfiRczm(const InputParameters & parameters);
protected:
  virtual RealVectorValue compute_normal_plain();
  virtual Real compute_u_n();
  virtual Real compute_u_t();
  virtual bool computeSoftening() override;
  virtual Real computeDamage() override;
  virtual RealVectorValue computeTangentVector();
  virtual RealVectorValue computeTraction() override;
  virtual RealTensorValue computeTractionDerivatives() override;
  virtual void initQpStatefulProperties() override;
  virtual Real compute_elastic_modulus();
  virtual Real computeEffectiveTraction();
  virtual Real computeEffectiveGap();
  virtual Real computeTn();
  virtual Real compute_equivalent_traction();
  virtual RealVectorValue computeEffectiveGapDerivative();
  ///c input material parameters
  const Real _T_max; //maximum effective traction materials can sustain;
  const Real _delta_max; //effective traction goes down to zero at this point;
  const Real _shear_weight; //shear contribution to failure or softening.
  /// Initial aperture/interface damage degree
  const VariableValue & _dmg_init;
  const MaterialProperty<RankTwoTensor> & _stress;
  const MaterialProperty<RankTwoTensor> & _stress_neighbor;
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
  const MaterialProperty<RankFourTensor> & _elasticity_tensor_neighbor;
};
