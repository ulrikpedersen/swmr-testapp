#include <iostream>
#include <string>
#include <assert.h>
#include <sys/time.h>


#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/ndc.h>
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;


#include "hdf5.h"
#include "swmr-testdata.h"

using namespace std;

int main() {
  DOMConfigurator::configure("Log4cxxConfig.xml");
  LoggerPtr log(Logger::getLogger("main"));

  LOG4CXX_DEBUG(log, "this is a debug message.");



  return 0;
}

