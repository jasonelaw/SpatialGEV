#ifndef model_ab_matern_hpp
#define model_ab_matern_hpp

#include "SpatialGEV/utils.hpp"

#undef TMB_OBJECTIVE_PTR
#define TMB_OBJECTIVE_PTR obj

template<class Type>
Type model_ab_matern(objective_function<Type>* obj){
  /*
  Model layer 1: y ~ GEV(a, b, s)
  Model layer 2: a ~ GP(0, Sigma_a(phi_a, kappa_a))
                 logb ~ GP(0, Sigma_b(phi_b, kappa_b))
  */ 
  using namespace density;
  using namespace SpatialGEV;
  
  // data inputs
  DATA_VECTOR(y); // response vector: mws. Assumed to be > 0
  DATA_IVECTOR(n_obs); // number of observations per location
  DATA_MATRIX(dd); // distance matrix
  DATA_SCALAR(sp_thres); // a number used to make the covariance matrix sparse by thresholding. If sp_thres=-1, no thresholding is made.
  DATA_INTEGER(reparam_s); // a flag indicating whether the shape parameter is zero: 0, constrained to positive: 1 , constrained to be negative: 2, or unconstrained: 3 
  DATA_SCALAR(s_mean); // The mean of the normal prior on s or log(|s|), depending on what reparametrization is used for s. 
  DATA_SCALAR(s_sd); // The standard deviation of the normal prior on s or log(|s|). If s_sd>9999, a flat prior is imposed.
  // parameter list
  PARAMETER_VECTOR(a); // random effect to be integrated out. 
  PARAMETER_VECTOR(log_b); // random effect to be integrated out: log-transformed scale parameters of the GEV model  
  PARAMETER(s); // initial shape parameter of the GEV model. IMPORTANT: If reparam_s = "negative" or "postive", the initial input should be log(|s|)
  PARAMETER(log_phi_a); // hyperparameter: Matern range parameter for a
  PARAMETER(log_kappa_a); // hyperparameter: Matern smoothness parameter for a
  PARAMETER(log_phi_b); // hyperparameter: Matern range parameter for b
  PARAMETER(log_kappa_b); // hyperparameter: Matern smoothness parameter for b

  int n = n_obs.size();
  Type phi_a = exp(log_phi_a);
  Type kappa_a = exp(log_kappa_a);
  Type phi_b = exp(log_phi_b);
  Type kappa_b = exp(log_kappa_b);
  
  // construct the covariance matrix
  matrix<Type> cova(n,n);
  matrix<Type> covb(n,n);
  cov_matern<Type>(cova, dd, phi_a, kappa_a, sp_thres);
  cov_matern<Type>(covb, dd, phi_b, kappa_b, sp_thres);
  
  // calculate the negative log likelihood
  Type nll = Type(0.0); 
  nll_accumulator_ab<Type>(nll, y, n_obs, a, log_b, s, n, reparam_s, s_mean, s_sd);  
  nll += MVNORM(cova)(a);
  nll += MVNORM(covb)(log_b);
  
  return nll;  
}

#undef TMB_OBJECTIVE_PTR
#define TMB_OBJECTIVE_PTR this

#endif
