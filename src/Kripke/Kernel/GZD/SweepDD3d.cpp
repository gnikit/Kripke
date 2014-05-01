#include "../Param.h"


/* Sweep routine for Diamond-Difference */
/* Macros for offsets with fluxes on cell faces */
#define Left_INDEX(i, j, k) (i) + (local_imax_1)*(j) \
  + (local_imax_1)*(local_jmax)*(k)
#define Front_INDEX(i, j, k) (i) + (local_imax)*(j) \
  + (local_imax)*(local_jmax_1)*(k)
#define Bottom_INDEX(i, j, k) (i) + (local_imax)*(j) \
  + (local_imax)*(local_jmax)*(k)
#define I_PLANE_INDEX(j, k) (k)*(local_jmax) + (j)
#define J_PLANE_INDEX(i, k) (k)*(local_imax) + (i)
#define K_PLANE_INDEX(i, j) (j)*(local_imax) + (i)

#define Ghost_INDEX(i, j, k) (i) + (local_imax_2)*(j) \
  + (local_imax_2)*(local_jmax_2)*(k)
#define Zonal_INDEX(i, j, k) (i) + (local_imax)*(j) \
  + (local_imax)*(local_jmax)*(k)

void gzd_nmd_SweepDD_3d(gzd_nmd_Param &p) {
  int num_directions = p.num_directions;
  int num_zones = p.num_zones;
  int num_groups = p.num_groups;
  Direction *direction = p.direction;

  int local_imax = p.nzones[0];
  int local_jmax = p.nzones[1];
  int local_kmax = p.nzones[2];
  int local_imax_1 = local_imax + 1;
  int local_jmax_1 = local_jmax + 1;

  double * __restrict__ dx = p.deltas[0];
  double * __restrict__ dy = p.deltas[1];
  double * __restrict__ dz = p.deltas[2];

  gzd_TVec psi_lf(num_groups, num_directions,
                  (local_imax+1)*local_jmax*local_kmax);
  gzd_TVec psi_fr(num_groups, num_directions,
                  local_imax*(local_jmax+1)*local_kmax);
  gzd_TVec psi_bo(num_groups, num_directions,
                  local_imax*local_jmax*(local_kmax+1));

  gzd_TVec i_plane_v(num_groups, num_directions, local_jmax*local_kmax);
  gzd_TVec j_plane_v(num_groups, num_directions, local_imax*local_kmax);
  gzd_TVec k_plane_v(num_groups, num_directions, local_imax*local_jmax);
  double ***i_plane = i_plane_v.data;
  double ***j_plane = j_plane_v.data;
  double ***k_plane = k_plane_v.data;

  gzd_TVec psi_internal(num_groups, num_directions, num_zones);
  double ***psi_internal_all = psi_internal.data;

  double ***psi = p.psi.data;
  double ***rhs = p.rhs.data;
  double **sigt = p.sigt.data;

  // All directions have same id,jd,kd, since we are modeling an "Angle Set"
  // So pull that information out now
  int istartz, istopz, in, il, ir;
  int id = direction[0].id;
  int jd = direction[0].jd;
  int kd = direction[0].kd;
  if(id > 0){
    istartz = 0; istopz = local_imax-1; in = 1; il = 0; ir = 1;
  }
  else {
    istartz = local_imax-1; istopz = 0; in = -1; il = 1; ir = 0;
  }

  int jstartz, jstopz, jn, jf, jb;
  if(jd > 0){
    jstartz = 0; jstopz = local_jmax-1; jn = 1; jf = 0; jb = 1;
  }
  else {
    jstartz = local_jmax-1; jstopz = 0; jn = -1; jf = 1; jb = 0;
  }

  int kstartz, kstopz, kn, kb, kt;
  if(kd > 0){
    kstartz = 0; kstopz = local_kmax-1; kn =  1; kb = 0; kt = 1;
  }
  else {
    kstartz = local_kmax-1; kstopz = 0; kn = -1; kb = 1; kt = 0;
  }

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for(int group = 0; group < num_groups; ++group){
    double **psi_g = psi[group];
    double **rhs_g = rhs[group];
    double **psi_lf_g = psi_lf.data[group];
    double **psi_fr_g = psi_fr.data[group];
    double **psi_bo_g = psi_bo.data[group];
    double **psi_internal_all_g = psi_internal_all[group];
    double **i_plane_g = i_plane[group];
    double **j_plane_g = j_plane[group];
    double **k_plane_g = k_plane[group];
    double * __restrict__ sigt_g = sigt[group];


    /* Copy the angular fluxes incident upon this subdomain */
    for(int k=0; k<local_kmax; k++){
      for(int j=0; j<local_jmax; j++){
        /* psi_lf has length (local_imax+1)*local_jmax*local_kmax */
        double * __restrict__ psi_lf_g_z =
          psi_lf_g[Left_INDEX(istartz+il, j, k)];
        double * __restrict__ i_plane_g_z = i_plane_g[I_PLANE_INDEX(j, k)];
        for(int d = 0; d < num_directions; ++d){
          psi_lf_g_z[d] = i_plane_g_z[d];
        }
      }
    }

    for(int k=0; k<local_kmax; k++){
      for(int i=0; i<local_imax; i++){
        /* psi_fr has length local_imax*(local_jmax+1)*local_kmax */
        double * __restrict__ psi_fr_g_z =
          psi_fr_g[Front_INDEX(i, jstartz+jf, k)];
        double * __restrict__ j_plane_g_z = j_plane_g[J_PLANE_INDEX(i, k)];
        for(int d = 0; d < num_directions; ++d){
          psi_fr_g_z[d] = j_plane_g_z[d];
        }
      }
    }

    for(int j=0; j<local_jmax; j++){
      for(int i=0; i<local_imax; i++){
        double * __restrict__ psi_bo_g_z =
          psi_bo_g[Bottom_INDEX(i, j, kstartz+ kb)];
        double * __restrict__ k_plane_g_z = k_plane_g[K_PLANE_INDEX(i, j)];
        for(int d = 0; d < num_directions; ++d){
          psi_bo_g_z[d] = k_plane_g_z[d];
        }
      }
    }

    /*  Perform transport sweep of the grid 1 cell at a time.   */
    for(int k=kstartz; std::abs(k-kstartz)<local_kmax; k+=kn){
      double dzk = dz[k+1];
      for(int j=jstartz; std::abs(j-jstartz)<local_jmax; j+=jn){
        double dyj = dy[j+1];
        for(int i=istartz; std::abs(i-istartz)<local_imax; i+=in){
          double dxi = dx[i+1];

          int z = Zonal_INDEX(i, j, k);
          double * __restrict__ psi_g_z = psi_g[z];
          double * __restrict__ rhs_g_z = rhs_g[z];

          double * __restrict__ psi_lf_g_zil =
            psi_lf_g[Left_INDEX(i+il, j, k)];
          double * __restrict__ psi_lf_g_zir =
            psi_lf_g[Left_INDEX(i+ir, j, k)];

          double * __restrict__ psi_fr_g_zjf =
            psi_fr_g[Front_INDEX(i, j+jf, k)];
          double * __restrict__ psi_fr_g_zjb =
            psi_fr_g[Front_INDEX(i, j+jb, k)];

          double * __restrict__ psi_bo_g_zkb =
            psi_bo_g[Bottom_INDEX(i, j, k+kb)];
          double * __restrict__ psi_bo_g_zkt =
            psi_bo_g[Bottom_INDEX(i, j, k+kt)];


          double * __restrict__ psi_internal_all_g_z = psi_internal_all_g[z];
          double * __restrict__ i_plane_g_z = i_plane_g[I_PLANE_INDEX(j, k)];
          double * __restrict__ j_plane_g_z = j_plane_g[J_PLANE_INDEX(i, k)];
          double * __restrict__ k_plane_g_z = k_plane_g[K_PLANE_INDEX(i, j)];


          for(int d = 0; d < num_directions; ++d){
            double xcos = direction[d].xcos;
            double ycos = direction[d].ycos;
            double zcos = direction[d].zcos;

            double zcos_dzk = 2.0*zcos/dzk;
            double ycos_dyj = 2.0*ycos/dyj;
            double xcos_dxi = 2.0*xcos/dxi;

            double *psi_int_lf = psi_internal_all_g_z;
            double *psi_int_fr = psi_internal_all_g_z;
            double *psi_int_bo = psi_internal_all_g_z;

            /* Add internal surface source data */
            psi_lf_g_zil[d] += psi_int_lf[d];
            psi_fr_g_zjf[d] += psi_int_fr[d];
            psi_bo_g_zkb[d] += psi_int_bo[d];

            /* Calculate new zonal flux */
            double psi_g_z_d =
              (rhs_g_z[d]
               + psi_lf_g_zil[d]*xcos_dxi
               + psi_fr_g_zjf[d]*ycos_dyj
               + psi_bo_g_zkb[d]*zcos_dzk)/
              (xcos_dxi + ycos_dyj + zcos_dzk + sigt_g[Zonal_INDEX(i, j, k)]);

            psi_g_z[d] = psi_g_z_d;

            /* Apply diamond-difference relationships */
            psi_lf_g_zir[d] = 2.0*psi_g_z_d - psi_lf_g_zil[d];
            psi_fr_g_zjb[d] = 2.0*psi_g_z_d - psi_fr_g_zjf[d];
            psi_bo_g_zkt[d] = 2.0*psi_g_z_d - psi_bo_g_zkb[d];
          }
        }
      }
    }


    /* Copy the angular fluxes exiting this subdomain */
    for(int k=0; k<local_kmax; k++){
      for(int j=0; j<local_jmax; j++){
        double * __restrict__ psi_lf_g_z =
          psi_lf_g[Left_INDEX(istopz+ir, j, k)];
        double * __restrict__ i_plane_g_z = i_plane_g[I_PLANE_INDEX(j, k)];
        for(int d = 0; d < num_directions; ++d){
          psi_lf_g_z[d] = i_plane_g_z[d];
        }
      }
    }

    for(int k=0; k<local_kmax; k++){
      for(int i=0; i<local_imax; i++){
        double * __restrict__ psi_fr_g_z =
          psi_fr_g[Front_INDEX(i, jstopz+jb, k)];
        double * __restrict__ j_plane_g_z = j_plane_g[J_PLANE_INDEX(i, k)];
        for(int d = 0; d < num_directions; ++d){
          psi_fr_g_z[d] = j_plane_g_z[d];
        }
      }
    }

    for(int j=0; j<local_jmax; j++){
      for(int i=0; i<local_imax; i++){
        double * __restrict__ psi_bo_g_z =
          psi_bo_g[Bottom_INDEX(i, j, kstopz+kt)];
        double * __restrict__ k_plane_g_z = k_plane_g[K_PLANE_INDEX(i, j)];
        for(int d = 0; d < num_directions; ++d){
          k_plane_g_z[d] = psi_bo_g_z[d];
        }
      }
    }


  } // group

}
