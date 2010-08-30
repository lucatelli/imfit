/* FILE: print_results.cpp ----------------------------------------- */

#include <stdio.h>
#include <string>

using namespace std;

#include "print_results.h"
#include "mpfit_cpp.h"
#include "param_struct.h"
#include "statistics.h"
#include "utilities_pub.h"

#define  FILE_OPEN_ERR_STRING "\n   Couldn't open file \"%s\"\n\n"



/* Local Functions: */
void PrintParam( FILE *outFile, string& paramName, double paramValue, double paramErr );



// The following is used by PrintResults()
void PrintParam( FILE *outFile, string& paramName, double paramValue, double paramErr )
{
  if (paramErr == 0.0)
    fprintf(outFile, "  %10s = %f\n", paramName.c_str(), paramValue);
  else
    fprintf(outFile, "  %10s = %f +/- %f\n", paramName.c_str(), paramValue, paramErr);
}


// This is a function to print the results of a fit.  It's based on code from
// Craig Markwardt's testmpfit.c, but will also accomodate results from a fit
// done with differential evolution (call with result=0 to indicate the latter).
void PrintResults( double *params, double *xact, mp_result *result, ModelObject *model,
									int nFreeParameters, mp_par *parameterInfo, int fitStatus )
{
  int  i;
  int  nValidPixels = model->GetNValidPixels();
  int  nDegreesFreedom = nValidPixels - nFreeParameters;
  string  mpfitMessage;
  double  aic, bic;
  
  if (result == 0) {
    // PrintResult was called with result from Differential Evolution fit, not mpfit
    // Only print results of fit if fitStatus >= 1
    if (fitStatus < 1)
      return;
    double  chiSquared = model->ChiSquared(params);
    printf("  CHI-SQUARE = %lf    (%d DOF)\n", chiSquared, nDegreesFreedom);
    printf("\n");
    aic = AIC_corrected(chiSquared, nFreeParameters, nValidPixels, 1);
    bic = BIC(chiSquared, nFreeParameters, nValidPixels, 1);
    printf("Reduced Chi^2 = %f\n", chiSquared / nDegreesFreedom);
    printf("AIC = %f, BIC = %f\n\n", aic, bic);
    // output the best-fit parameters
    for (i = 0; i < model->GetNParams(); i++) {
      PrintParam(stdout, model->GetParameterName(i), params[i] + parameterInfo[i].offset, 0.0);
    }
    return;
  }
  
  // OK, if we got this far, then we're dealing with mpfit output
  InterpretMpfitResult(fitStatus, mpfitMessage);
  printf("*** mpfit status = %d -- %s\n", fitStatus, mpfitMessage.c_str());
    // Only print results of fit if valid fit was achieved
  if ((params == 0) || (result == 0))
    return;
  printf("  CHI-SQUARE = %f    (%d DOF)\n", result->bestnorm, nDegreesFreedom);
  printf("  INITIAL CHI^2 = %f\n", result->orignorm);
  printf("        NPAR = %d\n", result->npar);
  printf("       NFREE = %d\n", result->nfree);
  printf("     NPEGGED = %d\n", result->npegged);
  printf("     NITER = %d\n", result->niter);
  printf("      NFEV = %d\n", result->nfev);
  printf("\n");
  aic = AIC_corrected(result->bestnorm, nFreeParameters, nValidPixels, 1);
  bic = BIC(result->bestnorm, nFreeParameters, nValidPixels, 1);
  printf("Reduced Chi^2 = %f\n", result->bestnorm / nDegreesFreedom);
  printf("AIC = %f, BIC = %f\n\n", aic, bic);
  
  if (xact) {
    for (i = 0; i < result->npar; i++) {
      printf("  P[%d] = %f +/- %f     (ACTUAL %f)\n", 
	     i, params[i], result->xerror[i], xact[i]);
    }
  } else {
    for (i = 0; i < result->npar; i++) {
      PrintParam(stdout, model->GetParameterName(i), params[i] + parameterInfo[i].offset, result->xerror[i]);
    }
  }    
}



void SaveParameters( double *params, ModelObject *model, mp_par *parameterInfo, 
                    string& outputFilename, int argc, char *argv[] )
{
  FILE  *file_ptr;
  
  if ((file_ptr = fopen(outputFilename.c_str(), "w")) == NULL) {
    fprintf(stderr, FILE_OPEN_ERR_STRING, outputFilename.c_str());
    exit(-1);
  }

  char  *timeStamp;
  timeStamp = TimeStamp();
  fprintf(file_ptr, "# Best-fit model results for imfit\n");
  fprintf(file_ptr, "# Generated on %s by the following command:\n#   ", 
          timeStamp);
  for (int i = 0; i < argc; i++)
    fprintf(file_ptr, " %s", argv[i]);
  fprintf(file_ptr, "\n\n");

  model->PrintModelParams(params, file_ptr);

  fclose(file_ptr);

}


/* END OF FILE: print_results.cpp ---------------------------------- */