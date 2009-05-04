#include <nucleo/nucleo.H>
#include <nucleo/utils/AppUtils.H>
#include <nucleo/core/ReactiveEngine.H>

#include "GridFlow.H"
#include "AutoFlow.H"
#include "OpticalFlowViewer.H"

#include <stdexcept>

using namespace nucleo ;

int
main(int argc, char **argv) {
  try {
    char *SOURCE = "videoin:?framerate=20" ;
    std::string FLOW_TYPE = "auto" ;
    int TMIN=3, TMAX=400 ;
    
    int firstArg = parseCommandLine(argc, argv, "i:f:t:T:", "sSii",
							 &SOURCE,&FLOW_TYPE,&TMIN,&TMAX) ;
    if (firstArg<0) {
	 std::cerr << std::endl << argv[0] 
			 << " [-i source] [-f flow-type]"
			 << " [-t min-dist-threshold] [-T max-dist-threshold] [arg]"
			 << std::endl ;
	 exit(1) ;
    }

    std::cerr << "Getting images from " << SOURCE << std::endl ;

    OpticalFlow *flow = 0 ;
    if (FLOW_TYPE=="grid") {
	 int grid_size = (argc-firstArg<1) ? 15 : atoi(argv[firstArg]) ;
	 flow = new GridFlow(SOURCE, grid_size) ;
    } else if (FLOW_TYPE=="auto") {
	 int max = (argc-firstArg<1) ? 300 : atoi(argv[firstArg]) ;
	 flow = new AutoFlow(SOURCE, max) ;
    } else {
	 std::cerr << "flow type: \"" << FLOW_TYPE << "\"" << std::endl ;
	 flow = new OpticalFlow(SOURCE) ;
    }

    OpticalFlowViewer viewer(flow,TMIN,TMAX) ;

    ReactiveEngine::run() ;

    delete flow ;
  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }

  return 0 ;
}
