#include "material.hh"

e_mat_type Material_Phong::get_type() { return E_PHONG; }
e_mat_type Material_Phong_Tex::get_type() { return E_PHONG_TEX; }
e_mat_type Material_PBR::get_type() { return E_PBR; }
e_mat_type Material_Flat::get_type() { return E_FLAT; }
e_mat_type Material_Flat_Tex::get_type() { return E_FLAT_TEX; }
