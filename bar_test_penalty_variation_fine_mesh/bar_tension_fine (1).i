[Mesh]
  [bar_model_mesh]
    type = FileMeshGenerator
    file = bar_fine_mesh.e
  []
 # [breakmesh_at_central]
 #   type = BreakMeshByBlockGenerator
 #   input = bar_model_mesh
 #   block_pair = '2 3'
 #   interface_name = CZM_Interface
 # []
  [explode_end_block]
    type = ExplodeMeshGenerator
    input = bar_model_mesh
    subdomains = '1 2 3 4'
    interface_name = DG_Interface
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
 # [ifk_x]
 #   type = Rczm
 #   component = 0
 #   variable = 'disp_x'
 #   neighbor_var = 'disp_x'
 #   displacements = 'disp_x disp_y disp_z'
 #   boundary = 'CZM_Interface'
 #   stabilized_para =1000
 # []
 # [ifk_y]
 # type = Rczm
 # component = 1
 # variable = 'disp_y'
 # neighbor_var = 'disp_y'
 # displacements = 'disp_x disp_y disp_z'
 # boundary = 'CZM_Interface'
 # stabilized_para = 1000
 # []
 # [ifk_z]
 # type = Rczm
 # component = 2
 # variable = 'disp_z'
 # neighbor_var = 'disp_z'
 # displacements = 'disp_x disp_y disp_z'
 # boundary = 'CZM_Interface'
 # stabilized_para = 1000
 # []
  [ifk_x_DG_Solid]
    type = DGSolid
    component = 0
    variable = 'disp_x'
    neighbor_var = 'disp_x'
    displacements = 'disp_x disp_y disp_z'
    boundary = 'DG_Interface'
    stabilized_para =1000
  []
  [ifk_y_DG_Solid]
  type = DGSolid
  component = 1
  variable = 'disp_y'
  neighbor_var = 'disp_y'
  displacements = 'disp_x disp_y disp_z'
  boundary = 'DG_Interface'
  stabilized_para = 1000
  []
  [ifk_z_DG_Solid]
  type = DGSolid
  component = 2
  variable = 'disp_z'
  neighbor_var = 'disp_z'
  displacements = 'disp_x disp_y disp_z'
  boundary = 'DG_Interface'
  stabilized_para = 1000
  []
[]

[Materials]
  [Elasticity_Matrix]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1.0e+10
    poissons_ratio = 0.0
    block = '1 2 3 4'
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2 3 4'
  []
 # [./Rigid_czm_model]
 #   type = OrtizPandolfiRczm
 #   boundary = 'CZM_Interface'
 #   maximum_effective_traction = 100.0e+6
 #   maximum_effective_gap = 0.1
 #   shear_weight = 0.707
 #   displacements = 'disp_x disp_y disp_z'
 #   outputs = all
 # [../]

  #[./czm_3dc]
  #  type = SalehaniIrani3DCTraction
  #  boundary = 'matrix_fiber'
  #  normal_gap_at_maximum_normal_traction = 0.0001
  #  tangential_gap_at_maximum_shear_traction = 1.0
  #  maximum_normal_traction = 100
  #  maximum_shear_traction = 0.0001
  #  displacements = 'disp_x disp_y disp_z'
  #[../]
[]

[Functions]
  [left_func]
    type = ParsedFunction
    value = '-t'
    #value = '0.1*sin(3.1415926/2.0*t)'
  []
  [right_func]
    type = ParsedFunction
    value = 't'
    #value = '0.1*sin(3.1415926/2.0*t)'
  []
[]

[BCs]
  # Prescribed displacements at three boundary surfaces
  # Enforce plain strain condition by constrainting
  # zero-displacement on z_plus and z-minus

  #[On_X_Minus]
  #  type = DirichletBC
  #  preset = true
  #  variable = disp_x
  #  value = 0
  #  boundary = 'X_Minus'
  #[]

  [On_X_Minus]
    type = FunctionDirichletBC
    variable = disp_x
    function = left_func
    preset = true
    boundary = 'X_Minus'
  []

  [On_Y_Minus]
    type = DirichletBC
    preset = true
    variable = disp_y
    value = 0
    boundary = 'Y_Minus'
  []

  [On_Z_Minus]
    type = DirichletBC
    preset = true
    variable = disp_z
    value = 0
    boundary = 'Z_Minus'
  []

  [On_X_Plus]
    type = FunctionDirichletBC
    variable = disp_x
    function = right_func
    preset = true
    boundary = 'X_Plus'
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
[]

[Postprocessors]
  [Reaction_Force_Along_X]
    type = NodalSum
    boundary = X_Plus
    variable = RF_X
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
  dt = 0.0002
  dtmax = 0.1
  dtmin = 0.000001
  end_time = 0.1

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
#  petsc_options_value = 'lu'
   petsc_options_value = 'lu     superlu_dist'
  solve_type = PJFNK

  line_search = none

  #   end_time = 1.0
  #   dt = 0.1
   l_max_its  = 30
  l_tol = 1e-8
  nl_max_its = 15

 # nl_abs_tol = 1e-6
 # nl_rel_tol = 1e-8

   nl_abs_tol = 1e-8
[]

[Outputs]
  file_base = out_0
  exodus = true
  [csv]
   type = CSV
  []
#  checkpoint = true
[]

[Debug]
  show_material_props = true
[]
