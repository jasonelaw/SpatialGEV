#ifndef model_abs_exp_hpp
#define model_abs_exp_hpp

#include "SpatialGEV/utils.hpp"

#undef TMB_OBJECTIVE_PTR
#define TMB_OBJECTIVE_PTR obj

template<class Type>
Type model_abs_exp(objective_function<Type>* obj){
  /*
  Model layer 1: y ~ GEV(a, b, s)
  Model layer 2: 
  a ~ GP(0, Sigma_a(sigma_a, ell_a))
  logb ~ GP(0, Sigma_b(sigma_b, ell_b))
  g(s) ~ GP(0, Sigma_s(sigma_s, ell_s)) where s is a transformation function of s
  */ 
  using namespace density;
  using namespace SpatialGEV;
  
  // data inputs
  DATA_VECTOR(y); // response vector: mws. Assumed to be > 0
  DATA_IVECTOR(n_obs); // number of observations per location
  DATA_MATRIX(design_mat_a); // n x r design matrix for a
  DATA_MATRIX(design_mat_b); // n x r design matrix for logb
  DATA_MATRIX(design_mat_s); // n x r design matrix for s
  DATA_MATRIX(dd); // distance matrix
  DATA_SCALAR(sp_thres); // a number used to make the covariance matrix sparse by thresholding. If sp_thres=-1, no thresholding is made.
  DATA_INTEGER(reparam_s); // a flag indicating whether the shape parameter is "zero", "unconstrained", constrained to be "negative", or constrained to be "positve"
  // parameter list
  PARAMETER_VECTOR(a); // random effect to be integrated out. 
  PARAMETER_VECTOR(log_b); // random effect to be integrated out: log-transformed scale parameters of the GEV model  
  PARAMETER_VECTOR(s); // if reparam_s = "negative" or "postive", the initial input should be log(|s|)
  PARAMETER_VECTOR(beta_a); // r x 1 mean vector coefficients for a
  PARAMETER_VECTOR(beta_b); // r x 1 mean vector coefficients for logb
  PARAMETER_VECTOR(beta_s); // r x 1 mean vector coefficients for s
  PARAMETER(log_sigma_a); // hyperparameter: log-transformed squared amplitude parameter (scalar) of the exponential covariance function in Sigma_a
  PARAMETER(log_ell_a); // hyperparameter: log-transformed smoothness parameter (scalar) of the exponential covariance function in Sigma_a
  PARAMETER(log_sigma_b); // hyperparameter: log-transformed squared amplitude parameter (scalar) of the exponential covariance function in Sigma_b
  PARAMETER(log_ell_b); // hyperparameter: log-transformed smoothness parameter (scalar) of the exponential covariance function in Sigma_b
  PARAMETER(log_sigma_s); // hyperparameter for s
  PARAMETER(log_ell_s); // as above

  int n = n_obs.size();
  Type sigma_a = exp(log_sigma_a);
  Type ell_a = exp(log_ell_a);
  Type sigma_b = exp(log_sigma_b);
  Type ell_b = exp(log_ell_b);
  Type sigma_s = exp(log_sigma_s); 
  Type ell_s = exp(log_ell_s); 
  
  // construct the covariance matrices
  matrix<Type> cova(n,n);
  matrix<Type> covb(n,n);
  matrix<Type> covs(n,n);
  cov_expo<Type>(cova, dd, sigma_a, ell_a, sp_thres);
  cov_expo<Type>(covb, dd, sigma_b, ell_b, sp_thres);
  cov_expo<Type>(covs, dd, sigma_s, ell_s, sp_thres);
  
  // calculate the negative log likelihood
  Type nll = Type(0.0); 
  vector<Type> mu_a = a - design_mat_a * beta_a;
  vector<Type> mu_b = log_b - design_mat_b * beta_b;
  vector<Type> mu_s = s - design_mat_s * beta_s;
  nll += MVNORM(cova)(mu_a);
  nll += MVNORM(covb)(mu_b);
  nll += MVNORM(covs)(mu_s);
  nll_accumulator_abs<Type>(nll, y, n_obs, a, log_b, s, n, reparam_s);
  
  return nll;  
}

#undef TMB_OBJECTIVE_PTR
#define TMB_OBJECTIVE_PTR this

#endif
