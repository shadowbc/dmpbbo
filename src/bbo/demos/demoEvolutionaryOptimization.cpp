/**
 * \file demoEvolutionaryOptimization.cpp
 * \author Freek Stulp
 * \brief  Demonstrates how to run an evolution strategy to optimize a distance function, implemented as a CostFunction.
 *
 * \ingroup Demos
 * \ingroup BBO
 *
 * This file is part of DmpBbo, a set of libraries and programs for the 
 * black-box optimization of dynamical movement primitives.
 * Copyright (C) 2014 Freek Stulp, ENSTA-ParisTech
 * 
 * DmpBbo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * DmpBbo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with DmpBbo.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iomanip> 
#include <eigen3/Eigen/Core>

#include "dmpbbo_io/EigenBoostSerialization.hpp"

#include "bbo/CostFunction.hpp"
#include "bbo/runEvolutionaryOptimization.hpp"
#include "bbo/DistributionGaussian.hpp"

#include "bbo/updaters/UpdaterMean.hpp"
#include "bbo/updaters/UpdaterCovarDecay.hpp"
#include "bbo/updaters/UpdaterCovarAdaptation.hpp"



using namespace std;
using namespace Eigen;
using namespace DmpBbo;

namespace DmpBbo 
{

/** CostFunction in which the distance to a pre-defined point must be minimized.
 *
 * \ingroup BBO
 * \ingroup Demos
 */
class DemoCostFunctionDistanceToPoint : public CostFunction
{
public:
  /** Constructor.
   * \param[in] point Point to which distance must be minimized.
   */
  DemoCostFunctionDistanceToPoint(const VectorXd& point)
  {
    point_ = point;
  }
  
  /** The cost function which defines the cost_function.
   *
   * \param[in] samples The samples 
   * \param[out] costs The scalar cost for each sample.
   */
  void evaluate(const MatrixXd& samples, VectorXd& costs) const {
    int n_dims    = samples.cols();
    int n_samples = samples.rows();

    assert(n_dims==point_.size());

    costs.resize(n_samples);
    for (int ss=0; ss<n_samples; ss++)
    {
      // Cost is distance to point
      costs[ss] = sqrt((samples.row(ss) - point_.transpose()).array().pow(2).sum());
    }
  }
  
  /** Returns a string representation of the object.
   * \return A string representation of the object.
   */
  string toString(void) const {
    string str = "CostFunctionDistanceToPoint";
    return str;
  }

private:
  /** Point to which distance is computed. */
  VectorXd point_;
  
};

}

/** Main function
 * \param[in] n_args Number of arguments
 * \param[in] args Arguments themselves
 * \return Success of exection. 0 if successful.
 */
int main(int n_args, char* args[])
{
  // If program has an argument, it is a directory to which to save files too (or --help)
  string directory;
  string covar_update = "decay";
  if (n_args>1)
  {
    if (string(args[1]).compare("--help")==0)
    {
      cout << "Usage: " << args[0] << " [directory] [covar_update]" << endl;
      cout << "                    directory: optional directory to save data to" << endl;
      cout << "                    covar_update: [none|decay|adaptation]" << endl;
      return 0;
    }
    else
    {
      directory = string(args[1]);
    }
    if (n_args>2)
    {
      covar_update = string(args[2]);
    }
  }
  
  int n_dims = 2;
  //VectorXd minimum = VectorXd::LinSpaced(n_dims,1,n_dims);
  VectorXd minimum = VectorXd::Zero(n_dims);
  
  CostFunction* cost_function = new DemoCostFunctionDistanceToPoint(minimum);
  
  VectorXd mean_init  =  5.0*VectorXd::Ones(n_dims);
  MatrixXd covar_init =  4.0*MatrixXd::Identity(n_dims,n_dims);
  DistributionGaussian* distribution = new DistributionGaussian(mean_init, covar_init); 
  
  Updater* updater = NULL;
  double eliteness = 10;
  string weighting_method = "PI-BB";
    
  if (covar_update.compare("none")==0)
  {
    updater = new UpdaterMean(eliteness,weighting_method);
  }
  else if (covar_update.compare("decay")==0)
  {
    double covar_decay_factor = 0.8;
    updater = new UpdaterCovarDecay(eliteness,covar_decay_factor,weighting_method);
  }
  else 
  {
    VectorXd base_level = VectorXd::Constant(n_dims,0.000001);
    eliteness = 10;
    bool diag_only = false;
    double learning_rate = 0.75;
    updater = new UpdaterCovarAdaptation(eliteness,weighting_method,base_level,diag_only,learning_rate);  
  }
    
  
  int n_samples_per_update = 10;
  
  int n_updates = 40;
  
  runEvolutionaryOptimization(cost_function, distribution, updater, n_updates, n_samples_per_update,directory);
  
}


