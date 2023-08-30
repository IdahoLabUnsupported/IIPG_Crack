#include "DGSolid.h"
#include "ElasticityTensorTools.h"
registerMooseObject("kitfoxApp", DGSolid);

InputParameters
DGSolid::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  //
  //  starting interface and DG related  parameters
  //
  params.addClassDescription("DG methods for interface-oriented solid mechanics");
  //
  //  Define DG stablized parameter and set it to 10.0 for default
  //
  params.addRequiredCoupledVar("displacements", "the string containing displacement variables");
  params.addParam<Real>("stabilized_para", 10.0, "DG stabilized parameter");
  return params;
}

DGSolid::DGSolid(const InputParameters & parameters)
  : JvarMapKernelInterface<InterfaceKernel>(parameters),
    //
    //  get the DG stabilized_para from input
    //
    _stabilized_para(getParam<Real>("stabilized_para")),
    _ndisp(coupledComponents("displacements")),
    _disp(coupledValues("displacements")),
    _disp_neighbor(coupledNeighborValues("displacements")),
    _disp_var(_ndisp),
    _stress(getMaterialPropertyByName<RankTwoTensor>("stress")),
    _stress_neighbor(getNeighborMaterialPropertyByName<RankTwoTensor>("stress")),
    //
    //  the following elasticity tensors are used to retrieve the shear modulus;
    //  It will be used to re-scale the DG stabilized parameter for unit consisitency.
    //
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>("elasticity_tensor")),
    _elasticity_tensor_neighbor(
        getNeighborMaterialPropertyByName<RankFourTensor>("elasticity_tensor")),
    //
    //  the following tangent modulus is refered to consistent tangent operator provided,
    //  for example, by plasticity problems.
    //
    _tangent_modulus(getMaterialProperty<RankFourTensor>("Jacobian_mult")),
    _tangent_modulus_neighbor(getNeighborMaterialProperty<RankFourTensor>("Jacobian_mult"))
{
  for (const auto i : make_range(_ndisp))
  {
    _disp_var[i] = coupled("displacements", i);
    if (_disp_var[i] == _var.number())
      _component = i;
  }
}

RealVectorValue
DGSolid::computeAverageTraction()
{
  //
  //  compute the average of traction across an interface
  //
  RealVectorValue traction;
  traction = (_stress[_qp] + _stress_neighbor[_qp]) * _normals[_qp] / 2.0;
  return traction;
}

RealVectorValue
DGSolid::computeDisplacementJump()
{
  //
  //  compute the jump of displacements across an interface
  //
  RealVectorValue u_jump;
  for (unsigned int i = 0; i < _ndisp; i++)
    u_jump(i) = -(*_disp[i])[_qp] + (*_disp_neighbor[i])[_qp];
  return u_jump;
}

Real
DGSolid::computeShearModulus()
{
  //
  //  retrieve shear-like modulus from elasticity tensor
  //
  //
  //  Always use the smaller shear-like modulus to contribute DG stabilization
  //
  Real shear_modulus_left(0.0);
  Real shear_modulus_right(0.0);
  shear_modulus_left = (_elasticity_tensor[_qp](0, 1, 0, 1) + _elasticity_tensor[_qp](0, 2, 0, 2) +
                        _elasticity_tensor[_qp](1, 2, 1, 2)) /
                       3.0;
  shear_modulus_right =
      (_elasticity_tensor_neighbor[_qp](0, 1, 0, 1) + _elasticity_tensor_neighbor[_qp](0, 2, 0, 2) +
       _elasticity_tensor_neighbor[_qp](1, 2, 1, 2)) /
      3.0;
  if (shear_modulus_left > shear_modulus_right)
    return shear_modulus_right;
  else
    return shear_modulus_left;
}

Real
DGSolid::computeQpResidual(Moose::DGResidualType type)
{
  //
  //  this computes the residual contributed from DG interface only
  // based on the IIPG method;
  //  traction+stabilized_para*G*[u]/h;
  //  G: shear-like modulus;
  //  h: element size (evaluated by square root of face area).
  //
  Real r(0.0);
  RealVectorValue traction = DGSolid::computeAverageTraction();
  RealVectorValue u_jump = DGSolid::computeDisplacementJump();
  Real shear_modulus = DGSolid::computeShearModulus();
  //
  //  Adding traction on surface based on IIPG formulation
  //
  r -= traction(_component);
  //
  //  Adding stabilized term
  //
  r += _stabilized_para * shear_modulus / sqrt(_current_side_volume) * u_jump(_component);
  switch (type)
  {
    //
    //  [test_right-test_left]*T where T represents the traction;
    //  test_left==test==test_primary; test_right=test_neighbor==test_secondary.
    //
    //  for left element
    //
    case Moose::Element:
      r *= -_test[_i][_qp];
      break;
    //
    //  for right element
    //
    case Moose::Neighbor:
      r *= _test_neighbor[_i][_qp];
      break;
  }
  return r;
}

Real
DGSolid::computeQpJacobian(Moose::DGJacobianType type)
{
  Real jac(0.0);
  //
  //  IIPG only;
  //
  //   This rouine adds Jacobian contributed from:
  //    (1) major face integeral:
  //     -\int 1/2{\sigma_left+sigma_righ}_normals\cdot [v] ds;
  //    (2) stabilized term:
  //      \lambda\int [u] \cdot[v] ds
  //
  Real shear_modulus = DGSolid::computeShearModulus();
  Real DG_para = _stabilized_para * shear_modulus / sqrt(_current_side_volume);
  RealVectorValue vxn;
  RealVectorValue grad_phi;
  switch (type)
  {
    //
    //   for K^(LL)
    //
    case Moose::ElementElement:
      //
      //  generate a tensor product by test function and normal vector:
      //
      vxn = _test[_i][_qp] * _normals[_qp];
      grad_phi = _grad_phi[_j][_qp];
      //
      //  add Jacobian contributed from major face integral
      //
      jac += (ElasticityTensorTools::elasticJacobian(
                 _tangent_modulus[_qp], _component, _component, vxn, grad_phi)) *
             0.5;
      //
      //  add Jacobian contributed from DG stabilized term
      //
      jac += DG_para * _test[_i][_qp] * _phi[_j][_qp];
      break;
      //
      //   for K^(LR)
      //
    case Moose::ElementNeighbor:
      vxn = _test[_i][_qp] * _normals[_qp];
      grad_phi = _grad_phi_neighbor[_j][_qp];
      //
      //  add Jacobian contributed from major face integral
      //
      jac += (ElasticityTensorTools::elasticJacobian(
                 _tangent_modulus_neighbor[_qp], _component, _component, vxn, grad_phi)) *
             0.5;
      jac -= DG_para * _test[_i][_qp] * _phi_neighbor[_j][_qp];
      break;
    //
    //   for K^(RL)
    //
    case Moose::NeighborElement:
      vxn = _test_neighbor[_i][_qp] * _normals[_qp];
      grad_phi = _grad_phi[_j][_qp];
      //
      //  add Jacobian contributed from major face integral
      //
      jac -= (ElasticityTensorTools::elasticJacobian(
                 _tangent_modulus[_qp], _component, _component, vxn, grad_phi)) *
             0.5;
      jac -= DG_para * _test_neighbor[_i][_qp] * _phi[_j][_qp];
      break;
    //
    //   for K^(RR)
    //
    case Moose::NeighborNeighbor:
      vxn = _test_neighbor[_i][_qp] * _normals[_qp];
      grad_phi = _grad_phi_neighbor[_j][_qp];
      //
      //  add Jacobian contributed from major face integral
      //
      jac -= (ElasticityTensorTools::elasticJacobian(
                 _tangent_modulus_neighbor[_qp], _component, _component, vxn, grad_phi)) *
             0.5;
      jac += DG_para * _test_neighbor[_i][_qp] * _phi_neighbor[_j][_qp];
      break;
  }
  return jac;
}

Real
DGSolid::computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar)
{
  Real jac(0.0);
  //
  //  note: the stabilized term will not contribute the offdiagonal Jacobian!
  //
  //
  //  find the displacement component associated to jvar
  //
  //
  //
  //
  //
  //
  //
  for (auto beta : make_range(_ndisp))
    if (jvar == _disp_var[beta])
    {
     auto off_diag_component = beta;
     RealVectorValue vxn;
     RealVectorValue grad_phi;
    switch (type)
    {
      //
      //   for K^(LL)
      //
      case Moose::ElementElement:
        //
        //  grenerate a tensor product by test function and normal vector:
        //
        vxn = _test[_i][_qp] * _normals[_qp];
        grad_phi = _grad_phi[_j][_qp];
        //
        //  add Jacobian contributed from major face integral
        //
        jac += (ElasticityTensorTools::elasticJacobian(
                   _tangent_modulus[_qp], _component, off_diag_component, vxn, grad_phi)) *
               0.5;
        //
        //  add Jacobian contributed from DG stabilized term
        //
        // jac += DG_para*_test[_i][_qp]*_phi[_j][_qp];
        break;
      //
      //   for K^(LR)
      //
      case Moose::ElementNeighbor:
        vxn = _test[_i][_qp] * _normals[_qp];
        grad_phi = _grad_phi_neighbor[_j][_qp];
        //
        //  add Jacobian contributed from major face integral
        //
        jac += (ElasticityTensorTools::elasticJacobian(
                   _tangent_modulus_neighbor[_qp], _component, off_diag_component, vxn, grad_phi)) *
               0.5;
        // jac -= DG_para*_test[_i][_qp]*_phi_neighbor[_j][_qp];
        break;
      //
      //   for K^(RL)
      //
      case Moose::NeighborElement:
        vxn = _test_neighbor[_i][_qp] * _normals[_qp];
        grad_phi = _grad_phi[_j][_qp];
        //
        //  add Jacobian contributed from major face integral
        //
        jac -= (ElasticityTensorTools::elasticJacobian(
                   _tangent_modulus[_qp], _component, off_diag_component, vxn, grad_phi)) *
               0.5;
        // jac -= DG_para*_test_neighbor[_i][_qp]*_phi[_j][_qp];
        break;
      //
      //   for K^(RR)
      //
      case Moose::NeighborNeighbor:
        vxn = _test_neighbor[_i][_qp] * _normals[_qp];
        grad_phi = _grad_phi_neighbor[_j][_qp];
        //
        //  add Jacobian contributed from major face integral
        //
        jac -= (ElasticityTensorTools::elasticJacobian(
                   _tangent_modulus_neighbor[_qp], _component, off_diag_component, vxn, grad_phi)) *
               0.5;
        // jac += DG_para*_test_neighbor[_i][_qp]*_phi_neighbor[_j][_qp];
        break;
    }
    return jac;
  }
  return 0.0;
}

