#include "Rczm.h"
#include <iostream>
registerMooseObject("kitfoxApp", Rczm);

InputParameters
Rczm::validParams()
{
  InputParameters params = DGSolid::validParams();
  // c
  // c Implement rigid/extrinsic cohesive law for crack propergation in DG framework.
  // c
  params.addParam<bool>("tied_contact", false, "preventing penetration after interface opening"
                        " by tied contact");
  params.addClassDescription(
      "Crack propagation by applying rigid/extrinsic cohesive law and DG method.");
  return params;
}

Rczm::Rczm(const InputParameters & parameters)
  : DGSolid(parameters),
    _u_jump(getMaterialPropertyByName<RealVectorValue>("displacement_jump_global")),
    _traction_on_interface(getMaterialPropertyByName<RealVectorValue>("traction_on_interface")),
    _material_tangent_modulus_on_interface(
    getMaterialPropertyByName<RealTensorValue>("material_tangent_modulus_on_interface")),
    _material_softening(getMaterialProperty<bool>("material_softening")),
    _material_softening_old(getMaterialPropertyOld<bool>("material_softening")),
    _dmg_degree_old(getMaterialPropertyOld<Real>("dmg_degree")),
    _tied_contact(getParam<bool>("tied_contact"))
{
}


Real
Rczm::computeQpResidual(Moose::DGResidualType type)
{
  Real r(0.0);
  if (_material_softening_old[_qp])
  {
    // c
    // c materials start softening and in tension state and
    // c call rigid cohesive models
    // c
    Real gap = _u_jump[_qp]*_normals[_qp];
    if(!_tied_contact && _dmg_degree_old[_qp] >= 1) return 0.0;
    if(_tied_contact && _dmg_degree_old[_qp] >= 1 && gap >= 0.0) return 0.0;
    if(_tied_contact && _dmg_degree_old[_qp] >= 1 && gap < 0.0)
       return DGSolid::computeQpResidual(type);
    r = _traction_on_interface[_qp](_component);
    switch (type)
    {
      case Moose::Element:
        r *= -_test[_i][_qp];
        break;
      case Moose::Neighbor:
        r *= _test_neighbor[_i][_qp];
        break;
    }
  }
  else
    r = DGSolid::computeQpResidual(type);
  return r;
}

Real
Rczm::computeQpJacobian(Moose::DGJacobianType type)
{
  Real jac = 0.0;
  if (_material_softening_old[_qp])
  {
    Real gap= _u_jump[_qp]*_normals[_qp];
    if(!_tied_contact && _dmg_degree_old[_qp] >= 1) return 0.0;
    if(_tied_contact && _dmg_degree_old[_qp] >= 1 && gap >= 0.0) return 0.0;
    if(_tied_contact && _dmg_degree_old[_qp] >= 1 && gap < 0.0)
       return DGSolid::computeQpJacobian(type);
    jac = _material_tangent_modulus_on_interface[_qp](_component, _component);
    switch (type)
    {
      case Moose::ElementElement:
        jac *= _test[_i][_qp] * _phi[_j][_qp];
        break;
      case Moose::ElementNeighbor:
        jac *= -_test[_i][_qp] * _phi_neighbor[_j][_qp];
        break;
      case Moose::NeighborElement:
        jac *= -_test_neighbor[_i][_qp] * _phi[_j][_qp];
        break;
      case Moose::NeighborNeighbor:
        jac *= _test_neighbor[_i][_qp] * _phi_neighbor[_j][_qp];
        break;
    }
  }
  else
  {
    jac = DGSolid::computeQpJacobian(type);
  }
  return jac;
}

Real
Rczm::computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar)
{
  for (unsigned int off_diag_component = 0; off_diag_component < _ndisp;
       ++off_diag_component)
    if (jvar == _disp_var[off_diag_component])
    {
      if (_material_softening_old[_qp])
      {
        Real gap= _u_jump[_qp]*_normals[_qp];
        if(!_tied_contact && _dmg_degree_old[_qp] >= 1) return 0.0;
        if(_tied_contact && _dmg_degree_old[_qp] >= 1 && gap >= 0.0) return 0.0;
        if(_tied_contact && _dmg_degree_old[_qp] >= 1 && gap < 0.0)
          return DGSolid::computeQpOffDiagJacobian(type, jvar);
        Real jac = _material_tangent_modulus_on_interface[_qp](_component, off_diag_component);
        switch (type)
        {
          case Moose::ElementElement:
            jac *= _test[_i][_qp] * _phi[_j][_qp];
            break;
          case Moose::ElementNeighbor:
            jac *= -_test[_i][_qp] * _phi_neighbor[_j][_qp];
            break;
          case Moose::NeighborElement:
            jac *= -_test_neighbor[_i][_qp] * _phi[_j][_qp];
            break;
          case Moose::NeighborNeighbor:
            jac *= _test_neighbor[_i][_qp] * _phi_neighbor[_j][_qp];
            break;
        }
      	return jac;
      }
      else
        return DGSolid::computeQpOffDiagJacobian(type, jvar);
    }

  return 0.0;
}
