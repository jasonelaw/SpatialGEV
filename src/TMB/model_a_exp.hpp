#ifndef model_a_exp_hpp
#define model_a_exp_hpp

#include "SpatialGEV/utils.hpp"

#undef TMB_OBJECTIVE_PTR
#define TMB_OBJECTIVE_PTR obj

template<class Type>
Type model_a_exp(objective_function<Type>* obj){
  /*
  Model layer 1: y ~ GEV(a, b, s)
  Model layer 2: a ~ GP(0, Sigma_a(sigma_a, ell_a))
  */ 
  using namespace density;
  using namespace SpatialGEV;
  
  // data inputs
  DATA_VECTOR(y); // response vector: mws. Assumed to be > 0
  DATA_IVECTOR(n_obs); // number of observations per location
  DATA_MATRIX(dd); // distance matrix
  DATA_SCALAR(sp_thres); // a number used to make the covariance matrix sparse by thresholding. If sp_thres=0, no thresholding is made.
  DATA_INTEGER(reparam_s); // a flag indicating whether the shape parameter is zero: 0, constrained to positive: 1 , constrained to be negative: 2, or unconstrained: 3 
  DATA_SCALAR(s_mean); // The mean of the normal prior on s or log(|s|), depending on what reparametrization is used for s. 
  DATA_SCALAR(s_sd); // The standard deviation of the normal prior on s or log(|s|). If s_sd>9999, a flat prior is imposed.
  // parameter list
  PARAMETER_VECTOR(a); // random effect to be integrated out. 
  PARAMETER(log_b); // log-transformed scale parameters of the GEV model  
  PARAMETER(s); // initial shape parameter of the GEV model. IMPORTANT: If reparam_s = "negative" or "postive", the initial input should be log(|s|)
  PARAMETER(log_sigma_a); // hyperparameter: log-transformed squared amplitude parameter (scalar) of the exponential covariance function in Sigma_a
  PARAMETER(log_ell_a); // hyperparameter: log-transformed smoothness parameter (scalar) of the exponential covariance function in Sigma_a

  int n = n_obs.size(); // number of locations
  Type sigma_a = exp(log_sigma_a);
  Type ell_a = exp(log_ell_a);
  
  // construct the covariance matrix
  matrix<Type> cova(n,n);
  cov_expo<Type>(cova, dd, sigma_a, ell_a, sp_thres);
  
  // calculate the negative log likelihood
  Type nll = Type(0.0); 
  nll_accumulator_a<Type>(nll, y, n_obs, a, log_b, s, n, reparam_s, s_mean, s_sd);  
  nll += MVNORM(cova)(a);
  
  return nll;  
}

#undef TMB_OBJECTIVE_PTR
#define TMB_OBJECTIVE_PTR this

#endif