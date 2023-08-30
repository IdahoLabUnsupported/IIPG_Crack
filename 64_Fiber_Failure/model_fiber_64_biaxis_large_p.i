[Mesh]
  [fiber_model_mesh]
    type = FileMeshGenerator
    file = fiber_64.e
  []
  [breakmesh]
    type = BreakMeshByBlockGenerator
    input = fiber_model_mesh
    block_pars = 'Matrix Fiber'
    interface_name = matrix_fiber
  []

[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_xz stress_yz hydrostatic_stress vonmises_stress'
    extra_vector_tags = react
    material_output_family = MONOMIAL
    material_output_order = FIRST
  []
[]
[InterfaceKernels]
  [ifk_x]
    type = Rczm
    component = 0
    variable = 'disp_x'
    neighbor_var = 'disp_x'
    displacements = 'disp_x disp_y disp_z'
    #boundary = 'interface'
    boundary = 'matrix_fiber'
    stabilized_para =10
  []
  [ifk_y]
  type = Rczm
  component = 1
  variable = 'disp_y'
  neighbor_var = 'disp_y'
    displacements = 'disp_x disp_y disp_z'
  #boundary = 'interface'
  boundary = 'matrix_fiber'
  stabilized_para = 10
  []
  [ifk_z]
  type = Rczm
  component = 2
  variable = 'disp_z'
  neighbor_var = 'disp_z'
    displacements = 'disp_x disp_y disp_z'
  #boundary = 'interface'
   boundary = 'matrix_fiber'
  stabilized_para = 10
  []
[]

[Materials]
  [Elasticity_Matrix]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 0.003
    poissons_ratio = 0.3
    block = 'Matrix'
  []
  [Elasticity_Fiber]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 0.072
    poissons_ratio = 0.21
    block = 'Fiber'
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = 'Matrix Fiber'
  []
  [./Rigid_czm_model]
    type = OrtizPandolfiRczm
    boundary = 'matrix_fiber'
    maximum_effective_traction = 8e-6
    maximum_effective_gap = 0.01
    shear_weight = 0.707
    displacements = 'disp_x disp_y disp_z'
    outputs = all
  [../]
[]

[Functions]
  [topfunc]
    type = ParsedFunction
    value = 't'
    #value = '0.1*sin(3.1415926/2.0*t)'
  []
[]

[BCs]
  # Prescribed displacements at three boundary surfaces
  # Enforce plain strain condition by constrainting
  # zero-displacement on z_plus and z-minus

  [roller_at_X_minus]
    type = DirichletBC
    preset = true
    variable = disp_x
    value = 0
    boundary = 'X_Minus'
  []

  [roller_at_Y_minus]
    type = DirichletBC
    preset = true
    variable = disp_y
    value = 0
    boundary = 'Y_Minus'
  []

  [roller_at_Z_minus]
    type = DirichletBC
    preset = true
    variable = disp_z
    value = 0
    boundary = 'Z_Minus'
  []

  [roller_at_Z_plus]
    type = DirichletBC
    preset = true
    variable = disp_z
    value = 0
    boundary = 'Z_Plus'
  []

  # restrict the rigid body motion after fully debonding
  #[spring_fiber_x]
  #  type = PenaltyDirichletBC
  #  variable = disp_x
  #  value = 0
  #  penalty = 10
  #  boundary = 'fiber_Z_minus'
  #[]
  #[spring_fiber_y]
  #  type = PenaltyDirichletBC
  #  variable = disp_y
  #  value = 0
  #  penalty = 10
  #  boundary = 'fiber_Z_minus'
  #[]
 # [spring_fiber_z]
 #   type = PenaltyDirichletBC
 #   variable = disp_z
 #   value = 0
 #   penalty = 10
 #   boundary = 'fiber_Z_minus'
 # []
  #
  #Apply displacement at X_plus
  #
  [U_at_X_plus]
    type = FunctionDirichletBC
    variable = disp_x
    function = topfunc
    preset = true
    boundary = 'X_Plus'
  []
  [U_at_Y_plus]
    type = FunctionDirichletBC
    variable = disp_y
    function = topfunc
    preset = true
    boundary = 'Y_Plus'
  []
[]
[AuxVariables]
  [stress_xx]
    order = FIRST
    family = MONOMIAL
  []
  [stress_yy]
    order = FIRST
    family = MONOMIAL
  []
  [stress_zz]
    order = FIRST
    family = MONOMIAL
  []
  [stress_xy]
    order = FIRST
    family = MONOMIAL
  []
  [stress_xz]
    order = FIRST
    family = MONOMIAL
  []
  [stress_yz]
    order = FIRST
    family = MONOMIAL
  []
 # [hydrostatic_stress]
 #   order = FIRST
 #   family = MONOMIAL
 # []
 # [vonmises_stress]
 #   order = FIRST
 #   family = MONOMIAL
 # []
  [RF_X]
    order = FIRST
  []

  [RF_Y]
    order = FIRST
  []
[]

[AuxKernels]
  [stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  []
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  []
  [stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  []
  [stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
  []
  [stress_xz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xz
    index_i = 0
    index_j = 2
  []
  [stress_yz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yz
    index_i = 1
    index_j = 2
  []
  #[hydrostatic_stress]
  #  type = RankTwoAux
  #  rank_two_tensor = stress
  #  variable = hydrostatic_stress
  #[]
  #[vonmises_stress]
  #  type = RankTwoAux
  #  rank_two_tensor = stress
  #  variable = vonmises_stress
  #[]
  [RF_X]
    type = TagVectorAux
    variable = RF_X
    v = disp_x
    vector_tag = react
  []
  [RF_Y]
    type = TagVectorAux
    variable = RF_Y
    v = disp_y
    vector_tag = react
  []
[]

[Postprocessors]
  [Reaction_Force_Along_X]
    type = NodalSum
    boundary = X_Plus
    variable = RF_X
  []
  [Reaction_Force_Along_Y]
    type = NodalSum
    boundary = Y_Plus
    variable = RF_Y
  []
[]

[Problem]
  extra_tag_vectors = react
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  [Quadrature]
   side_order = 'constant'
  []
  dt = 0.01
  dtmax = 0.1
  dtmin = 0.000001
  end_time = 2

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
#  petsc_options_value = 'lu'
   petsc_options_value = 'lu     superlu_dist'
  solve_type = PJFNK

  line_search = none

  #   end_time = 1.0
  #   dt = 0.1
   l_max_its  = 1
 # l_tol = 1e-6
  nl_max_its = 15

 # nl_abs_tol = 1e-8
 # nl_rel_tol = 1e-8

   nl_abs_tol = 1e-8
[]

[Outputs]
  file_base = large_p_fiber_64_biaxis_out_0
  exodus = true
  [csv]
   type = CSV
  []
#  checkpoint = true
[]

[Debug]
  show_material_props = true
[]
