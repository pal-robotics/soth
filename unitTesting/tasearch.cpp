/*
 *  Copyright
 */

#include "soth/HCOD.hpp"
#include "soth/debug.h"
#include "MatrixRnd.h"

void generateDeficientDataSet( std::vector<Eigen::MatrixXd> &J,
			       std::vector<soth::bound_vector_t> &b,
			       const int NB_STAGE,
			       const int RANKFREE[],
			       const int RANKLINKED[],
			       const int NR[],
			       const int NC )
{
  /* Initialize J and b. */
  J.resize(NB_STAGE);
  b.resize(NB_STAGE);

  unsigned int s = 0;
  for( int s=0;s<NB_STAGE;++s )
    {
      b[ s].resize(NR[ s]);

      assert( (RANKFREE[s]>0)||(RANKLINKED[s]>0) );

      J[s].resize( NR[s],NC ); J[s].setZero();
      if( RANKFREE[s]>0 )
	{
	  Eigen::MatrixXd Xhifree( NR[s],RANKFREE[s] );
	  Eigen::MatrixXd Jfr( RANKFREE[s],NC );
	  soth::MatrixRnd::randomize( Xhifree );
	  soth::MatrixRnd::randomize( Jfr );
	  if( Xhifree.cols()>0 ) J[s] += Xhifree*Jfr;
	}
      if( RANKLINKED[s]>0 )
	{
	  Eigen::MatrixXd Xhilinked( NR[s],RANKLINKED[s] );
	  soth::MatrixRnd::randomize( Xhilinked );
	  for( int sb=0;sb<s;++sb )
	  {
	    Eigen::MatrixXd Alinked( RANKLINKED[s],NR[sb] );
	    soth::MatrixRnd::randomize( Alinked );
	    J[s] += Xhilinked*Alinked*J[sb];
	  }
	}

      for( unsigned int i=0;i<NR[s];++i ) b[s][i] = (double)(i+1);
    }
}

int main (int argc, char** argv)
{
  sotDebugTrace::openFile();
  const int NB_STAGE = 3;
  const int RANKFREE[]   = { 5, 2, 5,     5, 3 };
  //const int RANKFREE[]   = { 5, 4, 5,     5, 3 };
  const int RANKLINKED[] = { 0, 2, 1,     5, 3 };
  const int NR[]         = { 5, 4, 5,     5, 8 };
  const int NC = 15;

  /* Initialize J and b. */
  std::vector<Eigen::MatrixXd> J(NB_STAGE);
  std::vector<soth::bound_vector_t> b(NB_STAGE);
  generateDeficientDataSet(J,b,NB_STAGE,RANKFREE,RANKLINKED,NR,NC);

  b[0][2] = std::make_pair(-0.3,.3);
  b[0][3] = std::make_pair(-0.4,.4);
  b[0][4] = std::make_pair(-0.5,.5);


  for( unsigned int i=0;i<NB_STAGE;++i )
    {
      std::cout << "J"<<i+1<<" = " << (soth::MATLAB)J[i] << std::endl;
      std::cout << "e"<<i+1<< " = " << b[i] << ";"<<std::endl;
    }
  //  assert( std::abs(J[0](0,0)-(-1.1149))<1e-5 );

  /* SOTH structure construction. */
  soth::HCOD hcod(NC,NB_STAGE);
  for( unsigned int i=0;i<NB_STAGE;++i )
    {
      hcod.pushBackStage( J[i],b[i] );
      assert(NR[i]>0);
      if (NR[i]>1)
        hcod.setInitialActiveSet( Eigen::VectorXi::LinSpaced(0, NR[i]-1, NR[i]),i);
      else
        hcod.setInitialActiveSet( Eigen::VectorXi::Zero(1), i);
    }
  hcod.setNameByOrder("stage_");

  VectorXd solution;
  hcod.activeSearch( solution );
  hcod.show(std::cout);
}