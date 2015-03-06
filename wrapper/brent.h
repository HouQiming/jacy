typedef double(*brent_function)(double);
typedef double(*spap_function)( void* p, double x );
double glomin ( double a, double b, double c, double m, double machep, 
  double e, double t, brent_function f, double *x );
double local_min ( double a, double b, double eps, double t, 
  brent_function f, double *x );
double local_min_rc ( double *a, double *b, int *status, double value );
double r8_abs ( double x );
double r8_epsilon ( void );
double r8_max ( double x, double y );
double r8_sign ( double x );
double brent_zero ( double a, double b, double machep, double t, 
  spap_function, void* param );
void zero_rc ( double a, double b, double t, double *arg, int *status, 
  double value );
