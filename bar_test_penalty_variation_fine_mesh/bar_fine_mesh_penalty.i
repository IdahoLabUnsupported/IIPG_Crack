[Mesh]
  [bar_model_mesh]
    type = FileMeshGenerator
    file = bar_fine_mesh.e
  []
  [explode_end_block]
    type = ExplodeMeshGenerator
    input = bar_model_mesh
    subdomains = '1 2 3 4'
    interface_name = DG_Interface
  []
  use_displaced_mesh = false
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = SMALL
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_xz stress_yz'
  []
[]
[InterfaceKernels]
  [ifk_x]
    type = DGSolid
    variable = 'disp_x'
    neighbor_var = 'disp_x'
    displacements = 'disp_x disp_y disp_z'
    boundary = 'DG_Interface'
    stabilized_para =1000
  []
  [ifk_y]
  type = DGSolid
  variable = 'disp_y'
  neighbor_var = 'disp_y'
    displacements = 'disp_x disp_y disp_z'
    boundary = 'DG_Interface'
  stabilized_para = 1000
  []
  [ifk_z]
  type = DGSolid
  variable = 'disp_z'
  neighbor_var = 'disp_z'
    displacements = 'disp_x disp_y disp_z'
    boundary = 'DG_Interface'
  stabilized_para = 1000
  []
[]

[Materials]
  [./Elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1.0e+10
    poissons_ratio = 0.0
    block = '1 2 3 4'
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = '1 2 3 4'
  [../]
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
    type = DirichletBC
    variable = disp_x
    preset = true
    boundary = 'X_Minus'
    value = -0.02
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
    type = DirichletBC
    variable = disp_x
    preset = true
    boundary = 'X_Plus'
    value = 0.02
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
[]

[Postprocessors]
  [stress_xx]
    type = PointValue
    outputs = csv
    point = '0.5 0.5 0.5'
    variable = stress_xx
  []
[]

[Preconditioning]

  [./ilu_may_use_less_mem]
    type = SMP
    full = true
    petsc_options = '-ksp_diagonal_scale -ksp_diagonal_scale_fix'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type'
    petsc_options_value = 'gmres asm ilu NONZERO'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = Newton
  nl_abs_tol = 1e-15
  nl_rel_tol = 1e-15
  nl_max_its = 15
  l_tol = 1e-7
  l_max_its = 50
  line_search = none
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = bar_penalty
  exodus = true
  csv = true
[]
