#ifndef __STAN__AGRAD__REV__MATRIX__LOG_DETERMINANT_HPP__
#define __STAN__AGRAD__REV__MATRIX__LOG_DETERMINANT_HPP__

#include <vector>
#include <stan/math/matrix/Eigen.hpp>
#include <stan/math/matrix/typedefs.hpp>
#include <stan/math/matrix/validate_multiplicable.hpp>
#include <stan/math/matrix/validate_square.hpp>
#include <stan/agrad/rev/var.hpp>
#include <stan/agrad/rev/matrix/typedefs.hpp>

namespace stan {
  namespace agrad {

    namespace {
      template<int R,int C>
      class log_determinant_vari : public vari {
        int _rows;
        int _cols;
        double* _A;
        vari** _adjARef;
      public:
        log_determinant_vari(const Eigen::Matrix<var,R,C> &A)
          : vari(log_determinant_vari_calc(A)), 
            _rows(A.rows()),
            _cols(A.cols()),
            _A((double*)stan::agrad::memalloc_.alloc(sizeof(double) 
                                                     * A.rows() * A.cols())),
            _adjARef((vari**)stan::agrad::memalloc_.alloc(sizeof(vari*) 
                                                          * A.rows() * A.cols()))
        {
          size_t pos = 0;
          for (size_type j = 0; j < _cols; j++) {
            for (size_type i = 0; i < _rows; i++) {
              _A[pos] = A(i,j).val();
              _adjARef[pos++] = A(i,j).vi_;
            }
          }
        }
        static 
        double log_determinant_vari_calc(const Eigen::Matrix<var,R,C> &A)
        {
          Eigen::Matrix<double,R,C> Ad(A.rows(),A.cols());
          for (size_type j = 0; j < A.cols(); j++)
            for (size_type i = 0; i < A.rows(); i++)
              Ad(i,j) = A(i,j).val();
          return Ad.fullPivHouseholderQr().logAbsDeterminant();
        }
        virtual void chain() {
          using Eigen::Matrix;
          using Eigen::Map;
          Matrix<double,R,C> adjA(_rows,_cols);
          adjA = adj_ 
            * Map<Matrix<double,R,C> >(_A,_rows,_cols)
            .inverse().transpose();
          size_t pos = 0;
          for (size_type j = 0; j < _cols; j++) {
            for (size_type i = 0; i < _rows; i++) {
              _adjARef[pos++]->adj_ += adjA(i,j);
            }
          }
        }
      };
    }

    template <int R, int C>
    inline var log_determinant(const Eigen::Matrix<var,R,C>& m) {
      stan::math::validate_square(m,"log_determinant");
      return var(new log_determinant_vari<R,C>(m));
    }
    
  }
}
#endif
