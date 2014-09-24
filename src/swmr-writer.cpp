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

class SWMRWriter {
public:
  SWMRWriter(const char * fname);
  ~SWMRWriter();
  void create_file();
  void get_test_data();
  void write_test_data(unsigned int niter);

private:
  LoggerPtr log;
  hid_t fid;
  string filename;
  Image_t *pimg;
};


SWMRWriter::SWMRWriter (const char* fname)
{
  this->log = Logger::getLogger("SWMRWriter");
  this->filename = fname;
  this->fid = -1;
  this->pimg = NULL;
}

void SWMRWriter::create_file()
{
  hid_t fapl;         /* File access property list */

  /* Create file access property list */
  fapl = H5Pcreate(H5P_FILE_ACCESS);
  assert( fapl >= 0);

  /* Set to use the latest library format */
  assert (H5Pset_libver_bounds(fapl, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) >= 0);

  /* Open the file */
  this->fid = H5Fopen(this->filename.c_str(), H5F_ACC_RDWR | H5F_ACC_SWMR_WRITE, fapl);
  assert( this->fid >= 0);

  /* Close file access property list */
  assert (H5Pclose(fapl) >= 0);
}

void SWMRWriter::get_test_data ()
{
  //this->pimg = (Image_t *)calloc(1, sizeof(Image_t));

}

void SWMRWriter::write_test_data (unsigned int niter)
{

}

SWMRWriter::~SWMRWriter ()
{
  delete this->log;
  if (this->fid >= 0){
    assert (H5Fclose(this->fid) >= 0);
    this->fid = -1;
  if (this->pimg != NULL) {

  }
  }

}


int main() {
  DOMConfigurator::configure("Log4cxxConfig.xml");
  LoggerPtr log(Logger::getLogger("main"));

  LOG4CXX_DEBUG(log, "this is a debug message.");



  return 0;
}

